/* This file should only deal with control flow in server-side */

#include <arpa/inet.h>
#include <asm-generic/errno-base.h>
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <regex.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <termios.h>

#include "common.h"
#include "server.h"
#include "colors.h"

// Data structure to store server information
ServerInfo serverData = {
	.PROTOCOL = AF_INET,
	.socketMaster = 0,
	.ip = DEFAULT_IP,
        .port = DEFAULT_PORT,
	.isConnected = false,
};

// Handle new messages from clients
void messagebroadcast(int *clients, int sender, Message *messageReceived)
{
	for (int i = 0; i < MAX_CLIENTS; ++i) {
		if (clients[i] > 0 && clients[i] != sender) {
			// Send message to all clients connected
			send(clients[i], messageReceived, sizeof(*messageReceived), 0);
			// Log message
			fprintf(stdout, "[%s] %s'%s'%s: %s", \
					messageReceived->date, \
					BGRN, messageReceived->user_data.username, CRESET, \
					messageReceived->message);
		}
	}
}

// Close the connection with a client
void connectionClose(int *client, int CLI_LEN, struct sockaddr_in *cli, struct sockaddr_in *address)
{
	// Get disconnected client details
	getpeername(*client, (struct sockaddr*)&cli, (socklen_t*)&CLI_LEN);
	OutputInfo("Client disconnected!\n\t- IP: %s\n\t- Port: %d\n", inet_ntoa(address->sin_addr), ntohs(address->sin_port));

	// Close client socket
	close(*client);
	*client = 0;
}

// Handle new client connections
void connectionNew(int new_client, int clients[MAX_CLIENTS], struct sockaddr_in* cli)
{
	for (int i = 0; i < MAX_CLIENTS; ++i) {
		if (clients[i] == 0) {
			clients[i] = new_client;
			OutputInfo("New client connected!\n\t- IP: %s\n\t- Port: %d\n\t- SocketFd: %d\n", 
					inet_ntoa(cli->sin_addr), ntohs(cli->sin_port), new_client);
			break;
		}
	}
}


// Initialize the server and handle client connections
void serverInitialize(void)
{
	struct sockaddr_in address, cli;
	fd_set socket_read_set;
	const int CLI_LEN = sizeof(cli);
	int clients[MAX_CLIENTS] = {0};
	int new_client;

	// Creating socket descriptor 
	OutputLog("Building connection to socket\n");
	if ((serverData.socketMaster = socket(serverData.PROTOCOL, SOCK_STREAM, 0)) == -1){
		Fatal("Socket creation failed\n");
	}

	// Attach socket to given serverData.port 
	memset(&address, 0, sizeof(address));
	address.sin_family = serverData.PROTOCOL;
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	address.sin_port = htons(serverData.port);

	OutputLog("Server using protocol %i\n", serverData.PROTOCOL);
	OutputLog("Server using port %i\n", serverData.port);

	OutputLog("Binding server to socket\n");
	if (bind(serverData.socketMaster, (struct sockaddr*)&address, sizeof(address)) == -1){
		Fatal("Socket binding failed\n");
	}

	if (listen(serverData.socketMaster, BACKLOG_SIZE) == -1){
		Fatal("Socket listening failed\n");
	}
	OutputLog("Server initialized, waiting for clients to connect...\n");

	// Handle multi-user connection
	while(true) {
		FD_ZERO(&socket_read_set); // Clear socket set
		FD_SET(serverData.socketMaster, &socket_read_set); // Add master socket to set

		// Add child socket to set
		int max_socket_descriptor = serverData.socketMaster;
		for (int i = 0; i < MAX_CLIENTS; ++i) {
			// Add socket to read list if valid
			if (clients[i] > 0) {
				FD_SET(clients[i], &socket_read_set);
			}
			// Set highest file descriptor to ensure
			// to check all active client sockets
			if (clients[i] > max_socket_descriptor){
				max_socket_descriptor = clients[i];
			}
		}

		// Wait for activity in one of the sockets
		if (select(max_socket_descriptor + 1, &socket_read_set, NULL, NULL, NULL) < 0 && (errno != EINTR)) {
			OutputLog("Failed to read socket content\n");
		}

		// Check for new incomming connection in the master socket
		if (FD_ISSET(serverData.socketMaster, &socket_read_set)) {
			if ((new_client = accept(serverData.socketMaster, (struct sockaddr*)&cli,(socklen_t*)&CLI_LEN)) == -1) {
				Fatal("Failed to connect client\n");
			}
			connectionNew(new_client, clients, &cli);
		}

		// Handles already connected client communication
		Message messageReceived = {0};
		for (int i = 0; i < MAX_CLIENTS; ++i) {
			// Next client if any message is found
			if (!FD_ISSET(clients[i], &socket_read_set)) continue;

			// Clear the messageReceived before reading new data
			memset(&messageReceived, 0, sizeof(messageReceived));

			// Read content from socket
			ssize_t valread = 0;
			if ((valread = read(clients[i], &messageReceived, sizeof(messageReceived))) == -1) {
				Fatal("Failed to read socket content\n");
			}

			// Handle disconnected clients
			if (valread == 0) {
				connectionClose(&clients[i], CLI_LEN, &cli, &address);
				continue;
			}

			// Echo back received message to all clients
			messagebroadcast(&clients[0], clients[i], &messageReceived);
		}
	}
}

int main(int argc, char *argv[])
{
	// Avoid missing arguments
	if (argc != 3){
		Fatal("Usage: %s [server_ip] [port]\n", argv[0]);
	}

	// Check username
	regex_t regex;
	if (regcomp(&regex, "^[a-zA-Z0-9]+$", REG_EXTENDED) != 0) {
		Fatal("Regex compilation failed\n");
	}
	regfree(&regex);

	// Define IP
	const char *IP = argv[1];
	strncpy(serverData.ip, IP, sizeof(serverData.ip));
	// Check and define server PORT
	const char *PORT = argv[2];
	unsigned int PORT_VAL = atoi(PORT);
	if (PORT_VAL < 1 || PORT_VAL > 65535) {
		Fatal("Invalid port number. Port must be between 1 and 65535\n");
	}
	serverData.port = PORT_VAL;

	OutputLog("Starting server...\n");
	serverInitialize(); 

	/* Closing the connected socket */
	close(serverData.socketMaster);

	return 0;
}
