/* This file should only deal with control flow in server-side */

#include "common.c"

int main(void)
{
	fprintf(stdout, "Starting server...\n");
	server_start(); 
	fprintf(stdout, "Waiting for messages...\n");
	chat_start();

	/* Closing the connected socket */
	close(socket_connected);
#ifdef _WIN32
	WSACleanup();
#endif
	return 0;
}
