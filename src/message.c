#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <pthread.h>
#include <string.h>

// Termui is a personal TUI library, to see more check:
// https://github.com/Anthhon/termui
#include "libs/termui/termui.h"

#include "message.h"
#include "client.h"
#include "colors.h"
#include "common.h"

message_t messageData = {0};
message_t messageHistory[MAX_HIST_SIZE] = {0};
static pthread_mutex_t historyMutex = PTHREAD_MUTEX_INITIALIZER;

void messageBuild(char *dest)
{
    // Get date
    time_t currentTime;
    struct tm *localTime;

    currentTime = time(NULL);
    localTime = localtime(&currentTime);
    strftime(messageData.date, sizeof(messageData.date), "%Y-%m-%d %H:%M:%S", localTime);
    messageData.has_content = true;

    // Append user data to message
    memcpy(&messageData.user_data, &clientData, sizeof(userinfo_t));
    // Serialize message into buffer
    memcpy(dest, &messageData, sizeof(message_t));
}

void messagePrint(message_t *messageReceived)
{
    pthread_mutex_lock(&historyMutex);

    // Free space to new message into history
    for (int i = MAX_HIST_SIZE - 2; i >= 0; --i) {
        // Next iteration if message is empty
        if (!messageHistory[i].has_content) {
            continue;
        }

        // Clear previous message
        memset(&messageHistory[i + 1], 0, sizeof(message_t)); 
        // Move actual message to the previous field
        memcpy(&messageHistory[i + 1], &messageHistory[i], sizeof(message_t));
    }
    // Append new message to hist
    memcpy(&messageHistory[0], messageReceived, sizeof(message_t));

    pthread_mutex_lock(&outputMutex);
    // Clear last messages
    for (int i = 0; i <= MAX_HIST_SIZE; ++i) {
        ClearLine(i);
    }

    // Print all messages inside history
    MoveTo(0, 0); // Go back to first line
    for (int i = MAX_HIST_SIZE; i >= 0; --i) {
        // Next iteration if message is empty
        if (messageHistory[i].has_content) {
            // Lock output file to print message
            printf("[%s] %s%s:%s %s\n", \
                    messageHistory[i].date, BGRN, \
                    messageHistory[i].user_data.username, CRESET, \
                    messageHistory[i].message);
        }
    }

    // Return to user input line
    ClearLine(MAX_HIST_SIZE + 2);
    printf("%s(%s)%s Message:", BYEL, clientData.username, CRESET);
    fflush(stdout);

    pthread_mutex_unlock(&outputMutex);
    pthread_mutex_unlock(&historyMutex);
}

