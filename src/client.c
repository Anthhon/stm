/* Client-side C program to understand Sockets programming */

#ifdef _WIN32
#include <winsock2.h>
#else
#include <netinet/in.h>
#include <sys/types.h>
#include <arpa/inet.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define INIT_GREEN "\033[0;32m"
#define END_COLOR "\033[0m"

#define BUFFER_SIZE 4096
#define MAX_MESSAGE_SIZE 256
#define TERMINATOR 1
#define TRUE 1
#define FALSE 0

#ifdef _WIN32
void initialize_winsock(){
	WSADATA wsa_data;
	if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0){
		fprintf(stderr, "ERROR: WSAStartup failed\n");
		exit(EXIT_FAILURE);
	}
}
#endif

void chat_start(int cli_socket)
{
	char msg_buffer[MAX_MESSAGE_SIZE];
	char msg_received[MAX_MESSAGE_SIZE];
	size_t byte_size = sizeof(msg_buffer);

	while (TRUE){
		/* Read client message, insert into buffer and sent it */
		int i = 0;
		memset(msg_buffer, 0, byte_size);
		fprintf(stdout, "Type your message: ");
		while ((msg_buffer[i++] = getchar()) != '\n');
		write(cli_socket, msg_buffer, byte_size);

		/* Read and print-out receive message */
		memset(msg_received, 0, byte_size);
		read(cli_socket, msg_received, byte_size);
		fprintf(stdout, "From server: %s", msg_received);

		/* Check if msg contains "Exit" then quit server */
		if (strncmp("Exit", msg_buffer, 4) == 0){
			fprintf(stdout, "Exiting server...\n");
			break;
		}
	}
}

int server_connect(const char *IP)
{
	struct sockaddr_in address;
	memset(&address, 0, sizeof(address));
	const int PORT = 25565;
	int socket_handler;
	
	/* Creating socket descriptor */
	fprintf(stdout, "Building connection to socket...\n");
#ifdef _WIN32
	WSADATA wsa_data;
	if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0){
	fprintf(stderr, "ERROR: socket initialization failed");
	exit(EXIT_FAILURE);
	}
#endif
	if ((socket_handler = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		fprintf(stderr, "ERROR: socket creation failed");
		exit(EXIT_FAILURE);
	}
	address.sin_family = AF_INET;
	address.sin_port = htons(PORT);

	/* Assign IP */
	fprintf(stdout, "Assigning %s IP to socket...\n", IP);
#ifdef _WIN32
	if (inet_pton(AF_INET, IP, &(address.sin_addr)) == 0){
#else
	if (inet_aton(IP, &(address.sin_addr)) == 0){
#endif
		fprintf(stderr, "ERROR: invalid IP inserted");
		exit(EXIT_FAILURE);
	}

	/* Initiate connection on a socket */
	fprintf(stdout, "Connecting to server %s...\n", IP);
	if (connect(socket_handler, 
			     (struct sockaddr*)&address, sizeof(address)) == -1){
		fprintf(stderr, "ERROR: socket connection failed");
		exit(EXIT_FAILURE);
	}

	return socket_handler;
}

int main(int argc, char const *argv[])
{
	if (argc != 3){
		fprintf(stdout, "Usage: %s [server_ip] [username]\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	const char *IP = argv[1];
	const char *USERNAME = argv[2];

	fprintf(stdout, "Connecting %s to the server...\n", USERNAME);
	int cli_socket = server_connect(IP);
	fprintf(stdout, "Starting chat as %s...\n", USERNAME);
	chat_start(cli_socket);

	/* Closing the connected socket */
	close(cli_socket);

#ifdef _WIN32
	WSACleanup();
#endif

	return(0);
}
