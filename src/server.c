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
	size_t byte_size = sizeof(msg_buffer);

	while (TRUE){
		/* Read and print-out receive message */
		(void)read(cli_socket, msg_buffer, byte_size);
		(void)fprintf(stdout, "From client: %s", msg_buffer);
		(void)memset(msg_buffer, 0, byte_size);

		/* Read server message, insert into buffer and sent it */
		int i = 0;
		(void)fprintf(stdout, "Type your message: ");
		while ((msg_buffer[i++] = getchar()) != '\n');
		write(cli_socket, msg_buffer, byte_size);
		(void)memset(msg_buffer, 0, byte_size);

		/* Check if msg contains "Exit" then quit server */
		if (strcmp("Exit", msg_buffer) == 0){
			(void)fprintf(stdout, "Exiting server...");
			break;
		}
	}
}

int main()
{
	struct sockaddr_in address, cli;
	const int SERVER_PROTOCOL = AF_INET;
	const int BACKLOG_LEN = 3;
	const int PORT = 8080;
	const int CLI_LEN = sizeof(cli);
	int socket_handler;
	int new_socket;

	/* Creating socket descriptor */
	if ((socket_handler = socket(SERVER_PROTOCOL, SOCK_STREAM, 0)) == -1){
		(void)fprintf(stderr, "ERROR: socket creation failed");
		exit(EXIT_FAILURE);
	}
	bzero(&address, sizeof(address)); /* Fill address with 0's */

	address.sin_family = SERVER_PROTOCOL;
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	address.sin_port = htons(PORT);

	/* Attach socket to given IP and PORT */
	if (bind(socket_handler, (struct sockaddr*)&address, sizeof(address)) == -1){
		(void)fprintf(stderr, "ERROR: socket binding at %d failed", PORT);
		exit(EXIT_FAILURE);
	}
	if (listen(socket_handler, BACKLOG_LEN) == -1){
		(void)fprintf(stderr, "ERROR: socket listening failed");
		exit(EXIT_FAILURE);
	}

	if ((new_socket = accept(socket_handler,
			  (struct sockaddr*)&cli,
			  (socklen_t*)&CLI_LEN)) == -1){
		(void)fprintf(stderr, "ERROR: failed to accept new client");
		exit(EXIT_FAILURE);
	}

	chat_start(new_socket);	

	/* Close open socket after chatting */
	(void)close(new_socket);

	return 0;
}
