#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#endif

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>


// https://stackoverflow.com/questions/3219393/stdlib-and-colored-output-in-c
#define ANSI_COLOR_GREEN   "\e[32m"
#define ANSI_COLOR_YELLOW  "\e[33m"
#define ANSI_COLOR_RESET   "\e[0m"

#define TERMINATOR       1
#define TRUE             1
#define FALSE            0

// This info is related with message format
// +----------------------------------------+
// |             Date (16 bytes)            |
// +----------------------------------------+
// |            Padding (4 bytes)           |
// +----------------------------------------+
// |            Sender (32 bytes)           |
// +----------------------------------------+
// |            Padding (4 bytes)           |
// +----------------------------------------+
// |                                        |
// |                                        |
// |           Message (964 bytes)          |
// |                                        |
// |                                        |
// +----------------------------------------+
// |            Padding (4 bytes)           |
// +----------------------------------------+
#define BUFFER_SIZE 1024 
#define METADATA_MESSAGE_SIZE 964 
#define METADATA_USERNAME_SIZE 32 
#define METADATA_DATE_SIZE 16
#define METADATA_PADDING_SIZE 16
#define PADDING "\0\0\0\0"

#define DATE_POS 0 
#define USERNAME_POS 20 
#define MESSAGE_POS 56


pthread_mutex_t output_mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct userInfo{
	char username[METADATA_USERNAME_SIZE];
} userInfo;
userInfo user_info;

const int PORT = 6969;
int socket_connected = FALSE;
int socket_fd = 0;


void fatal(char *message)
{
	fprintf(stderr, "%s\n", message);
	exit(EXIT_FAILURE);
}

#ifdef _WIN32
void initialize_winsocs()
{
	WSADATA wsa_data;
	if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0)
		fatal("WSAStartup failed");
}
#endif

void build_complete_msg(char *dest, char *username, char *message){
	// Get message date
	time_t currentTime;
	struct tm* localTime;
	char date[METADATA_DATE_SIZE] = {0};
	currentTime = time(NULL);
	localTime = localtime(&currentTime);
	strftime(date, sizeof(date), "%m-%d %H:%M", localTime);

	// Build message
	strncpy(&dest[DATE_POS], date, strlen(date));
	strncpy(&dest[USERNAME_POS], username, strlen(username));
	strncpy(&dest[MESSAGE_POS], message, strlen(message));
}

void chat_read(void)
{
	char msg_received[METADATA_MESSAGE_SIZE] = {0};

	// Read and print-out received message
	while(socket_connected){
		int valread = 0;
		if ((valread = read(socket_fd, msg_received, METADATA_MESSAGE_SIZE)) == -1)
			fatal("Failed to read content from socket buffer");

		// Check if any text was read from socket
		if (valread){

			pthread_mutex_lock(&output_mutex);
			printf("\r"); // Clear current line
			fprintf(stdout, "[%s] %s%s:%s %s", &msg_received[DATE_POS], ANSI_COLOR_GREEN, &msg_received[USERNAME_POS], ANSI_COLOR_RESET, &msg_received[MESSAGE_POS]);
			
			// Reproduce the line user input line
			printf("\r"); // Clear current line
			fprintf(stdout, "%s%s:%s ", ANSI_COLOR_YELLOW, user_info.username, ANSI_COLOR_RESET);
			fflush(stdout);
			pthread_mutex_unlock(&output_mutex);

			memset(msg_received, 0, METADATA_MESSAGE_SIZE);
		}
	}
}

void chat_write(void)
{
	char plain_msg_buffer[METADATA_MESSAGE_SIZE] = {0};
	char complete_msg_buffer[BUFFER_SIZE] = {0};

	// Read server message, insert into buffer and sent it 
	while(socket_connected){
		pthread_mutex_lock(&output_mutex);
		fprintf(stdout, "%s%s:%s ", ANSI_COLOR_YELLOW, user_info.username, ANSI_COLOR_RESET);
		fflush(stdout);
		pthread_mutex_unlock(&output_mutex);

		if (fgets(plain_msg_buffer, METADATA_MESSAGE_SIZE, stdin) == NULL){
			fatal("Wasn't possible to read user message");
		};

		// Check if msg contains "Exit" then quit server 
		if (strncmp("Exit", plain_msg_buffer, 4) == 0){
			fprintf(stdout, "Exiting server...\n");
			socket_connected = FALSE;
		}

		build_complete_msg(complete_msg_buffer, user_info.username, plain_msg_buffer);
		write(socket_fd, complete_msg_buffer, sizeof(complete_msg_buffer) - TERMINATOR);

		memset(plain_msg_buffer, 0, sizeof(plain_msg_buffer));
		memset(complete_msg_buffer, 0, sizeof(complete_msg_buffer));
	}
}

void chat_start(void)
{
	// Create and designate thread functions to read and write content in/to socket 
	pthread_t t1, t2;

	// Assign a thread to each process
	if (pthread_create(&t1, NULL, (void *) chat_write, NULL) != 0){
		fatal("failed to create new thread");
	}
	if (pthread_create(&t2, NULL, (void *) chat_read, NULL)){
		fatal("failed to create new thread");
	}

	// Wait till both threads end
	pthread_join(t1, NULL);
	pthread_join(t2, NULL);
}

void client_connect(const char *IP)
{
	struct sockaddr_in address;
	memset(&address, 0, sizeof(address));
	int socket_handler;
	
	// Creating socket descriptor 
	fprintf(stdout, "building connection to socket...\n");
#ifdef _WIN32
	WSADATA wsa_data;
	if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0){
		fatal("socket initialization failed");
	}
#endif
	if ((socket_handler = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		fatal("socket creation failed");
	}
	address.sin_family = AF_INET;
	address.sin_port = htons(PORT);

	// Assign IP 
	fprintf(stdout, "Assigning %s IP to socket...\n", IP);
#ifdef _WIN32
	if (inet_pton(AF_INET, IP, &(address.sin_addr)) != 1){
#else
	if (inet_aton(IP, &(address.sin_addr)) == 0){
#endif
		fatal("invalid IP inserted");
	}
	// Initiate connection on a socket 
	fprintf(stdout, "Connecting to server %s...\n", IP);
	if (connect(socket_handler,(struct sockaddr*)&address, sizeof(address)) == -1)
		fatal("socket connection failed");

	socket_connected = TRUE;
	socket_fd = socket_handler; // Define socket as global-variable
}

void server_start(void)
{
	struct sockaddr_in address, cli;
	const int SERVER_PROTOCOL = AF_INET;
	const int CLI_LEN = sizeof(cli);
	const int BACKLOG_LEN = 32;
	int socket_handler;

	// Creating socket descriptor 
	fprintf(stdout, "Building connection to socket...\n");
#ifdef _WIN32
	WSADATA wsa_data;
	if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0){
		fatal("socket initialization failed");
	}
#endif
	if ((socket_handler = socket(SERVER_PROTOCOL, SOCK_STREAM, 0)) == -1){
		fatal("socket creation failed");
	}
	memset(&address, 0, sizeof(address)); // Fill address with 0's 
	address.sin_family = SERVER_PROTOCOL;
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	address.sin_port = htons(PORT);

	// Attach socket to given PORT 
	fprintf(stdout, "Building server...\n");
	if (bind(socket_handler, (struct sockaddr*)&address, sizeof(address)) == -1){
		fatal("socket binding failed");
	}
	fprintf(stdout, "Waiting for client to connect...\n");
	if (listen(socket_handler, BACKLOG_LEN) == -1){
		fatal("socket listening failed");
	}
	fprintf(stdout, "Accepting connection from client...\n");
	if ((socket_handler = accept(socket_handler,(struct sockaddr*)&cli,(socklen_t*)&CLI_LEN)) == -1){
		fatal("failed to accept new client");
	}
	socket_connected = TRUE;
	socket_fd = socket_handler; // Define socket as global-variable
}
