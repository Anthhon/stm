#ifndef COMMON_H
#define COMMON_H

#include<arpa/inet.h>
#include<netinet/in.h>
#include<pthread.h>
#include<regex.h>
#include<sys/types.h>
#include<stdbool.h>
#include<stdint.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
#include<unistd.h>
#include <termios.h>

#define MAX_IP_SIZE 12
#define MAX_USERNAME_SIZE 32 
#define MAX_MESSAGE_SIZE 2048
#define METADATA_DATE_SIZE 8
#define BACKLOG_SIZE 32
#define TERMINATOR 1

#define DEFAULT_ADMIN_NAME "Admin"
#define DEFAULT_IP "127.0.0.1"
#define DEFAULT_PORT 8080

#define EXIT_COMMAND "exit"

#define Fatal(...) fprintf(stdout, __VA_ARGS__); \
		   exit(EXIT_FAILURE);
#define OutputInfo(...) fprintf(stdout, "[+] "); fprintf(stdout, __VA_ARGS__);

/**
 * @brief Structure to hold server info.
 */
typedef struct ServerInfo {
	const uint8_t PROTOCOL;
	int8_t socketHandler;
	char ip[12];
        uint16_t port;
	bool isConnected;
}ServerInfo;

/**
 * @brief Structure to hold user info.
 */
typedef struct UserInfo {
	char username[MAX_USERNAME_SIZE];
}UserInfo;

/**
 * @brief Structure to hold message info.
 */
typedef struct Message {
	bool has_content;
        char date[METADATA_DATE_SIZE];
        UserInfo user_data;
        char message[MAX_MESSAGE_SIZE];
}Message;

/**
 * @brief Handle chat commands
 * 
 * @param command Pointer to the buffer where the command is stored
 */
void handleCommand(char *command);

/**
 * @brief Adds a received message to the chat message history.
 * 
 * This function adds the received message to the chat message history, shifting
 * existing messages to make space for the new message. It also prints all messages
 * inside the history to the standard output.
 * 
 * @param messageReceived Pointer to the received message info.
 */
void messageAddToHistory(Message *messageReceived);

/**
 * @brief Builds a chat message with timestamp and user data.
 * 
 * @param dest Pointer to the destination buffer to store the serialized message.
 */
void messageBuild(char *dest);

/**
 * @brief Reads and prints received messages from the server.
 */
void chatRead(void);

/**
 * @brief Reads user input messages, sends them to the server, and handles user exit.
 */
void chatWrite(void);

/**
 * @brief Starts the chat server by creating threads for reading and writing messages.
 */
void chatStart(void);

#endif
