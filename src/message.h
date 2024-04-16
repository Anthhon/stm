#ifndef MESSAGE_H_
#define MESSAGE_H_

#include "common.h"

extern message_t messageData;
extern message_t messageHistory[MAX_HIST_SIZE];

void messagePrint(message_t *messageReceived);
void messageBuild(char *dest);

#endif // MESSAGE_H_
