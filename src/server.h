#ifndef SERVER_H
#define SERVER_H

#define WELCOME_MESSAGE "Connected to server successfully!"

void messageNew(int *clients, int sender, Message *messageReceived);
void connectionClose(int *client, int CLI_LEN, struct sockaddr_in *cli, struct sockaddr_in *address);
void connectionNew(int new_socket, struct sockaddr_in* cli);
void serverInitialize(void);

#endif // SERVER_H
