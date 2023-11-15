/* This file should only deal with control flow in server-side */

#include <arpa/inet.h>
#include <asm-generic/errno-base.h>
#include <bits/pthreadtypes.h>
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

pthread_mutex_t outputMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t clientsMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t socketSetMutex = PTHREAD_MUTEX_INITIALIZER;

// Data structure to store server information
ServerInfo serverData = {
	.PROTOCOL = AF_INET,
	.socketMaster = 0,
	.ip = DEFAULT_IP,
        .port = DEFAULT_PORT,
	.isConnected = false,
};

struct sockaddr_in address, cli;
fd_set socket_read_set;
const int CLI_LEN = sizeof(cli);
int clients[MAX_CLIENTS] = {0};
volatile int connected_clients = 0;

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
	pthread_mutex_lock(&outputMutex);
	OutputInfo("Client disconnected!\n\t- IP: %s\n\t- Port: %d\n", inet_ntoa(address->sin_addr), ntohs(address->sin_port));
	pthread_mutex_unlock(&outputMutex);

	// Close client socket
	close(*client);
	*client = 0;
	--connected_clients;
}

// Handle new client connections
void connectionNew(int new_client, struct sockaddr_in *clientAddress)
{
	for (int i = 0; i < MAX_CLIENTS; ++i) {
		pthread_mutex_lock(&clientsMutex);
		if (clients[i] == 0) {
			clients[i] = new_client;
			//memcpy(clients[i], cli, sizeof(struct sockaddr_in));
			pthread_mutex_lock(&outputMutex);
			OutputInfo("New client connected!\n\t- IP: %s\n\t- Port: %d\n\t- SocketFd: %d\n", 
					inet_ntoa(clientAddress->sin_addr), ntohs(clientAddress->sin_port), new_client);
			pthread_mutex_unlock(&outputMutex);
			break;
		}
		pthread_mutex_unlock(&clientsMutex);
	}
	++connected_clients;
}

// Handles already connected client communication
void handleCommunication(void)
{
	while(true) {
		// Stop communication processes in case anyone is connected
		if (!connected_clients) continue;

		Message messageReceived = {0};

		for (int i = 0; i < MAX_CLIENTS; ++i) {
			// Next client if any message is found
			pthread_mutex_lock(&socketSetMutex);
			if (!FD_ISSET(clients[i], &socket_read_set)) {
				continue;
			}
			pthread_mutex_unlock(&socketSetMutex);

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
			pthread_mutex_lock(&outputMutex);
			OutputLog("Broadcasting received message...\n");
			pthread_mutex_unlock(&outputMutex);
			messagebroadcast(&clients[0], clients[i], &messageReceived);
		}
	}
}

// Handle new users connection
void handleConnection(void)
{
	struct sockaddr_in clientAddress;
	socklen_t clientAddressLength = sizeof(clientAddress);

	pthread_mutex_lock(&outputMutex);
	OutputLog("Server initialized, waiting for clients to connect...\n");
	pthread_mutex_unlock(&outputMutex);
	while(true) {
		int new_client;
		pthread_mutex_lock(&socketSetMutex);
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
		pthread_mutex_unlock(&socketSetMutex);

		// Wait for activity in one of the sockets
		if (select(max_socket_descriptor + 1, &socket_read_set, NULL, NULL, NULL) < 0 && (errno != EINTR)) {
			pthread_mutex_lock(&outputMutex);
			OutputLog("Failed to read socket content\n");
			pthread_mutex_unlock(&outputMutex);
		}

		// Check for new incomming connection in the master socket
		pthread_mutex_lock(&socketSetMutex);
		if (FD_ISSET(serverData.socketMaster, &socket_read_set)) {
			pthread_mutex_lock(&clientsMutex);
			if ((new_client = accept(serverData.socketMaster, (struct sockaddr*)&clientAddress,(socklen_t*)&clientAddressLength)) == -1) {
				Fatal("Failed to connect client\n");
			}
			connectionNew(new_client, &clientAddress);
			pthread_mutex_unlock(&clientsMutex);
		}
		pthread_mutex_unlock(&socketSetMutex);
	}
}

// Initialize the server and handle client connections
void serverInitialize(void)
{
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

	// Create and assign threads to each process
	pthread_t connectionThread, communicationThread;

	if (pthread_create(&connectionThread, NULL, (void*)handleConnection, NULL) != 0) {
		Fatal("Failed to create new thread");
	}
	if (pthread_create(&communicationThread, NULL, (void*)handleCommunication, NULL) != 0) {
		Fatal("Failed to create new thread");
	}

	// Wait till both threads end
	pthread_join(connectionThread, NULL);
	pthread_join(communicationThread, NULL);
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
