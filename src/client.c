/* This file should only deal with control flow in client-side */

#include "common.c"
#include "common.h"

void client_connect(const char *IP)
{
	struct sockaddr_in address;
	memset(&address, 0, sizeof(address));
	
	// Creating socket descriptor 
	OutputInfo("Building connection to socket\n");
	if ((serverData.socketHandler = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		Fatal("socket creation failed");
	}
	address.sin_family = AF_INET;
	address.sin_port = htons(serverData.port);

	// Assign IP 
	OutputInfo("Assigning IP to socket\n");
	if (inet_aton(IP, &(address.sin_addr)) == 0){
		Fatal("invalid IP address\n");
	}

	// Initiate connection on a socket 
	if (connect(serverData.socketHandler,(struct sockaddr*)&address, sizeof(address)) == -1) {
		Fatal("Socket connection failed\n");
	}
	// Declare that the connection has been made
	serverData.isConnected = true;
}

int main(int argc, char const *argv[])
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

	OutputInfo("Connecting to the server %s...\n", serverData.ip);
	client_connect(serverData.ip);
	OutputInfo("Starting chat as %s...\n", userData.username);
	chatStart();

	/* Closing the connected socket */
	close(serverData.isConnected);

	return(0);
}
