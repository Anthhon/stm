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
#include <unistd.h>

// https://stackoverflow.com/questions/3219393/stdlib-and-colored-output-in-c
#define ANSI_COLOR_GREEN   "\e[32m"
#define ANSI_COLOR_YELLOW  "\e[33m"
#define ANSI_COLOR_RESET   "\e[0m"

#define BUFFER_SIZE      4096
#define MAX_MESSAGE_SIZE 256
#define TERMINATOR       1
#define TRUE             1
#define FALSE            0

pthread_mutex_t output_mutex = PTHREAD_MUTEX_INITIALIZER;
char *msg_buffer_ptr = NULL;
char *msg_received_ptr = NULL;
const int PORT = 8080;
static volatile int flag_msg_received = TRUE;
int socket_connected = FALSE;
int socket_fd = 0;

#ifdef _WIN32
void initialize_winsocs(){
	WSADATA wsa_data;
	if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0){
		fprintf(stderr, "ERROR: WSAStartup failed\n");
		exit(EXIT_FAILURE);
	}
}
#endif

void chat_read(void)
{
	char msg_received[MAX_MESSAGE_SIZE];
	size_t byte_size = sizeof(msg_received);
	msg_received_ptr = msg_received;

	// Read and print-out receive message
	while(socket_connected){
		int valread = 0;
		memset(msg_received, 0, byte_size);
		// Check for errors
		if ((valread = read(socket_fd, msg_received, byte_size)) == -1){
			fprintf(stderr, "ERROR: Failed to read content from socket buffer\n");
			exit(EXIT_FAILURE);
		}
		// Check if any text was read from socket
		if (valread > 0){
			pthread_mutex_lock(&output_mutex);
			printf("\r\033[K"); // Clear current line
			fprintf(stdout, "%sCLIENT:%s %s", ANSI_COLOR_GREEN, ANSI_COLOR_RESET, msg_received);
			// Tries to reproduce the line user input line
			fprintf(stdout, "%sYOU(in-while):%s %s", ANSI_COLOR_YELLOW, ANSI_COLOR_RESET, msg_buffer_ptr);
			flag_msg_received = TRUE;
			fflush(stdout);
			pthread_mutex_unlock(&output_mutex);
		}
		memset(msg_received, 0, byte_size);
	}
}

void chat_write(void)
{
	char msg_buffer[MAX_MESSAGE_SIZE];
	size_t byte_size = sizeof(msg_buffer);
	msg_buffer_ptr = msg_buffer;

	// Read server message, insert into buffer and sent it 
	while(socket_connected){
		int i = 0;
		size_t msg_counter = 0;

		memset(msg_buffer, 0, byte_size);
		pthread_mutex_lock(&output_mutex);
		fprintf(stdout, "%sYOU(out-while):%s ", ANSI_COLOR_YELLOW, ANSI_COLOR_RESET);
		fflush(stdout);
		pthread_mutex_unlock(&output_mutex);

		while ((msg_buffer[i++] = getchar()) != '\n'){
			// If user is typing a message and receive another one
			// the terminal is going to clena the current line and
			// print out the message and the message buffer content
			if (flag_msg_received == TRUE){
				pthread_mutex_lock(&output_mutex);
				flag_msg_received = FALSE; // Return flag to original value
				fflush(stdout);
				pthread_mutex_unlock(&output_mutex);
			}
			++msg_counter;
		}
		if (msg_counter > MAX_MESSAGE_SIZE){
			memset(msg_buffer, 0, byte_size);
			fprintf(stderr, "ERROR: Invalid message size: %li\n", msg_counter);
			continue;
		}
		else write(socket_fd, msg_buffer, byte_size);

		// Check if msg contains "Exit" then quit server 
		if (strncmp("Exit", msg_buffer, 4) == 0){
			fprintf(stdout, "Exiting server...\n");
			socket_connected = FALSE;
		}
		memset(msg_buffer, 0, byte_size);
	}
}

void chat_start(void)
{
	// Create and designate thread functions to read and write content in/to socket 
	pthread_t t1, t2;

	pthread_create(&t1, NULL, (void *) chat_write, NULL);
	pthread_create(&t2, NULL, (void *) chat_read, NULL);

	pthread_join(t1, NULL);
	pthread_join(t2, NULL);
}

void client_connect(const char *IP)
{
	struct sockaddr_in address;
	memset(&address, 0, sizeof(address));
	int socket_handler;
	
	// Creating socket descriptor 
	fprintf(stdout, "Building connection to socket...\n");
#ifdef _WIN32
	WSADATA wsa_data;
	if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0){
	fprintf(stderr, "ERROR: socket initialization failed\n");
	exit(EXIT_FAILURE);
	}
#endif
	if ((socket_handler = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		fprintf(stderr, "ERROR: socket creation failed\n");
		exit(EXIT_FAILURE);
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
		fprintf(stderr, "ERROR: invalid IP inserted\n");
		exit(EXIT_FAILURE);
	}

	// Initiate connection on a socket 
	fprintf(stdout, "Connecting to server %s...\n", IP);
	if (connect(socket_handler, 
			     (struct sockaddr*)&address, sizeof(address)) == -1){
		fprintf(stderr, "ERROR: socket connection failed\n");
		exit(EXIT_FAILURE);
	}
	socket_connected = TRUE;

	socket_fd = socket_handler; // Define socket as global-variable
}

void server_start(void)
{
	struct sockaddr_in address, cli;
	const int SERVER_PROTOCOL = AF_INET;
	const int CLI_LEN = sizeof(cli);
	const int BACKLOG_LEN = 10;
	int socket_handler;

	// Creating socket descriptor 
	fprintf(stdout, "Building connection to socket...\n");
#ifdef _WIN32
	WSADATA wsa_data;
	if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0){
	fprintf(stderr, "ERROR: socket initialization failed\n");
	exit(EXIT_FAILURE);
	}
#endif
	if ((socket_handler = socket(SERVER_PROTOCOL, SOCK_STREAM, 0)) == -1){
		fprintf(stderr, "ERROR: socket creation failed\n");
		exit(EXIT_FAILURE);
	}
	memset(&address, 0, sizeof(address)); // Fill address with 0's 

	address.sin_family = SERVER_PROTOCOL;
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	address.sin_port = htons(PORT);

	// Attach socket to given PORT 
	fprintf(stdout, "Building server...\n");
	if (bind(socket_handler, (struct sockaddr*)&address, sizeof(address)) == -1){
		fprintf(stderr, "ERROR: socket binding at %d failed\n", PORT);
		exit(EXIT_FAILURE);
	}
	fprintf(stdout, "Waiting for client to connect...\n");
	if (listen(socket_handler, BACKLOG_LEN) == -1){
		fprintf(stderr, "ERROR: socket listening failed\n");
		exit(EXIT_FAILURE);
	}

	fprintf(stdout, "Accepting connection from client...\n");
	if ((socket_handler = accept(socket_handler,
			  (struct sockaddr*)&cli,
			  (socklen_t*)&CLI_LEN)) == -1){
		fprintf(stderr, "ERROR: failed to accept new client\n");
		exit(EXIT_FAILURE);
	}
	socket_connected = TRUE;

	socket_fd = socket_handler; // Define socket as global-variable
}
