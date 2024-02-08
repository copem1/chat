/******************************************************************************
* myServer.c
* 
* Writen by Prof. Smith, updated Jan 2023
* Use at your own risk.  
*
*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdint.h>

#include "networks.h"
#include "safeUtil.h"
#include "util.h"
#include "pollLib.h"
#include "socketTable.h"

#define MAXBUF 1400
#define MAX_MSG_LEN 200
#define DEBUG_FLAG 1

void recvFromClient(socketTable *st, int clientSocket);
int checkArgs(int argc, char *argv[]);
void serverControl(int mainServerSocket);
void addNewSocket(socketTable *st, int newClientSocket);
void processClient(socketTable *st, int newClientSocket);
void removeClientSocket(int clientSocket);
void sendToClient(int socketNum);
char *clientAccept(socketTable *st, int clientSocket);
void checkValidHandle(socketTable *st, int clientSocket, char *handleName, int flag);
void handleMessage(socketTable *st, int clientSocket, uint8_t *dataBuffer, int messageLen);
void formatMsgPacket(socketTable *st, int clientSocket, char *destHandle, int messageLen, uint8_t *dataBuffer);
void sendData(socketTable *st, char *destHandle, int msgDataFlag);


// SOCKET TABLE DOES NOT NEED TO BE STRUNK IN SIZE ( TABLE LEN )

int main(int argc, char *argv[])
{
	int mainServerSocket = 0;   //socket descriptor for the server socket
	//int clientSocket = 0;   //socket descriptor for the client socket
	int portNumber = 0;
	portNumber = checkArgs(argc, argv);
	
	//create the server socket
	mainServerSocket = tcpServerSetup(portNumber);
	serverControl(mainServerSocket);
	//clientSocket = tcpAccept(mainServerSocket, DEBUG_FLAG);
	return 0;
}

void serverControl(int mainServerSocket) {

	setupPollSet();
	addToPollSet(mainServerSocket);
	socketTable *st = setupSocketTable();

	while (1) {
		int newClientSocket = pollCall(-1);

		if (newClientSocket == mainServerSocket) {
			addNewSocket(st, newClientSocket); // newClientSocket is still main server socket = 0
			//listHandleNames(st);
		}
		else {
			processClient(st, newClientSocket);
		}

	}
	close(mainServerSocket);
	removeFromPollSet(mainServerSocket);
}

char *clientAccept(socketTable *st, int clientSocket) {

	uint8_t pdu_buff[MAX_HANDLE_LEN];
	int flag = 0;
	int handleLen = 0;
	char *handleName = (char*)malloc(sizeof(char) * 100); // NEED TO CHANGE, CANNOT MALLOC IN SERVER/CCLIENT
	//char handleName[100];
	int messageLen = 0;

	//now get the data from the client_socket
	if ((messageLen = recvPDU(clientSocket, pdu_buff, MAX_HANDLE_LEN)) < 0)
	{
		perror("recv call");
		exit(-1);
	}

	if (messageLen > 0)
	{
		memcpy(&flag, &pdu_buff[0], 1);
		memcpy(&handleLen, &pdu_buff[1], 1);
		memcpy(handleName, &pdu_buff[2], handleLen);
		handleName[handleLen] = '\0';
		// printf("flag incoming from client: %d \n", flag);
		// printf("incoming client handle length: %d \n", handleLen);
		//printf("%s has connected to the server. \n", handleName); //prints correct handle name!!!

		//check if handle name already exists
		checkValidHandle(st, clientSocket, handleName, flag);
	}
	else
	{
		printf("Connection closed by other side\n");
		//close(clientSocket);
		removeClientSocket(clientSocket);
	}
	return handleName;
}

void checkValidHandle(socketTable *st, int clientSocket, char *handleName, int flag) {
	//int validFlag = 0;
	if (flag == 1) {
		if (lookupHandleName(st, handleName) != NULL) { // handle already in table
			uint8_t flagbuff[1];
			int serverFlag = 3;
			int sent = 0;
			memcpy(&flagbuff[0], &serverFlag, 1);
			sent = sendPDU(clientSocket, &flagbuff[0], 1);
			if (sent < 0)
			{
				perror("send call");
				exit(-1);
			}
			removeClientFromSocketTable(st, clientSocket, handleName);
			removeClientSocket(clientSocket);		
		} 
		else { // valid handle
			//validFlag = 1;
			uint8_t flagbuff[1];
			int serverFlag = 2;
			int sent = 0;
			memcpy(&flagbuff[0], &serverFlag, 1);
			sent = sendPDU(clientSocket, &flagbuff[0], 1);
			if (sent < 0)
			{
				perror("send call");
				exit(-1);
			}
			//addClientToSocketTable(st, clientSocket, handleName);
		}

	}
	//return validFlag;
}

void addNewSocket(socketTable *st, int mainServerSocket) {
	int newClientSocket = 0;
	newClientSocket = tcpAccept(mainServerSocket, DEBUG_FLAG);
	addToPollSet(newClientSocket);
	char *handleName = clientAccept(st, newClientSocket);
	addClientToSocketTable(st, newClientSocket, handleName);
}

void processClient(socketTable *st, int clientSocket) {
	recvFromClient(st, clientSocket);
}


void recvFromClient(socketTable *st, int clientSocket)
{
	uint8_t dataBuffer[MAXBUF];
	int messageLen = 0;
	
	//now get the data from the client_socket
	if ((messageLen = recvPDU(clientSocket, dataBuffer, MAXBUF)) < 0)
	{
		perror("recv call");
		exit(-1);
	}

	if (messageLen > 0)
	{
		//printf("Message received on socket %d, length: %d Data: %s\n", clientSocket, messageLen, dataBuffer);
		
		handleMessage(st, clientSocket, dataBuffer, messageLen);
	}
	else
	{
		printf("Connection closed by other side\n");
		removeClientSocket(clientSocket);
	}
}


void handleListFlag(socketTable *st, int clientSocket, uint8_t *dataBuffer) {
	uint8_t listflag[1];
	//int messageLen = 0;
	memcpy(&listflag, &dataBuffer[0], 1);
	//printf("%d \n", listflag[0]);

	if (listflag[0] == 10) {
		uint8_t serverHandles[5];
		int flag = 11;
		int sent = 0;
		//printf("%d is num of clients on server \n", st->numClients);
		int clients = st->numClients;
		memcpy(&serverHandles[0], &flag, 1);
		memcpy(&serverHandles[1], &clients, 4);
		// for (int i = 0; i < st->numClients; i++) {
		// 	memcpy()
		// }
		sent = sendPDU(clientSocket, serverHandles, 5);
		if (sent < 0)
		{
			perror("send call");
			exit(-1);
		}
	}

}

void formatExitPacket(int flag, int clientSocket) {
	if (flag == 8) {
		uint8_t flagbuff[1];
		//int exitflag = 0;
		//int messageLen = 0;
		int serverflag = 9;
		int sent = 0;
		memcpy(&flagbuff[0], &serverflag, 1);
		sent = sendPDU(clientSocket, flagbuff, 1);
		if (sent < 0)
		{
			perror("send call");
			exit(-1);
		}

	}
}

int processMsgFromClient(socketTable *st, int clientSocket, uint8_t *dataBuffer, int messageLen) {
	//uint8_t dataBuffer[MAXBUF];
	//int bytesReceived = recvPDU(clientSocket, dataBuffer, MAXBUF);
	//printf("%d \n", bytesReceived);
	int flag = 0;
	int senderHandleLen = 0;
	int destHandleLen = 0;
	int numDestHandles = 0;

	memcpy(&flag, &dataBuffer[0], 1);
	//printf("%d \n", flag);
	if (flag == 5) {
		memcpy(&senderHandleLen, &dataBuffer[1], 1);
		char senderHandle[senderHandleLen];
		memcpy(senderHandle, &dataBuffer[2], senderHandleLen);
		senderHandle[senderHandleLen] = '\0';
		memcpy(&numDestHandles, &dataBuffer[senderHandleLen + 2], 1);

		memcpy(&destHandleLen, &dataBuffer[senderHandleLen + 3], 1);
		char destHandle[destHandleLen];
		//char *destHandle = getHandleName(st, clientSocket);
		memcpy(destHandle, &dataBuffer[senderHandleLen + 4], destHandleLen);
		destHandle[destHandleLen] = '\0';
		// printf("%s \n", destHandle);
		// printf("%s \n", senderHandle);
		// printf("dest habdle len: %d \n", destHandleLen);
		// printf("snder handle len: %d \n", senderHandleLen);

		int sent = 0;
		// printf("dest socket num: %d \n", getSocketNum(st, destHandle));
		// printf("sender socket num: %d \n", getSocketNum(st, senderHandle));
		sent = sendPDU(getSocketNum(st, destHandle), dataBuffer, messageLen);
		if (sent < 0)
		{
			perror("send call");
			exit(-1);
		}
	}
	if (flag == 6) {
		memcpy(&senderHandleLen, &dataBuffer[1], 1);
		printf("sender handle len : %d\n", senderHandleLen);
		char senderHandle[senderHandleLen];
		memcpy(senderHandle, &dataBuffer[2], senderHandleLen);
		senderHandle[senderHandleLen] = '\0';
		printf("sender handle : %s\n", senderHandle);

		memcpy(&numDestHandles, &dataBuffer[senderHandleLen + 2], 1);
		printf("num dest handles : %d\n", numDestHandles);

		int movePtr = senderHandleLen + 3;
		printf("first handle len : %d\n", dataBuffer[movePtr]);
		int i = 0;
		for (i = 0; i < numDestHandles; i++) {
			memcpy(&destHandleLen, &dataBuffer[movePtr], 1);
			printf("dest handle len : %d \n", destHandleLen);
			char destHandle[destHandleLen];
			memcpy(destHandle, &dataBuffer[movePtr + 1], destHandleLen);
			destHandle[destHandleLen] = '\0';
			printf("dest handle : %s \n", destHandle);

			// int sent = 0;
			// printf("dest socket num: %d \n", getSocketNum(st, destHandle));
			// printf("sender socket num: %d \n", getSocketNum(st, senderHandle));
			// sent = sendPDU(getSocketNum(st, destHandle), dataBuffer, messageLen);
			// if (sent < 0)
			// {
			// 	perror("send call");
			// 	exit(-1);
			// }


			movePtr = movePtr + destHandleLen + 1;
		}
		
	}

	return messageLen;
}

//void processMulticast(socketTable *st, int clientSocket, uint8_t *dataBuffer, int messageLen) {}

void handleMessage(socketTable *st, int clientSocket, uint8_t *dataBuffer, int messageLen) {
	//char *destHandle = NULL;
	// token = strtok((char *)dataBuffer, " ");
	// printf("%s. \n", token);
	// if (token != NULL) {

	int flag = dataBuffer[0];
	if (flag == 8) {
		//int destHandleLen = dataBuffer[1];
		//char destHandle[destHandleLen];
		formatExitPacket(flag, clientSocket);
	}
	else if (flag == 5) {
		processMsgFromClient(st, clientSocket, dataBuffer, messageLen);
	}
	else if (flag == 6) {
		processMsgFromClient(st, clientSocket, dataBuffer, messageLen);
	}
	else if (flag == 10) { //list clients
		handleListFlag(st, clientSocket, dataBuffer);
	}

}


int checkArgs(int argc, char *argv[])
{
	// Checks args and returns port number
	int portNumber = 0;

	if (argc > 2)
	{
		fprintf(stderr, "Usage %s [optional port number]\n", argv[0]);
		exit(-1);
	}
	
	if (argc == 2)
	{
		portNumber = atoi(argv[1]);
	}
	
	return portNumber;
}

