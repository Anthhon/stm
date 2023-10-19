#ifndef COMMON_H
#define COMMON_H

#define MAX_IP_SIZE 12
#define MAX_USERNAME_SIZE 32 
#define MAX_MESSAGE_SIZE 4096
#define METADATA_DATE_SIZE 21
#define BACKLOG_SIZE 8
#define MAX_HIST_SIZE 32
#define MAX_CLIENTS 64
#define TERMINATOR 1

// These values are just used to initialize some structs
//#define DEFAULT_ADMIN_NAME "Admin"
#define DEFAULT_IP "127.0.0.1"
#define DEFAULT_PORT 8080

#define EXIT_COMMAND "exit"

#define Fatal(...) fprintf(stderr, __VA_ARGS__); \
		   exit(EXIT_FAILURE);
#define OutputInfo(...) fprintf(stdout, "[+] "); fprintf(stdout, __VA_ARGS__);
#define OutputLog(...) fprintf(stdout, "[LOG] "); fprintf(stdout, __VA_ARGS__);

/**
 * @brief Structure to hold server info.
 */
typedef struct ServerInfo {
	const uint8_t PROTOCOL;
	int8_t socketMaster;
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

///**
// * @brief Structure to hold client info.
// */
//typedef struct ClientInfo {
//    int socket;
//    UserInfo user;
//    char ip[MAX_IP_SIZE];
//    char date[METADATA_DATE_SIZE];
//} ClientInfo;

/**
 * @brief Structure to hold message info.
 */
typedef struct Message {
	bool has_content;
        char date[METADATA_DATE_SIZE];
        UserInfo user_data;
        char message[MAX_MESSAGE_SIZE];
}Message;

#endif // COMMON_H
