/* This file should only deal with app flow control in client-side */

#include "common.c"

int main(int argc, char const *argv[])
{
	if (argc != 3){
		fprintf(stdout, "Usage: %s [server_ip] [username]\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	const char *IP = argv[1];
	const char *USERNAME = argv[2];

	fprintf(stdout, "Connecting %s to the server...\n", USERNAME);
	client_connect(IP);
	fprintf(stdout, "Starting chat as %s...\n", USERNAME);
	chat_start();

	/* Closing the connected socket */
	close(socket_connected);
#ifdef _WIN32
	WSACleanup();
#endif
	return(0);
}
