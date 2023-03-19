/* Server-side C program to understand Sockets programming */

#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define INIT_GREEN "\033[0;31m"
#define END_COLOR "\033[0m"

#define PORT 8080
#define BUFFER_SIZE 4096
#define TERMINATOR 1

int main(int agrc, char const *argv[])
{
	struct sockaddr_in address;
	char buffer[BUFFER_SIZE] = { 0 };
	char server_name[] = "SERVER";	
	char message[] = "Sending testing message";
	int socket_handler, new_socket, read_status;
	int addrlen = sizeof(address);
	int backlog_len = 3;
	int opt = 1;

	/* Assign server name to message */
	char *f_message = malloc(sizeof(*f_message) * (strlen(server_name) + strlen(message) + TERMINATOR));
	(void)sprintf(f_message, "%s: %s", server_name, message); 

	/* Creating socket descriptor */
	if ((socket_handler = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		(void)fprintf(stderr, "ERROR: socket creation failed");
		exit(EXIT_FAILURE);
	}

	/* Initializing socket */	
	if (setsockopt(socket_handler, SOL_SOCKET,
		       SO_REUSEADDR | SO_REUSEPORT,
		       &opt, sizeof(opt))){
		(void)fprintf(stderr, "ERROR: socket operations setting failed");
		exit(EXIT_FAILURE);
	}
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);

	/* Attach socket to port 8080 */
	if (bind(socket_handler, (struct sockaddr*)&address, sizeof(address)) == -1){
		(void)fprintf(stderr, "ERROR: socket binding at %d failed", PORT);
		exit(EXIT_FAILURE);
	}
	if (listen(socket_handler, backlog_len) == -1){
		(void)fprintf(stderr, "ERROR: socket listening failed");
		exit(EXIT_FAILURE);
	}
	if ((new_socket = accept(socket_handler,
			  (struct sockaddr*)&address,
			  (socklen_t*)&addrlen)) == -1){
		(void)fprintf(stderr, "ERROR: failed to accept new socket");
		exit(EXIT_FAILURE);
	}

	/* Read socket handler */
	if ((read_status = read(new_socket, buffer, BUFFER_SIZE)) == -1){
		(void)fprintf(stderr, "ERROR: message reading failed");
		exit(EXIT_FAILURE);
	}
	(void)fprintf(stdout, "%s\n", buffer);
	(void)send(new_socket, f_message, strlen(f_message), 0);

	/* Ending the connected socket */
	(void)close(new_socket);
	/* Ending the listening socket */
	(void)shutdown(socket_handler, SHUT_RDWR);

	return 0;
}
