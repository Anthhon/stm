/* This file should only deal with control flow in client-side */

#include "common.c"

void client_connect(const char *IP)
{
	struct sockaddr_in address;
	memset(&address, 0, sizeof(address));
	
	// Creating socket descriptor 
	OutputInfo("Building connection to socket\n");
	if ((serverData.socket_handler = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		Fatal("socket creation failed");
	}
	address.sin_family = AF_INET;
	address.sin_port = htons(serverData.port);

	// Assign IP 
	OutputInfo("Assigning %s IP to socket\n", IP);
	if (inet_aton(IP, &(address.sin_addr)) == 0){
		Fatal("invalid IP inserted");
	}
	// Initiate connection on a socket 
	OutputInfo("Connecting to server %s\n", IP);
	if (connect(serverData.socket_handler,(struct sockaddr*)&address, sizeof(address)) == -1) {
		Fatal("Socket connection failed\n");
	}

	serverData.socket_connected = true;
}

int main(int argc, char const *argv[])
{
	// TODO: Implement better arguments checking to
	// avoid putting arguments in wrong order
	if (argc != 3){
		Fatal("Usage: %s [server_ip] [username]\n", argv[0]);
	}

	// Check username len
	const size_t username_len = strlen(argv[2]);
	if (username_len > MAX_USERNAME_SIZE - TERMINATOR) {
		Fatal("Username is too long\n");
	}

	// Define username
	const char *IP = argv[1];
	const char *USERNAME = argv[2];
	strncpy(userData.username, USERNAME, username_len);

	OutputInfo("Connecting to the server\n");
	client_connect(IP);
	OutputInfo("Starting chat as %s...\n", USERNAME);
	chat_start();

	/* Closing the connected socket */
	close(serverData.socket_connected);

	return(0);
}
