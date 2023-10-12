#ifndef COMMON_H
#define COMMON_H

#define MAX_USERNAME_SIZE 32 
#define MAX_MESSAGE_SIZE 2048
#define METADATA_DATE_SIZE 8
#define TERMINATOR 1

#define Fatal(...) fprintf(stdout, __VA_ARGS__); \
		   exit(EXIT_FAILURE);
#define OutputInfo(...) fprintf(stdout, "[+] "); fprintf(stdout, __VA_ARGS__);

typedef struct ServerMetadata {
	const int PROTOCOL;
	const int BACKLOG_SIZE;
	int socket_handler;
        unsigned short int port;
	bool socket_connected;
}ServerMetadata;

typedef struct UserMetadata {
	char username[MAX_USERNAME_SIZE];
}UserMetadata;

/**
 * @brief Structure to hold message metadata.
 */
typedef struct MessageMetadata {
        char date[METADATA_DATE_SIZE];
        UserMetadata user_data;
        char message[MAX_MESSAGE_SIZE];
}MessageMetadata;

/**
 * @brief Builds a chat message with timestamp and user data.
 * 
 * @param dest Pointer to the destination buffer to store the serialized message.
 */
void message_build(char *dest);

/**
 * @brief Reads and prints received messages from the server.
 */
void chat_read(void);

/**
 * @brief Reads user input messages, sends them to the server, and handles user exit.
 */
void chat_write(void);

/**
 * @brief Starts the chat server by creating threads for reading and writing messages.
 */
void chat_start(void);

#endif
