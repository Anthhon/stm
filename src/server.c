/* Server-side C program to understand Sockets programming */

#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define INIT_GREEN "\033[0;31m"
#define END_COLOR "\033[0m"

#define BUFFER_SIZE 4096
#define MAX_MESSAGE_SIZE 256
#define TERMINATOR 1
#define TRUE 1
#define FALSE 0

void chat_start(int cli_socket)
{
	char msg_buffer[MAX_MESSAGE_SIZE];
	char msg_received[MAX_MESSAGE_SIZE];
	size_t byte_size = sizeof(msg_buffer);

	while (TRUE){
		/* Read and print-out receive message */
		memset(msg_received, 0, byte_size);
		read(cli_socket, msg_received, byte_size);
		fprintf(stdout, "From client: %s", msg_received);

		/* Read server message, insert into buffer and sent it */
		int i = 0;
		memset(msg_buffer, 0, byte_size);
		fprintf(stdout, "Type your message: ");
		while ((msg_buffer[i++] = getchar()) != '\n');
		write(cli_socket, msg_buffer, byte_size);

		/* Check if msg contains "Exit" then quit server */
		if (strncmp("Exit", msg_buffer, 4) == 0){
			fprintf(stdout, "Exiting server...\n");
			break;
		}
	}
}

int server_start()
{
	struct sockaddr_in address, cli;
	const int SERVER_PROTOCOL = AF_INET;
	const int CLI_LEN = sizeof(cli);
	const int BACKLOG_LEN = 10;
	const int PORT = 25565;
	int socket_handler;

	/* Creating socket descriptor */
	fprintf(stdout, "Building connection to socket...\n");
	if ((socket_handler = socket(SERVER_PROTOCOL, SOCK_STREAM, 0)) == -1){
		fprintf(stderr, "ERROR: socket creation failed\n");
		exit(EXIT_FAILURE);
	}
	memset(&address, 0, sizeof(address)); /* Fill address with 0's */

	address.sin_family = SERVER_PROTOCOL;
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	address.sin_port = htons(PORT);

	/* Attach socket to given PORT */
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

	return socket_handler;
}

int main()
{
	fprintf(stdout, "Starting server...\n");
	int server_socket = server_start(); 
	fprintf(stdout, "Waiting for messages...\n");
	chat_start(server_socket);

	/* Closing the connected socket */
	close(server_socket);

	return 0;
}
