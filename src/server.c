/* Server-side C program to understand Sockets programming */

#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main(int agrc, char const *argv[])
{
	struct sockaddr_in adress;
	char buffer[BUFFER_SIZE] = { 0 };
	char *message = "Hello from server!\n";
	int socket_handler, new_socket, valread;
	int addrlen = sizeof(adress);
	int backlog_len = 3;
	int opt = 1;

	/* Creating socket descriptor */
	if ((socket_handler = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		perror("[SOCKET HANDLER] ");
		exit(EXIT_FAILURE);
	}

	/* Initializing socket */	
	if (setsockopt(socket_handler, SOL_SOCKET,
		       SO_REUSEADDR | SO_REUSEPORT,
		       &opt, sizeof(opt))){
		perror("[SETSOCKOPT] ");
		exit(EXIT_FAILURE);
	}
	adress.sin_family = AF_INET;
	adress.sin_addr.s_addr = INADDR_ANY;
	adress.sin_port = htons(PORT);

	/* Attach socket to port 8080 */
	if (bind(socket_handler, (struct sockaddr*)&adress, sizeof(adress)) == -1){
		perror("[BIND FAILED] ");
		exit(EXIT_FAILURE);
	}
	if (listen(socket_handler, backlog_len) == -1){
		perror("[LISTEN FAILED] ");
		exit(EXIT_FAILURE);
	}
	if ((new_socket = accept(socket_handler,
			  (struct sockaddr*)&adress,
			  (socklen_t*)&addrlen)) == -1){
		perror("[ACCEPT FAILED] ");
		exit(EXIT_FAILURE);
	}

	/* Read socket handler */
	valread = read(new_socket, buffer, BUFFER_SIZE);
	printf("%s\n", buffer);
	send(new_socket, message, strlen(message), 0);
	printf("Hello message sent\n");

	/* Ending the connected socket */
	close(new_socket);
	/* Ending the listening socket */
	shutdown(socket_handler, SHUT_RDWR);

	return 0;
}
