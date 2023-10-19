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

ServerInfo serverData = {
	.PROTOCOL = AF_INET,
	.socketMaster = 0,
	.ip = DEFAULT_IP,
        .port = DEFAULT_PORT,
	.isConnected = false,
};

void messageNew(int *clients, int sender, Message *messageReceived)
{
	for (int i = 0; i < MAX_CLIENTS; ++i) {
		if (clients[i] > 0 && clients[i] != sender) {
			// Send message to all clients connected
			send(clients[i], messageReceived, sizeof(*messageReceived), 0);
			// Create message log
			fprintf(stdout, "[%s] %s'%s'%s: %s", \
                                       messageReceived->date, BGRN, \
                                       messageReceived->user_data.username, CRESET, \
                                       messageReceived->message);
		}
	}
}

void connectionClose(int client, int CLI_LEN, size_t *client_count, struct sockaddr_in *cli, struct sockaddr_in *address)
{
	// Get disconnected client details
	getpeername(client, (struct sockaddr*)&cli, (socklen_t*)&CLI_LEN);
	OutputInfo("Client disconnected!\n\t- IP: %s\n\t- Port: %d\n", inet_ntoa(address->sin_addr), ntohs(address->sin_port));

	// Close client socket
	close(client);
	client = 0;
	--client_count;
}

void connectionNew(int new_socket, int clients[MAX_CLIENTS], struct sockaddr_in* cli, size_t* client_count)
{
	for (int i = 0; i < MAX_CLIENTS; ++i) {
		if (clients[i] == 0) {
			clients[i] = new_socket;
			OutputInfo("New client connected!\n\t- IP: %s\n\t- Port: %d\n\t- SocketFd: %d\n", 
					inet_ntoa(cli->sin_addr), ntohs(cli->sin_port), new_socket);
			(*client_count)++;
			break;
		}
	}
}

void serverInitialize(void)
{
	struct sockaddr_in address, cli;
	const int CLI_LEN = sizeof(cli);
	int clients[MAX_CLIENTS] = {0};
	int new_socket;
	size_t client_count = 0;
	fd_set readfds;

	// Creating socket descriptor 
	OutputInfo("Building connection to socket...\n");
	if ((serverData.socketMaster = socket(serverData.PROTOCOL, SOCK_STREAM, 0)) == -1){
		Fatal("Socket creation failed\n");
	}

	// Attaching socket to given serverData.port
	memset(&address, 0, sizeof(address));
	address.sin_family = serverData.PROTOCOL;
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	address.sin_port = htons(serverData.port);

	// Attach socket to given serverData.port 
	OutputInfo("Building server...\n");
	if (bind(serverData.socketMaster, (struct sockaddr*)&address, sizeof(address)) == -1){
		Fatal("Socket binding failed\n");
	}

	if (listen(serverData.socketMaster, BACKLOG_SIZE) == -1){
		Fatal("Socket listening failed\n");
	}
	OutputInfo("Server initialized waiting for clients to connect...\n");

	// Handle multi-user connection
	while(true) {
		FD_ZERO(&readfds); // Clear socket set
		FD_SET(serverData.socketMaster, &readfds); // Add master socket to set

		// Add child socket to set
		int max_sd = serverData.socketMaster;
		for (int i = 0; i < MAX_CLIENTS; ++i) {
			// Add socket to read list if valid
			if (clients[i] > 0) {
				FD_SET(clients[i], &readfds);
			}
			// Highest file descriptor
			if (clients[i] > max_sd){
				max_sd = clients[i];
			}
		}

		// Wait for activity in one of the sockets
		if (select(max_sd + 1, &readfds, NULL, NULL, NULL) < 0 && (errno != EINTR)) {
			OutputInfo("Failed to read socket content\n");
		}

		// Check for new incomming connection in the master socket
		if (FD_ISSET(serverData.socketMaster, &readfds)) {
			if ((new_socket = accept(serverData.socketMaster, (struct sockaddr*)&cli,(socklen_t*)&CLI_LEN)) == -1) {
				Fatal("Failed to connect client\n");
			}
			connectionNew(new_socket, clients, &cli, &client_count);
		}

		// IO operation on other socket
		Message messageReceived = {0};
		for (int i = 0; i < MAX_CLIENTS; ++i) {
			if (FD_ISSET(clients[i], &readfds)) {
				// Clear the messageReceived content before reading new data
				memset(&messageReceived, 0, sizeof(messageReceived));

				// Read content from socket
				ssize_t valread = 0;
				if ((valread = read(clients[i], &messageReceived, sizeof(messageReceived))) == -1) {
					Fatal("Failed to read socket content\n");
				}

				// Handle disconnected clients
				if (valread == 0) {
					connectionClose(clients[i], CLI_LEN, &client_count, &cli, &address);
					continue;
				}

				// Echo back received message to all clients
				messageNew(&clients[0], clients[i], &messageReceived);
			}
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

	OutputInfo("Starting server...\n");
	serverInitialize(); 

	/* Closing the connected socket */
	close(serverData.socketMaster);

	return 0;
}
