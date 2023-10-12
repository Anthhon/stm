/* This file should only deal with control flow in server-side */

#include "common.c"

void server_start(void)
{
	struct sockaddr_in address, cli;
	const int CLI_LEN = sizeof(cli);

	// Creating socket descriptor 
	fprintf(stdout, "Building connection to socket...\n");
	if ((serverData.socket_handler = socket(serverData.PROTOCOL, SOCK_STREAM, 0)) == -1){
		Fatal("Socket creation failed\n");
	}

	// Attaching socket to given serverData.port
	memset(&address, 0, sizeof(address));
	address.sin_family = serverData.PROTOCOL;
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	address.sin_port = htons(serverData.port);

	// Attach socket to given serverData.port 
	fprintf(stdout, "Building server...\n");
	if (bind(serverData.socket_handler, (struct sockaddr*)&address, sizeof(address)) == -1){
		Fatal("Socket binding failed\n");
	}
	fprintf(stdout, "Waiting for client to connect...\n");
	if (listen(serverData.socket_handler, serverData.BACKLOG_SIZE) == -1){
		Fatal("Socket listening failed\n");
	}
	fprintf(stdout, "Accepting connection from client...\n");
	if ((serverData.socket_handler = accept(serverData.socket_handler,(struct sockaddr*)&cli,(socklen_t*)&CLI_LEN)) == -1){
		Fatal("Failed to accept new client\n");
	}

	serverData.socket_connected = true;
}

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
	close(serverData.socket_connected);
#ifdef _WIN32
	WSACleanup();
#endif

	return 0;
}
