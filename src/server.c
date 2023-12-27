/* This file should only deal with control flow in server-side */

#include <arpa/inet.h>
#include <asm-generic/errno-base.h>
#include <bits/pthreadtypes.h>
#include <errno.h>
#include <fcntl.h>
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

fd_set socket_read_set;
int clients[MAX_CLIENTS] = {0};
struct sockaddr_in address, cli;
const int CLI_LEN = sizeof(cli);
volatile int connected_clients = 0;

// Handle new messages from clients
void messageBroadcast(int *clients, int sender, Message *messageReceived)
{
    // Log message
    LogInfo("[%s] %s'%s'%s: %s", \
            messageReceived->date,
            BGRN, messageReceived->user_data.username, CRESET, \
            messageReceived->message);

    // Send message to all clients connected
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i] > 0 && clients[i] != sender) {
            int broadcast_status = 0;
            if ((broadcast_status = send(clients[i], messageReceived, sizeof(*messageReceived), 0)) == -1) {
                LogInfo("Error sending message => [%s] %s'%s'%s: %s", \
                        messageReceived->date, \
                        BGRN, messageReceived->user_data.username, CRESET, \
                        messageReceived->message);
            }
        }
    }
}

// Close the connection with a client
void connectionClose(int *client, int CLI_LEN, struct sockaddr_in *cli, struct sockaddr_in *address)
{
    // Log disconnected client details
    getpeername(*client, (struct sockaddr*)&cli, (socklen_t*)&CLI_LEN);
    pthread_mutex_lock(&outputMutex);
    LogInfo("Client disconnected!\n\t- IP: %s\n\t- Port: %d\n", inet_ntoa(address->sin_addr), ntohs(address->sin_port));
    pthread_mutex_unlock(&outputMutex);

    // Close client socket
    close(*client);
    *client = 0;
    --connected_clients;
}

// Handle new client connections
void connectionNew(int new_client, struct sockaddr_in *clientAddress)
{
    ++connected_clients;
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i] == 0) {
            clients[i] = new_client;
            //memcpy(clients[i], cli, sizeof(struct sockaddr_in));
            pthread_mutex_lock(&outputMutex);
            LogInfo("New client connected!\n\t- IP: %s\n\t- Port: %d\n\t- SocketFd: %d\n", 
                    inet_ntoa(clientAddress->sin_addr), ntohs(clientAddress->sin_port), new_client);
            pthread_mutex_unlock(&outputMutex);
            break;
        }
    }
}

// Handles already connected client communication
void handleCommunication(void)
{
    while(true) {
        // Freezes communication proccess until someone is connected,
        // removing this could cause thread lock at server start
        if (!connected_clients) continue;

        // Search messages sent by all clients
        Message messageReceived = {0};

        for (int i = 0; i < MAX_CLIENTS; ++i) {
            // Next client if any message is found
            if (!FD_ISSET(clients[i], &socket_read_set)) {
                continue;
            }

            // Clear the messageReceived before reading new data
            memset(&messageReceived, 0, sizeof(messageReceived));

            int flags = fcntl(clients[i], F_GETFL, 0);
            fcntl(clients[i], F_SETFL, flags | O_NONBLOCK);

            ssize_t valread = read(clients[i], &messageReceived, sizeof(messageReceived));
            if (valread == -1) {
                if (errno != EAGAIN && errno != EWOULDBLOCK) {
                    LogExit("Failed to read socket content\n");
                }
                // No data available at this moment
            } else if (valread == 0) {
                // Handle client disconnection
                connectionClose(&clients[i], CLI_LEN, &cli, &address);
                continue;
            } else {
                // Process received data
                pthread_mutex_lock(&outputMutex);
                messageBroadcast(&clients[0], clients[i], &messageReceived);
                pthread_mutex_unlock(&outputMutex);
            }
        }
    }
}

// Handle new users connection
void handleConnection(void)
{
    struct sockaddr_in clientAddress;
    socklen_t clientAddressLength = sizeof(clientAddress);

    pthread_mutex_lock(&outputMutex);
    LogInfo("Server initialized, waiting for clients to connect...\n");
    pthread_mutex_unlock(&outputMutex);

    while(true) {
        int new_client;
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
            pthread_mutex_lock(&outputMutex);
            LogExit("Failed to read socket content\n");
            pthread_mutex_unlock(&outputMutex);
        }

        // Check for new incomming connection in the master socket
        pthread_mutex_lock(&socketSetMutex);
        if (FD_ISSET(serverData.socketMaster, &socket_read_set)) {
            pthread_mutex_lock(&clientsMutex);

            if ((new_client = accept(serverData.socketMaster, (struct sockaddr*)&clientAddress,(socklen_t*)&clientAddressLength)) == -1) {
                LogExit("Failed to connect client\n");
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
    LogInfo("Building connection to socket\n");
    if ((serverData.socketMaster = socket(serverData.PROTOCOL, SOCK_STREAM, 0)) == -1){
        LogExit("Socket creation failed\n");
    }

    // Attach socket to given serverData.port 
    memset(&address, 0, sizeof(address));
    address.sin_family = serverData.PROTOCOL;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(serverData.port);

    LogInfo("Server using protocol %i\n", serverData.PROTOCOL);
    LogInfo("Server using port %i\n", serverData.port);

    LogInfo("Binding server to socket\n");
    if (bind(serverData.socketMaster, (struct sockaddr*)&address, sizeof(address)) == -1){
        LogExit("Socket binding failed\n");
    }

    if (listen(serverData.socketMaster, BACKLOG_SIZE) == -1){
        LogExit("Socket listening failed\n");
    }

    // Create and assign threads to each process
    pthread_t connectionThread, communicationThread;

    if (pthread_create(&connectionThread, NULL, (void*)handleConnection, NULL) != 0) {
        LogExit("Failed to create new thread");
    }
    if (pthread_create(&communicationThread, NULL, (void*)handleCommunication, NULL) != 0) {
        LogExit("Failed to create new thread");
    }

    // Wait till both threads end
    pthread_join(connectionThread, NULL);
    pthread_join(communicationThread, NULL);
}

int main(int argc, char *argv[])
{
    // Avoid missing arguments
    if (argc != 3){
        fprintf(stdout, "Usage: %s [server_ip] [port]\n", argv[0]);
        exit(EXIT_SUCCESS);
    }

    // Check username
    regex_t regex;
    if (regcomp(&regex, "^[a-zA-Z0-9]+$", REG_EXTENDED) != 0) {
        LogExit("Regex compilation failed\n");
    }
    regfree(&regex);

    // Define IP
    const char *IP = argv[1];
    strncpy(serverData.ip, IP, sizeof(serverData.ip));
    // Check and define server PORT
    const char *PORT = argv[2];
    unsigned int PORT_VAL = atoi(PORT);
    if (PORT_VAL < 1 || PORT_VAL > 65535) {
        LogExit("Invalid port number. Port must be between 1 and 65535\n");
    }
    serverData.port = PORT_VAL;

    LogInfo("Starting server...\n");
    serverInitialize(); 

    /* Closing the connected socket */
    close(serverData.socketMaster);

    return 0;
}
