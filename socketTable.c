#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include "socketTable.h"



// typedef struct client {
//     //client *prev; // pointer to prev client
//     client *next; // pointer to next client
//     int socketNum;
//     char handleName[MAX_HANDLE_LEN];
//     int handleLength;
//     int flag;
// } client;

// typedef struct socketTable {
//     client *curr; // pointer to current client
//     int numClients;
// } socketTable;

socketTable *setupSocketTable() {
    socketTable *st = malloc(sizeof(socketTable));
    if (st == NULL) {
        perror("malloc; not enough space for socket table.");
        exit(1);
    }
    st->numClients = 0;
    return st;
}

client *createClient() {
    client *c = malloc(sizeof(client));
    if (c == NULL) {
        perror("malloc; not enough space for client.");
        exit(1);
    }
    return c;
}

void setupClient(client *c, int socketNum, char *handleName) {
    c->handleLength = strlen(handleName);
    c->socketNum = socketNum;
    //st->currentClient = c;
    memcpy(c->handleName, handleName, strlen(handleName));
}

void addClientToSocketTable(socketTable *st, int socketNum, char *handleName) {
    client *c = createClient();
    setupClient(c, socketNum, handleName);

    if (st->numClients == 0) {
        st->curr = c;
    }
    else {
        c->prev = st->curr;
        st->curr = c;
    }
    st->numClients++;
}

void removeClientFromSocketTable(socketTable *st, int socketNum, char *handleName) {
    client *curr = st->curr;
    client *prev = NULL;
    for (int i = 0; i < st->numClients; i++) {
        if (strcmp(curr->handleName, handleName) == 0) {
            if (prev == NULL) {
                st->curr = curr->prev; // overwrite only handle in list
            }
            else {
                prev->prev = curr->prev;
            }
            free(curr);
            st->numClients--;
        }
        prev = curr;
        curr = curr->prev;
    }
}

//void updateTableLength() {}

int getNumClients(socketTable *st) {
    return st->numClients;
}

char *getHandleName(socketTable *st, int socketNum) {
    client *curr = st->curr;
    for (int i = 0; i < st->numClients; i++) {
        if (curr->socketNum == socketNum) {
            return curr->handleName;
        }
        curr = curr->prev;
    }
    return "handle not in socket table. \n";
}

char **listHandleNames(socketTable *st) {
    char **clientList = malloc(sizeof(char *) * st->numClients);
    client *curr = st->curr;
    //int len = st->numClients;
    printf("Handle List: \n");
    for (int i = 0; i < st->numClients; i++) {
        client *next = curr->prev; //st->curr->prev
        clientList[i] = curr->handleName;
        printf("%s , socketNum: %d \n", clientList[i], curr->socketNum);
        curr = next;
    }
    return clientList;
}

client *lookupHandleName(socketTable *st, char *handleName) {
    client *curr = st->curr;
    for (int i = 0; i < st->numClients; i++) {
        //printf("%s, %s \n", handleName, curr->handleName);
        client *prev = curr->prev;
        if (strcmp(curr->handleName, handleName) == 0) {
            return curr;
        }
        curr = prev;
    }
    return NULL;
}


int getHandleLength(socketTable *st, int socketNum) {
    client *curr = st->curr;
    for (int i = 0; i < st->numClients; i++) {
        if (curr->socketNum == socketNum) {
            return curr->handleLength;
        }
        curr = curr->prev;
    }
    return -1;
}


int getSocketNum(socketTable *st, char *handleName) {
    client *curr = st->curr;
    for (int i = 0; i < st->numClients; i++) {
        if (strcmp(curr->handleName, handleName) == 0) {
            return curr->socketNum;
        }
        curr = curr->prev;
    }
    return -1;
}


void lookupSocketNum() {}
