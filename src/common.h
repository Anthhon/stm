#ifndef COMMON_H
#define COMMON_H

#define MAX_USERNAME_SIZE 32 
#define MAX_MESSAGE_SIZE 2048
#define TERMINATOR       1

typedef struct ServerMetadata {
        unsigned short int port;
}ServerMetadata;

typedef struct UserMetadata {
	char username[MAX_USERNAME_SIZE];
}UserMetadata;

// This info is related with message format
// +----------------------------------------+
// |             Date (16 bytes)            |
// +----------------------------------------+
// |            Sender (32 bytes)           |
// +----------------------------------------+
// |                                        |
// |                                        |
// |           Message (964 bytes)          |
// |                                        |
// |                                        |
// +----------------------------------------+
//
// TODO: Plain better message struct
typedef struct MessageMetadata {
        int date;
        UserMetadata user_data;
        char message[MAX_MESSAGE_SIZE];
}MessageMetadata;


void fatal(char *message);
void build_complete_msg(char *dest, char *username, char *message);
void chat_read(void);
void chat_write(void);
void chat_start(void);
void client_connect(const char *IP);
void server_start(void);

#endif
