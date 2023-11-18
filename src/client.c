/**
 * @file common.c
 * @brief Simple chat server implementation using sockets and threads.
 *
 * This file contains the implementation of a basic chat server. It uses TCP sockets for
 * communication and utilizes threads to handle reading and writing messages concurrently.
 * The server can handle multiple clients, allowing them to exchange messages until a client
 * sends the "Exit" message, indicating their desire to disconnect from the server.
 */

#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <regex.h>
#include <sys/types.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <termios.h>

// Termui is a personal TUI library, to see more check:
// https://github.com/Anthhon/termui
#include "libs/termui/termui.h"

#include "colors.h"
#include "common.h"
#include "client.h"

pthread_mutex_t outputMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t historyMutex = PTHREAD_MUTEX_INITIALIZER;

terminalInfo term_info = {0};

ServerInfo serverData = {
       .PROTOCOL = AF_INET,
       .socketMaster = 0,
       .ip = DEFAULT_IP,
       .port = DEFAULT_PORT,
       .isConnected = false,
};

UserInfo userData = {0};
Message messageData = {0};

Message messageHistory[MAX_HIST_SIZE] = {0};

void handleCommand(char *command)
{
       // Check for different commands
       if (strncmp(EXIT_COMMAND, command, strlen(EXIT_COMMAND)) == 0) {
               printf("Exiting server...\n");
               serverData.isConnected = false;
               exit(EXIT_SUCCESS);
               return;
       }
       return;
}

void messageBuild(char *dest)
{
       // Get date
       time_t currentTime;
       struct tm *localTime;

       currentTime = time(NULL);
       localTime = localtime(&currentTime);
       strftime(messageData.date, sizeof(messageData.date), "%Y-%m-%d %H:%M:%S", localTime);
       messageData.has_content = true;

       // Append user data to message
       memcpy(&messageData.user_data, &userData, sizeof(userData));
       // Serialize message into buffer
       memcpy(dest, &messageData, sizeof(messageData));
}

void messagePrint(Message *messageReceived)
{
       pthread_mutex_lock(&historyMutex);

       // Free space to new message into history
       for (int i = MAX_HIST_SIZE - 2; i >= 0; --i) {
               // Next iteration if message is empty
               if (!messageHistory[i].has_content) {
		       continue;
	       }

               // Clear previous message
               memset(&messageHistory[i + 1], 0, sizeof(Message)); 
               // Move actual message to the previous field
               memcpy(&messageHistory[i + 1], &messageHistory[i], sizeof(Message));
       }
       // Append new message to hist
       memcpy(&messageHistory[0], messageReceived, sizeof(Message));

       pthread_mutex_lock(&outputMutex);
       // Clear last messages
       for (int i = 0; i <= MAX_HIST_SIZE; ++i) {
	       ClearLine(i);
       }

       // Print all messages inside history
       MoveTo(0, 0); // Go back to first line
       for (int i = MAX_HIST_SIZE; i >= 0; --i) {
               // Next iteration if message is empty
               if (messageHistory[i].has_content) {
                       // Lock output file to print message
                       printf("[%s] %s%s:%s %s", \
                                       messageHistory[i].date, BGRN, \
                                       messageHistory[i].user_data.username, CRESET, \
                                       messageHistory[i].message);
               }
       }

       // Return to user input line
       ClearLine(MAX_HIST_SIZE + 2);
       printf("%s%s:%s ", BYEL, userData.username, CRESET);
       fflush(stdout);

       pthread_mutex_unlock(&outputMutex);
       pthread_mutex_unlock(&historyMutex);
}

void chatRead(void)
{
       Message messageReceived = {0};
       ssize_t value_read = 0;

       // Read and print-out received message
       while(serverData.isConnected){
               if ((value_read = read(serverData.socketMaster, &messageReceived, sizeof(Message))) == -1) {
                       Fatal("Failed to read content from socket\n");
               }

               // Check if any text was read from socket
               if (!value_read) {
		       continue;
	       }

	       // Print messageReceived and clean buffer
	       messagePrint(&messageReceived);
	       memset(&messageReceived, 0, sizeof(messageReceived));
	       }
}

void chatWrite(void)
{
       // Print username
       pthread_mutex_lock(&outputMutex);
       printf("%s%s:%s ", BYEL, userData.username, CRESET);
       pthread_mutex_unlock(&outputMutex);

       // Read server message, insert into buffer and sent it 
       while(serverData.isConnected){
               // Get new messages
               if (fgets(messageData.message, MAX_MESSAGE_SIZE, stdin) == NULL){
                       Fatal("Error while trying to read message\n");
               };

               // Check for a command call
               if (strncmp("/", messageData.message, 1) == 0) {
                       char *command = messageData.message + 1;
                       handleCommand(command);
               }
               
               // Build message into 'messageToSend' and send it
               char messageToSend[sizeof(messageData)];

               messageBuild(messageToSend);
               write(serverData.socketMaster, messageToSend, sizeof(Message));

               // Add message to history and print it all
               messagePrint(&messageData);

               // Clean message buffers
               memset(messageData.message, 0, sizeof(messageData.message));
               memset(messageToSend, 0, sizeof(Message));
       }
}

void chatStart(void)
{
       // Create threads
       pthread_t chatWriteThread, chatReadThread;

       // Assign a thread to each process
       printf("\e[1;1H\e[2J"); // Clear terminal
       if (pthread_create(&chatWriteThread, NULL, (void *) chatWrite, NULL) != 0){
               Fatal("Failed to create new thread\n");
       }
       if (pthread_create(&chatReadThread, NULL, (void *) chatRead, NULL)){
               Fatal("Failed to create new thread\n");
       }

       // Wait till both threads end
       pthread_join(chatWriteThread, NULL);
       pthread_join(chatReadThread, NULL);
}

void clientConnect(const char *IP)
{
       struct sockaddr_in address;
       memset(&address, 0, sizeof(address));
       
       // Creating socket descriptor 
       OutputInfo("Building connection to socket\n");
       if ((serverData.socketMaster = socket(AF_INET, SOCK_STREAM, 0)) == -1){
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
       if (connect(serverData.socketMaster,(struct sockaddr*)&address, sizeof(address)) == -1) {
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
       clientConnect(serverData.ip);

       // Init TUI library
       termuiInit(&term_info);

       OutputInfo("Starting chat as %s...\n", userData.username);
       chatStart();

       /* Closing the connected socket */
       close(serverData.isConnected);

       return(0);
}
