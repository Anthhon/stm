/* Client-side C program to understand Sockets programming */

#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main(int argc, char const *argv[])
{
	struct sockaddr_in adress;
	char buffer[BUFFER_SIZE] = { 0 };
	char *message = "Hello from client";	
	int socket_handler, valread, status;
	int addrlen = sizeof(adress);

	if (argc != 2){
		printf("Usage: %s [server_ip]\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	/* Creating socket descriptor */
	if ((socket_handler = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		perror("[SOCKET HANDLER] ");
		exit(EXIT_FAILURE);
	}
	adress.sin_family = AF_INET;
	adress.sin_port = htons(PORT);

	/* Convert IPv4 and IPv6 addresses from text to binary */	
	if (inet_pton(AF_INET, argv[1], &adress.sin_addr) <= 0){
		perror("[ADRESS FAILED] ");
		exit(EXIT_FAILURE);
	}

	/* Initiate connection on a socket */
	if ((status = connect(socket_handler, 
			     (struct sockaddr*)&adress, sizeof(adress))) == -1){
		perror("[STATUS ERROR] ");
		exit(EXIT_FAILURE);
	}

	send(socket_handler, message, strlen(message), 0);	
	printf("Hello message sent\n");
	valread = read(socket_handler, buffer, BUFFER_SIZE);
	printf("%s", buffer);

	/* Closing the connected socket */
	close(socket_handler);
	return(0);
}
