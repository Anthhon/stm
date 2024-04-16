// 
// common.c
// Simple chat server implementation using sockets and threads.
// 
// This file contains the implementation of a basic chat server. It uses TCP sockets for
// communication and utilizes threads to handle reading and writing messages concurrently.
// The server can handle multiple clients, allowing them to exchange messages until a client
// sends the "Exit" message, indicating their desire to disconnect from the server.
// 

#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <regex.h>
#include <sys/types.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <termios.h>

// Termui is a personal TUI library, to see more check:
// https://github.com/Anthhon/termui
#include "libs/termui/termui.h"

#include "message.h"
#include "client.h"
#include "colors.h"
#include "common.h"

static terminalInfo term_info;
userinfo_t clientData = {0};
pthread_mutex_t outputMutex = PTHREAD_MUTEX_INITIALIZER;

serverinfo_t serverData = {
    .PROTOCOL = AF_INET,
    .socket_master = 0,
    .ip = DEFAULT_IP,
    .port = DEFAULT_PORT,
    .is_connected = false,
};

static void handleCommand(char *command)
{
    // Check for different commands
    if (strncmp(EXIT_COMMAND, command, strlen(EXIT_COMMAND)) == 0) {
        serverData.is_connected = false;
        LogExit("Exiting server...\n");
    }
    return;
}

void clientRead(void)
{
    message_t messageReceived = {0};
    ssize_t value_read = 0;

    // Read and print-out received message
    while(serverData.is_connected){
        if ((value_read = read(serverData.socket_master, &messageReceived, sizeof(message_t))) == -1) {
            Fatal("Failed to read content from socket\n");
        }

        // Check if any text was read from socket
        if (!value_read) {
            continue;
        }

        // Print messageReceived and clean buffer
        messagePrint(&messageReceived);
        memset(&messageReceived, 0, sizeof(message_t));
    }
}

void clientWrite(void)
{
    // Print username
    pthread_mutex_lock(&outputMutex);
    printf("%s%s:%s ", BYEL, clientData.username, CRESET);
    pthread_mutex_unlock(&outputMutex);

    // Read server message, insert into buffer and sent it 
    while(serverData.is_connected){
        // Get new messages
        if (fgets(messageData.message, MAX_MESSAGE_SIZE, stdin) == NULL){
            Fatal("Error while trying to read message\n");
        };

        // Filter message content
        for (int i = 0; i < MAX_MESSAGE_SIZE; ++i) {
            if (messageData.message[i] == '\n') messageData.message[i] = '\0';
        }

        // Check for a command call
        if (strncmp("/", messageData.message, 1) == 0) {
            char *command = messageData.message + 1;
            handleCommand(command);
        }

        // Build message into 'messageToSend' and send it
        char messageToSend[sizeof(message_t)];

        messageBuild(messageToSend);
        write(serverData.socket_master, messageToSend, sizeof(message_t));

        // Add message to history and print it all
        messagePrint(&messageData);

        // Clean message buffers
        memset(messageData.message, 0, sizeof(messageData.message));
        memset(messageToSend, 0, sizeof(message_t));
    }
}

void clientStart(void)
{
    // Create threads
    pthread_t clientWriteThread, clientReadThread;

    // Assign a thread to each process
    printf("\e[1;1H\e[2J"); // Clear terminal
    if (pthread_create(&clientWriteThread, NULL, (void *) clientWrite, NULL) != 0){
        Fatal("Failed to create new thread\n");
    }
    if (pthread_create(&clientReadThread, NULL, (void *) clientRead, NULL)){
        Fatal("Failed to create new thread\n");
    }

    // Wait till both threads end
    pthread_join(clientWriteThread, NULL);
    pthread_join(clientReadThread, NULL);
}

void clientConnect(const char *IP)
{
    struct sockaddr_in address;
    memset(&address, 0, sizeof(struct sockaddr_in));

    // Creating socket descriptor 
    LogInfo("Building connection to socket\n");
    if ((serverData.socket_master = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        Fatal("socket creation failed");
    }
    address.sin_family = AF_INET;
    address.sin_port = htons(serverData.port);

    // Assign IP 
    LogInfo("Assigning IP to socket\n");
    if (inet_aton(IP, &(address.sin_addr)) == 0){
        Fatal("invalid IP address\n");
    }

    // Initiate connection on a socket 
    if (connect(serverData.socket_master,(struct sockaddr*)&address, sizeof(struct sockaddr_in)) == -1) {
        Fatal("Socket connection failed\n");
    }

    // Declare that the connection has been made
    serverData.is_connected = true;
}

int main(int argc, char const *argv[])
{
    // Avoid missing arguments
    if (argc != 4){
        Fatal("Usage: %s [server_ip] [username] [port]\n", argv[0]);
    }

    // Check and define username
    regex_t regex;
    const char *USERNAME = argv[2];
    if (regcomp(&regex, "^[a-zA-Z0-9]+$", REG_EXTENDED) != 0) {
        LogExit("Regex compilation failed\n");
    }
    if (regexec(&regex, USERNAME, 0, NULL, 0) != 0) {
        Fatal("Invalid username format. Use only alphanumeric characters\n");
    }
    regfree(&regex);

    const size_t username_len = strlen(USERNAME);
    if (username_len > MAX_USERNAME_SIZE - TERMINATOR) {
        LogExit("Username \"%s\" is too long\n", argv[2]);
    }
    strncpy(clientData.username, USERNAME, sizeof(clientData.username));

    // Define IP
    const char *IP = argv[1];
    strncpy(serverData.ip, IP, sizeof(serverData.ip));

    // Check and define server PORT
    const char *PORT = argv[3];
    unsigned int PORT_VAL = atoi(PORT);
    if (PORT_VAL < 1 || PORT_VAL > 65535) {
        LogExit("Invalid port number. Port must be between 1 and 65535\n");
    }
    serverData.port = PORT_VAL;

    LogInfo("Connecting to the server %s...\n", serverData.ip);
    clientConnect(serverData.ip);

    // Init TUI library
    termuiInit(&term_info);

    LogInfo("Starting chat as %s...\n", clientData.username);
    clientStart();

    // Closing the connected socket
    close(serverData.is_connected);

    return(0);
}
