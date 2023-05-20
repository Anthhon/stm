/* This file should only deal with control flow in client-side */

#include "common.c"

int main(int argc, char const *argv[])
{
	if (argc != 3){
		fprintf(stdout, "Usage: %s [server_ip] [username]\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	const size_t username_len = strlen(argv[2]);
	if (username_len >= METADATA_USERNAME_SIZE - TERMINATOR)
		fatal("Username is too long");

	// Define username
	const char *IP = argv[1];
	const char *USERNAME = argv[2];
	strncpy(user_info.username, USERNAME, username_len);

	fprintf(stdout, "Connecting to the server...\n");
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
