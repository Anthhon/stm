#ifndef CLIENT_H
#define CLIENT_H

/**
 * @brief Handle chat commands
 * 
 * @param command Pointer to the buffer where the command is stored
 */
void handleCommand(char *command);

/**
 * @brief Print all received messages at history.
 * 
 * @param messageReceived Pointer to the received message info.
 */
void messagePrint(Message *messageReceived);

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

#endif // CLIENT_H
