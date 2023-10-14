/**
 * @file common.c
 * @brief Simple chat server implementation using sockets and threads.
 *
 * This file contains the implementation of a basic chat server. It uses TCP sockets for
 * communication and utilizes threads to handle reading and writing messages concurrently.
 * The server can handle multiple clients, allowing them to exchange messages until a client
 * sends the "Exit" message, indicating their desire to disconnect from the server.
 */

#include"colors.h"
#include"common.h"

pthread_mutex_t outputMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t historyMutex = PTHREAD_MUTEX_INITIALIZER;


// Initialize metadata info
ServerInfo serverData = {
	.PROTOCOL = AF_INET,
	.socketHandler = 0,
	.ip = DEFAULT_IP,
        .port = DEFAULT_PORT,
	.isConnected = false,
};

UserInfo userData = {0};
Message messageData = {0};

size_t messageHistorySize = 0;
Message messageHistory[BACKLOG_SIZE] = {0};

void handleCommand(char *command)
{
	// Check for different commands
	if (strncmp(EXIT_COMMAND, command, strlen(EXIT_COMMAND)) == 0) {
		fprintf(stdout, "Exiting server...\n");
		serverData.isConnected = false;
		exit(EXIT_SUCCESS);
		return;
	}
	return;
}

void messagePrint(Message *messageReceived)
{
	// Free space to new message into history
	pthread_mutex_lock(&historyMutex);
	for (int i = BACKLOG_SIZE - 2; i >= 0; --i) {
		// Next iteration if message is empty
		if (!messageHistory[i].has_content) continue;

		// Clear previous message
		memset(&messageHistory[i + 1], 0, sizeof(Message)); 
		// Move actual message to the previous field
		memcpy(&messageHistory[i + 1], &messageHistory[i], sizeof(Message));
	}

	// Append new message to hist
	memcpy(&messageHistory[0], messageReceived, sizeof(Message));

	// Print all messages inside history
	pthread_mutex_lock(&outputMutex);

	// Clear last messages
	for (int i = 0; i <= BACKLOG_SIZE; ++i) {
		(void)printf("\e[%d;1H", i);
		(void)printf("\e[K\r");
	}

	printf("\e[0;1H"); // Go back to first line
	for (int i = BACKLOG_SIZE; i >= 0; --i) {
		// Next iteration if message is empty
		if (messageHistory[i].has_content) {
			// Lock output file to print message
			fprintf(stdout, "[%s] %s%s:%s %s", \
					messageHistory[i].date, BGRN, \
					messageHistory[i].user_data.username, CRESET, \
					messageHistory[i].message);
		}
	}
	pthread_mutex_unlock(&historyMutex);
	// Return to user input line
	printf("\e[%d;%dH", BACKLOG_SIZE + 2, (unsigned)strlen(userData.username) + 2);
	//fprintf(stdout, "%s%s:%s ", BYEL, userData.username, CRESET);
	fflush(stdout);
	pthread_mutex_unlock(&outputMutex);
}

void messageBuild(char *dest)
{
	// Get date
	time_t currentTime;
	struct tm *localTime;

	currentTime = time(NULL);
	localTime = localtime(&currentTime);
	strftime(messageData.date, sizeof(messageData.date), "%m-%d", localTime);
	messageData.has_content = true;

	// Append user data to message
	memcpy(&messageData.user_data, &userData, sizeof(userData));
	// Serialize message into buffer
	memcpy(dest, &messageData, sizeof(messageData));
}

void chatRead(void)
{
	Message messageReceived = {0};

	// Read and print-out received message
	while(serverData.isConnected){
		ssize_t valread = 0;
		if ((valread = read(serverData.socketHandler, &messageReceived, sizeof(messageReceived))) == -1) {
			Fatal("Failed to read content from socket\n");
		}

		// Check if any text was read from socket
		if (valread){
			// Print messageReceived and clean buffer
			messagePrint(&messageReceived);
			memset(&messageReceived, 0, sizeof(messageReceived));
		}
	}
}

void chatWrite(void)
{
	// Print username
	pthread_mutex_lock(&outputMutex);
	printf("%s%s:%s ", BYEL, userData.username, CRESET);
	fflush(stdout);
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

		// Check if msg contains "Exit" then quit server 
		if (strncmp("/exit", messageData.message, 5) == 0){
		}
		
		// Build message into 'messageToSend' and send it
		char messageToSend[sizeof(messageData)];

		// Build and send message
		messageBuild(messageToSend);
		write(serverData.socketHandler, messageToSend, sizeof(messageToSend));

		// Clear message input
		printf("\e[%d;1H", BACKLOG_SIZE + 2);
		printf("\e[K\r");
		fprintf(stdout, "%s%s:%s ", BYEL, userData.username, CRESET);

		// Add message to history and print it all
		messagePrint(&messageData);

		// Clean message buffers
		memset(messageData.message, 0, sizeof(messageData.message));
		memset(messageToSend, 0, sizeof(messageToSend));
	}
}

void chatStart(void)
{
	// Create and designate thread functions to read and write content in/to socket 
	pthread_t t1, t2;

	// Assign a thread to each process
	printf("\e[1;1H\e[2J"); // Clear terminal
	if (pthread_create(&t1, NULL, (void *) chatWrite, NULL) != 0){
		Fatal("Failed to create new thread\n");
	}
	if (pthread_create(&t2, NULL, (void *) chatRead, NULL)){
		Fatal("Failed to create new thread\n");
	}

	// Wait till both threads end
	pthread_join(t1, NULL);
	pthread_join(t2, NULL);
}
