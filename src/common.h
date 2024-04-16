#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>

#define MAX_IP_SIZE 12
#define MAX_USERNAME_SIZE 32 
#define MAX_MESSAGE_SIZE 4096
#define METADATA_DATE_SIZE 21
#define BACKLOG_SIZE 8
#define MAX_HIST_SIZE 16
#define MAX_CLIENTS 64
#define TERMINATOR 1

// These values are just used to initialize some structs
//#define DEFAULT_ADMIN_NAME "Admin"
#define DEFAULT_IP "127.0.0.1"
#define DEFAULT_PORT 8080

#define EXIT_COMMAND "exit"

#define Fatal(...) printf(__VA_ARGS__); \
                  printf("-----------------------------------------\n"); \
                  perror("[ERROR]");                                   \
                  printf("-----------------------------------------\n"); \
		   exit(EXIT_FAILURE);

#define LogInfo(...) fprintf(stdout, "[+] "); \
    fprintf(stdout, __VA_ARGS__)
#define LogExit(...) fprintf(stderr, "[!] "); \
    fprintf(stderr, __VA_ARGS__); \
    exit(EXIT_FAILURE)

#define DEBUG

#ifdef DEBUG
#define _Debug(code) \
    do { \
        code \
    } while(0)
#else
#define _Debug(code)
#endif

// 
// Structure to hold general server info.
// 
typedef struct {
	const uint8_t PROTOCOL;
	int8_t socket_master;
	char ip[12];
        uint16_t port;
	bool is_connected;
} serverinfo_t;

// 
// Structure to hold user info.
// 
typedef struct {
	char username[MAX_USERNAME_SIZE];
} userinfo_t;

//// 
//// Structure to hold client info.
//// 
//typedef struct ClientInfo {
//    int socket;
//    userinfo_t user;
//    char ip[MAX_IP_SIZE];
//    char date[METADATA_DATE_SIZE];
//} ClientInfo;

// 
// Structure to hold message info.
// 
typedef struct message_t {
	bool has_content;
        char date[METADATA_DATE_SIZE];
        userinfo_t user_data;
        char message[MAX_MESSAGE_SIZE];
}message_t;

#endif // COMMON_H
