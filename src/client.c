/* Client-side C program to understand Sockets programming */

#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define INIT_GREEN "\033[0;32m"
#define END_COLOR "\033[0m"

#define BUFFER_SIZE 4096
#define MAX_MESSAGE_SIZE 256
#define TERMINATOR 1
#define PORT 8080
#define TRUE 1
#define FALSE 0

void chat_start(int cli_socket){
	char msg_buffer[MAX_MESSAGE_SIZE];
	size_t byte_size = sizeof(msg_buffer);

	while (TRUE){
		/* Read client message, insert into buffer and sent it */
		int i = 0;
		(void)fprintf(stdout, "Type your message: ");
		while ((msg_buffer[i++] = getchar()) != '\n');
		write(cli_socket, msg_buffer, byte_size);		
		(void)memset(msg_buffer, 0, byte_size);

		/* Read and print-out receive message */
		(void)read(cli_socket, msg_buffer, byte_size);
		(void)fprintf(stdout, "From server: %s", msg_buffer);
		(void)memset(msg_buffer, 0, byte_size);

		/* Check if msg contains "Exit" then quit server */
		if (strcmp("Exit", msg_buffer) == 0){
			(void)fprintf(stdout, "Exiting server...");
			break;
		}
	}
}

void socket_build(const char *IP){
	struct sockaddr_in address;
	int socket_handler, status;
	
	/* Creating socket descriptor */
	if ((socket_handler = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		(void)fprintf(stderr, "ERROR: socket creation failed");
		exit(EXIT_FAILURE);
	}
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = inet_addr(IP);
	address.sin_port = htons(PORT);

	/* Initiate connection on a socket */
	if ((status = connect(socket_handler, 
			     (struct sockaddr*)&address, sizeof(address))) == -1){
		(void)fprintf(stderr, "ERROR: socket connection failed");
		exit(EXIT_FAILURE);
	}

	chat_start(socket_handler);

	/* Closing the connected socket */
	(void)close(socket_handler);
}

int main(int argc, char const *argv[])
{
	if (argc != 3){
		(void)fprintf(stdout, "Usage: %s [server_ip] [username]\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	socket_build(argv[1]);	

	return(0);
}
