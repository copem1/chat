#ifndef __UTIL_H__
#define __UTIL_H__


#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "networks.h"
#include "safeUtil.h"
#include "pollLib.h"
#define MAXBUF 1400



int sendPDU(int clientSocket, uint8_t *dataBuffer, int lengthOfData);
int recvPDU(int socketNumber, uint8_t *dataBuffer, int bufferSize);
void removeClientSocket(int clientSocket);


int packHandle(uint8_t *dataBuffer, char *handleName);

int unpackHandle(uint8_t *dataBuffer, char *handleName);


#endif