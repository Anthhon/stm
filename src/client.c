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
#define TERMINATOR 1
#define PORT 8080

int main(int argc, char const *argv[])
{
	struct sockaddr_in address;
	char buffer[BUFFER_SIZE] = { 0 };
	char username[] = "Anthhon";	
	char message[] = "Hello server, that's my test message";	
	int socket_handler, status, read_status;

	/* Assign username to the message */
	char *f_message = malloc(sizeof(*f_message) * (strlen(username) + strlen(message) + TERMINATOR));	
	(void)sprintf(f_message, "%s: %s", username, message);

	if (argc != 2){
		(void)fprintf(stdout, "Usage: %s [server_ip]\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	/* Creating socket descriptor */
	if ((socket_handler = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		(void)fprintf(stderr, "ERROR: socket creation failed");
		exit(EXIT_FAILURE);
	}
	address.sin_family = AF_INET;
	address.sin_port = htons(PORT);

	/* Convert IPv4 and IPv6 addresses from text to binary */	
	if (inet_pton(AF_INET, argv[1], &address.sin_addr) <= 0){
		(void)fprintf(stderr, "ERROR: addresses conversions failed");
		exit(EXIT_FAILURE);
	}

	/* Initiate connection on a socket */
	if ((status = connect(socket_handler, 
			     (struct sockaddr*)&address, sizeof(address))) == -1){
		(void)fprintf(stderr, "ERROR: socket connection failed");
		exit(EXIT_FAILURE);
	}

	(void)send(socket_handler, f_message, strlen(f_message), 0);	
	if ((read_status = read(socket_handler, buffer, BUFFER_SIZE)) == -1){
		(void)fprintf(stderr, "ERROR: message reading failed");
		exit(EXIT_FAILURE);
	};
	(void)fprintf(stdout, "%s\n", buffer);

	/* Closing the connected socket */
	(void)close(socket_handler);

	return(0);
}
