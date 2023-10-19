#ifndef CLIENT_H
#define CLIENT_H

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

#endif // CLIENT_H
