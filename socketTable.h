#ifndef __SOCKETTABLE_H__
#define __SOCKETTABLE__


#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>

#define MAX_HANDLE_LEN 100


typedef struct client {
    struct client *prev; // pointer to prev client
    // pointer to next client
    int socketNum;
    char handleName[MAX_HANDLE_LEN];
    int handleLength;
    int flag;
} client;

typedef struct socketTable {
    client *curr; // pointer to prev client
    int numClients;
} socketTable;

socketTable *setupSocketTable();
client *createClient();

void setupClient(client *c, int socketNum, char *handleName);

void addClientToSocketTable(socketTable *st, int socketNum, char *handleName);

void removeClientFromSocketTable(socketTable *st, int socketNum, char *handleName);
//void updateTableLength();

char **listHandleNames(socketTable *st);

int getNumClients(socketTable *st);

char *getHandleName(socketTable *st, int socketNum);

int getHandleLength(socketTable *st, int socketNum);

int getSocketNum(socketTable *st, char *handleName);

client *lookupHandleName(socketTable *st, char *handleName); // to find who to send the message to

void lookupSocketNum();

char *getHandleName(socketTable *st, int socketNum);


#endif