/* This file should only deal with control flow in server-side */

#include "common.c"

int main(void)
{
	// Define username
	const char USERNAME[] = "Admin";
	strncpy(userData.username, USERNAME, strlen(USERNAME));

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
