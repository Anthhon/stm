/* This file should only deal with app flow control in server-side */

#include "common.c"

int main(void)
{
	fprintf(stdout, "Starting server...\n");
	int server_socket = server_start(); 
	fprintf(stdout, "Waiting for messages...\n");
	server_chat_start(server_socket);

	/* Closing the connected socket */
	close(server_socket);
#ifdef _WIN32
	WSACleanup();
#endif
	return 0;
}
