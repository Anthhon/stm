/* This file should only deal with control flow in server-side */

#include "common.c"
#include "common.h"

void server_start(void)
{
	struct sockaddr_in address, cli;
	const int CLI_LEN = sizeof(cli);

	// Creating socket descriptor 
	OutputInfo("Building connection to socket...\n");
	if ((serverData.socketHandler = socket(serverData.PROTOCOL, SOCK_STREAM, 0)) == -1){
		Fatal("Socket creation failed\n");
	}

	// Attaching socket to given serverData.port
	memset(&address, 0, sizeof(address));
	address.sin_family = serverData.PROTOCOL;
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	address.sin_port = htons(serverData.port);

	// Attach socket to given serverData.port 
	OutputInfo("Building server...\n");
	if (bind(serverData.socketHandler, (struct sockaddr*)&address, sizeof(address)) == -1){
		Fatal("Socket binding failed\n");
	}
	OutputInfo("Waiting for client to connect...\n");
	if (listen(serverData.socketHandler, BACKLOG_SIZE) == -1){
		Fatal("Socket listening failed\n");
	}
	OutputInfo("Accepting connection from client...\n");
	if ((serverData.socketHandler = accept(serverData.socketHandler,(struct sockaddr*)&cli,(socklen_t*)&CLI_LEN)) == -1){
		Fatal("Failed to accept new client\n");
	}
	// Declare that the connection has been made
	serverData.isConnected = true;
}

int main(int argc, char *argv[])
{
	// Avoid missing arguments
	if (argc != 4){
		Fatal("Usage: %s [server_ip] [username] [port]\n", argv[0]);
	}

	// Check username
	regex_t regex;
	if (regcomp(&regex, "^[a-zA-Z0-9]+$", REG_EXTENDED) != 0) {
		Fatal("Regex compilation failed\n");
	}
	if (regexec(&regex, argv[2], 0, NULL, 0) != 0) {
		Fatal("Invalid username format. Use only alphanumeric characters\n");
	}
	regfree(&regex);
	
	const size_t username_len = strlen(argv[2]);
	if (username_len > MAX_USERNAME_SIZE - TERMINATOR) {
		Fatal("Username \"%s\" is too long\n", argv[2]);
	}

	// Define IP
	const char *IP = argv[1];
	strncpy(serverData.ip, IP, sizeof(serverData.ip));
	// Define username
	const char *USERNAME = argv[2];
	strncpy(userData.username, USERNAME, sizeof(userData.username));
	// Check and define server PORT
	const char *PORT = argv[3];
	unsigned int PORT_VAL = atoi(PORT);
	if (PORT_VAL < 1 || PORT_VAL > 65535) {
		Fatal("Invalid port number. Port must be between 1 and 65535\n");
	}
	serverData.port = PORT_VAL;

	OutputInfo("Starting server...\n");
	server_start(); 
	OutputInfo("Waiting for messages...\n");
	chatStart();

	/* Closing the connected socket */
	close(serverData.isConnected);

#ifdef _WIN32
	WSACleanup();
#endif

	return 0;
}
