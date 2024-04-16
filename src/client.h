#ifndef CLIENT_H
#define CLIENT_H

#include <pthread.h>
#include "common.h"

extern userinfo_t clientData;
extern pthread_mutex_t outputMutex;

void clientRead(void);
void clientWrite(void);
void clientStart(void);

#endif // CLIENT_H
