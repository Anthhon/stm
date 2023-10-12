/**
 * @file common.c
 * @brief Simple chat server implementation using sockets and threads.
 *
 * This file contains the implementation of a basic chat server. It uses TCP sockets for
 * communication and utilizes threads to handle reading and writing messages concurrently.
 * The server can handle multiple clients, allowing them to exchange messages until a client
 * sends the "Exit" message, indicating their desire to disconnect from the server.
 */

#include<arpa/inet.h>
#include<stdbool.h>
#include<netinet/in.h>
#include<sys/types.h>
#include<pthread.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
#include<unistd.h>

#include"colors.h"
#include"common.h"

pthread_mutex_t output_mutex = PTHREAD_MUTEX_INITIALIZER;

// Initialize metadata info
ServerMetadata serverData = {
	.PROTOCOL = AF_INET,
	.BACKLOG_SIZE = 32,
	.socket_handler = 0,
	.ip = DEFAULT_IP,
        .port = DEFAULT_PORT,
	.socket_connected = false,
};
UserMetadata userData = {0};
MessageMetadata messageData = {0};

void message_build(char *dest)
{
	// Get date
	time_t currentTime;
	struct tm *localTime;

	currentTime = time(NULL);
	localTime = localtime(&currentTime);
	strftime(messageData.date, sizeof(messageData.date), "%m-%d", localTime);

	// Append user data to message
	memcpy(&messageData.user_data, &userData, sizeof(userData));
	// Serialize message into buffer
	memcpy(dest, &messageData, sizeof(messageData));
}

void chat_read(void)
{
	MessageMetadata messageReceived = {0};

	// Read and print-out received message
	while(serverData.socket_connected){
		int valread = 0;
		if ((valread = read(serverData.socket_handler, &messageReceived, sizeof(messageReceived))) == -1) {
			Fatal("Failed to read content from socket\n");
		}

		// Check if any text was read from socket
		if (valread){
			pthread_mutex_lock(&output_mutex);
			printf("\r"); // Clear current line
			fprintf(stdout, "[%s] %s%s:%s %s", messageReceived.date, BGRN, messageReceived.user_data.username, CRESET, messageReceived.message);
			
			// Reproduce the line user input line
			printf("\r"); // Clear current line
			fprintf(stdout, "%s%s:%s ", BYEL, userData.username, CRESET);
			fflush(stdout);
			pthread_mutex_unlock(&output_mutex);

			// Clean messageReceived buffer
			memset(&messageReceived, 0, sizeof(messageReceived));
		}
	}
}

void chat_write(void)
{
	// Read server message, insert into buffer and sent it 
	while(serverData.socket_connected){
		// Print username
		pthread_mutex_lock(&output_mutex);
		fprintf(stdout, "%s%s:%s ", BYEL, userData.username, CRESET);
		fflush(stdout);
		pthread_mutex_unlock(&output_mutex);

		// Get new messages
		if (fgets(messageData.message, MAX_MESSAGE_SIZE, stdin) == NULL){
			Fatal("Error while trying to read message\n");
		};

		// Check if msg contains "Exit" then quit server 
		if (strncmp("Exit", messageData.message, 4) == 0){
			fprintf(stdout, "Exiting server...\n");
			serverData.socket_connected = false;
		}

		
		// Build and send message 
		char messageToSend[sizeof(messageData)];

		message_build(messageToSend);
		write(serverData.socket_handler, messageToSend, sizeof(messageToSend));

		// Clean message buffers
		memset(messageData.message, 0, sizeof(messageData.message));
		memset(messageToSend, 0, sizeof(messageToSend));
	}
}

void chat_start(void)
{
	// Create and designate thread functions to read and write content in/to socket 
	pthread_t t1, t2;

	// Assign a thread to each process
	if (pthread_create(&t1, NULL, (void *) chat_write, NULL) != 0){
		Fatal("Failed to create new thread\n");
	}
	if (pthread_create(&t2, NULL, (void *) chat_read, NULL)){
		Fatal("Failed to create new thread\n");
	}

	// Wait till both threads end
	pthread_join(t1, NULL);
	pthread_join(t2, NULL);
}
