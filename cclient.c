/******************************************************************************
* myClient.c
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
#include <errno.h>

#include "networks.h"
#include "safeUtil.h"
#include "util.h"
#include "pollLib.h"
#include "socketTable.h"

#define MAXBUF 1400
#define DEBUG_FLAG 1

void sendToServer(int socketNum, char *handleName);
int readFromStdin(uint8_t * buffer);
char *checkArgs(int argc, char * argv[]);
void clientControl(int clientSocket, char *handleName);
int processMsgFromServer(int mainServerSocket);
void clientRequest(int clientSocket, char *handleName);
void handleExit(int socketNum, char *handleName);


int main(int argc, char * argv[])
{
	int socketNum = 0;         //socket descriptor
	char *handleName = checkArgs(argc, argv);

	/* set up the TCP Client socket  */
	socketNum = tcpClientSetup(argv[2], argv[3], DEBUG_FLAG);

	clientControl(socketNum, handleName);
	return 0;
}

void clientControl(int clientSocket, char *handleName) {
	setupPollSet();
	addToPollSet(STDIN_FILENO);
	addToPollSet(clientSocket);
	int bytesReceived = -1;

	clientRequest(clientSocket, handleName);

	while (bytesReceived != 0) {
		printf("$: ");
		fflush(stdout);
		int newClientSocket = pollCall(-1);
		if (newClientSocket == STDIN_FILENO) { 
			//printf("Enter data: ");
			sendToServer(clientSocket, handleName); //process stdin

		}
		else if (newClientSocket == -1) {
			perror("Invalid client socket.\n");
			exit(1);
		}
		else {
			bytesReceived = processMsgFromServer(clientSocket);
			// check first flag, if 5 send recieve in one client
			// else for specific mylticast flag, send to all in list

		}
	}
	if (bytesReceived == 0) {
		printf("Server has terminated. \n");
		removeClientSocket(clientSocket);
        exit(1);
	}
	close(clientSocket);
	removeFromPollSet(clientSocket);
}

void clientRequest(int clientSocket, char *handleName) {
	uint8_t pdu_buff[MAX_HANDLE_LEN];
	int sent = 0;
	int handleLen = strlen(handleName);
	int flag = 1;
	
	memcpy(&pdu_buff[0], &flag, 1);
	memcpy(&pdu_buff[1], &handleLen, 1);
	memcpy(&pdu_buff[2], handleName, handleLen); //prints correct handle name!!!

	sent = sendPDU(clientSocket, pdu_buff, handleLen + 2); // for flag and handlelen bytes
	if (sent < 0)
	{
		perror("send call");
		exit(-1);
	}

	uint8_t flagbuff[1];
	int messageLen = 0;
	int serverFlag = 0;
	if ((messageLen = recvPDU(clientSocket, flagbuff, 1)) < 0)
	{
		perror("recv call");
		exit(-1);
	}
	memcpy(&serverFlag, &flagbuff[0], 1);
	//printf("server return flag: %d\n", serverFlag);

	if (serverFlag == 3) {
		printf("Handle name: %s already exists. \n", handleName);
		//remove from socket table
		removeClientSocket(clientSocket);
        exit(1);
	}
}

int processMsgFromServer(int clientSocket) {
	uint8_t dataBuffer[MAXBUF];
	int bytesReceived = recvPDU(clientSocket, dataBuffer, MAXBUF);
	//printf("%d \n", bytesReceived);
	int flag = 0;
	int senderHandleLen = 0;
	int destHandleLen = 0;
	int numDestHandles = 0;
	memcpy(&flag, &dataBuffer[0], 1);
	if (flag == 5) {
		memcpy(&senderHandleLen, &dataBuffer[1], 1);
		char senderHandle[senderHandleLen];
		char msgdata[200];
		memcpy(senderHandle, &dataBuffer[2], senderHandleLen);
		senderHandle[senderHandleLen] = '\0';
		memcpy(&numDestHandles, &dataBuffer[senderHandleLen + 2], 1);
		memcpy(&destHandleLen, &dataBuffer[senderHandleLen + 3], 1);
		char destHandle[destHandleLen];
		memcpy(destHandle, &dataBuffer[senderHandleLen + 4], destHandleLen);
		destHandle[destHandleLen] = '\0';
		// printf("dest habdle len: %d \n", destHandleLen);
		// printf("snder handle len: %d \n", senderHandleLen);

		// printf("dest handle : %s \n", destHandle);
		// printf("sender handle : %s \n", senderHandle);

		int msgPtr = senderHandleLen + 4 + destHandleLen;
		// printf("bytes received: %d \n", bytesReceived);
		// printf("msgPTr: %d \n", msgPtr);

		memcpy(&msgdata[0], &dataBuffer[msgPtr], bytesReceived - msgPtr);

		//printf("flag value: %d \n", flag);
		printf("%s: %s \n", senderHandle, &msgdata[0]);
	}
	if (flag == 6) {
		printf("IN HERERNO WRWRWRW\n");
	}
	if (flag == 11) {
		uint8_t flagbuff[1];
		int flag = 11;
		int sent = 0;
		memcpy(&flagbuff[0], &flag, 1);
		sent = sendPDU(clientSocket, &flagbuff[0], 1);
		if (sent < 0)
		{
			perror("send call");
			exit(-1);
		}
		
	}


	return bytesReceived;
}

void listHandles(int sendLen, uint8_t *sendBuf, int socketNum, char *handleName) {
	uint8_t flagbuff[1];
	int flag = 10;
	int sent = 0;
	memcpy(&flagbuff[0], &flag, 1);

	sent = sendPDU(socketNum, flagbuff, 1);
	if (sent < 0)
	{
		perror("send call");
		exit(-1);
	}

	uint8_t serverbuff[5];
	int messageLen = 0;
	int serverFlag = 0;
	int serverHandles = 0;
	//printf("%d \n", se)
	if ((messageLen = recvPDU(socketNum, serverbuff, 5)) < 0)
	{
		perror("recv call");
		exit(-1);
	}
	memcpy(&serverFlag, &serverbuff[0], 1);
	memcpy(&serverHandles, &serverbuff[1], 4);
	if (serverFlag == 11) {
		printf("Number of clients: %d \n", serverHandles);
		//send flag 12 to server for each handle
	}
	

}

void handleExit(int socketNum, char *handleName) {
	uint8_t flagbuff[1];
	int flag = 8;
	int sent = 0;
	memcpy(&flagbuff[0], &flag, 1);
	sent = sendPDU(socketNum, flagbuff, 1);
	if (sent < 0)
	{
		perror("send call");
		exit(-1);
	}
	uint8_t returnflagbuff[1];
	int messageLen = 0;
	if ((messageLen = recvPDU(socketNum, returnflagbuff, 1)) < 0)
	{
		perror("recv call");
		exit(-1);
	}
	int serverflag = 0;
	memcpy(&serverflag, &returnflagbuff[0], 1);
	if (serverflag == 9) {
		removeClientSocket(socketNum);
        exit(1);
	}

}

void formatMsgPacketToServer(int socketNum, char *handleName, uint8_t *sendBuf, int sendLen, char *destHandle) {
	uint8_t msgbuff[MAXBUF];
	int msgflag = 5;
	int senderHandleLen = strlen(handleName);
	char *senderHandle = handleName;
	int oneHandle = 1; // sending to 1 other client
	int destHandleLen = strlen(destHandle);
	// printf("%s \n", destHandle);
	// printf("%d \n", destHandleLen);
	// printf("%s \n", senderHandle);
	// printf("%d \n", senderHandleLen);


	int ptrToMsgData = 4 + destHandleLen;
	int msgSize = sendLen - ptrToMsgData;

	memcpy(&msgbuff[0], &msgflag, 1);
	memcpy(&msgbuff[1], &senderHandleLen, 1);
	memcpy(&msgbuff[2], senderHandle, senderHandleLen);
	memcpy(&msgbuff[senderHandleLen + 2], &oneHandle, 1);
	memcpy(&msgbuff[senderHandleLen + 3], &destHandleLen, 1);
	memcpy(&msgbuff[senderHandleLen + 4], destHandle, destHandleLen);
	memcpy(&msgbuff[ptrToMsgData + senderHandleLen], &sendBuf[ptrToMsgData], msgSize);
	msgbuff[ptrToMsgData + senderHandleLen + msgSize] = '\0';

	// printf("msgbuff[0]: %d\n", msgbuff[0]);
	// printf("msgbuff[1]: %d\n", msgbuff[1]);
	// printf("msgbuff[2]: %s\n", &msgbuff[2]);
	// printf("msgbuff[+ 2]: %d\n", msgbuff[senderHandleLen + 2]);
	// printf("msgbuff[+ 3]: %d\n", msgbuff[senderHandleLen + 3]);
	// printf("msgbuff[+ 4]: %s\n", &msgbuff[senderHandleLen + 4]);
	// printf("msgbuff[msg]: %s \n", &msgbuff[senderHandleLen + ptrToMsgData]);
	int sent = 0;
	sent = sendPDU(socketNum, msgbuff, ptrToMsgData + senderHandleLen + msgSize);
	//printf("socket num: %d \n", getSocketNum(st, destHandle));
	if (sent < 0)
	{
		perror("send call");
		exit(-1);
	}

	//printf("Amount of data sent is: %d\n", sent);

}

void formatMulticastPacketToServer(int socketNum, char *handleName, char *c_buffer, int sendLen) {
	uint8_t mc_buff[MAXBUF];
	int mc_flag = 6;
	int senderHandleLen = strlen(handleName);
	char *senderHandle = handleName;
	int destHandleLen = 0;

	memcpy(&mc_buff[0], &mc_flag, 1);
	memcpy(&mc_buff[1], &senderHandleLen, 1);
	memcpy(&mc_buff[2], senderHandle, senderHandleLen);

    char *token = strtok(c_buffer, " ");  // Get the first token
	int i = 0;
	int num = 0;
	int tokenLen = 0;
	int movePtr = senderHandleLen + 3;

	int messageLen;
	char *message;
    while (token != NULL) {
		if (i < 1) {
			// printf("command c: %s\n", token);  // Process the token (print it in this example)
			token = strtok(NULL, " ");  // Get the next token	
			i++;	
		}
		if (i > 0 && i < 2) {
			num = atoi(token);
			// printf("num handles: %d \n", num);
			memcpy(&mc_buff[senderHandleLen + 2], &num, 1);
			token = strtok(NULL, " ");  // Get the next token
			i++;
		}
		if (i >= 2 && num != 0) {
			tokenLen = strlen(token);
			// printf("token : %s \n", token);
			// printf("token len : %d \n", tokenLen);
			memcpy(&mc_buff[movePtr + tokenLen], &destHandleLen, 1);
			// printf("%c \n ", mc_buff[movePtr + tokenLen]);
			memcpy(&mc_buff[movePtr + tokenLen + 1], token, destHandleLen);
			token = strtok(NULL, " ");
			movePtr = movePtr + tokenLen;
			printf("%d \n", movePtr);
			i++;
			num--;
		}
		if (num == 0) {
			messageLen = strlen(token);
			message = token;
			token = strtok(NULL, " ");	 
		}
    }
	message[messageLen] = '\0';
	// printf("%d \n", messageLen);
	// printf("%s \n", message);
	memcpy(&mc_buff[movePtr], message, messageLen);

	// printf("mc buff: %c \n", mc_buff[movePtr]); // start of message
	// printf("mc buff: %c \n", mc_buff[movePtr + messageLen - 1]); // start of message

	int sent = 0;
	sent = sendPDU(socketNum, mc_buff, movePtr + messageLen - 1);
	if (sent < 0)
	{
		perror("send call");
		exit(-1);
	}
}


void sendToServer(int socketNum, char *handleName)
{
	uint8_t sendBuf[MAXBUF];   //data buffer
	int sendLen = 0;        //amount of data to send
	//int sent = 0;            //actual amount of data sent/* get the data and send it   */


	sendLen = readFromStdin(sendBuf);

	char c_buffer[sendLen];
	memcpy(c_buffer, sendBuf, sendLen);
	c_buffer[sendLen] = '\0';

	char *token = NULL;
	token = strtok((char *)sendBuf, " ");
	char *destHandle = NULL;
	if (token != NULL) {
		if (strcmp(token, "%e") == 0 || strcmp(token, "%E") == 0) {
			handleExit(socketNum, handleName);
		}
		if (strcmp(token, "%L") == 0 || strcmp(token, "%l") == 0) {
			listHandles(sendLen, sendBuf, socketNum, handleName);
		}
		if (strcmp(token, "%m") == 0 || strcmp(token, "%M") == 0) {
			destHandle = strtok(NULL, " ");
			formatMsgPacketToServer(socketNum, handleName, sendBuf, sendLen, destHandle);
		}
		if (strcmp(token, "%c") == 0 || strcmp(token, "%C") == 0) {
			formatMulticastPacketToServer(socketNum, handleName, c_buffer, sendLen);
		}

	}
}

int readFromStdin(uint8_t * buffer)
{
	char aChar = 0;
	int inputLen = 0;        
	
	// Important you don't input more characters than you have space 
	buffer[0] = '\0';
	// printf("Enter data: ");
	while (inputLen < (MAXBUF - 1) && aChar != '\n')
	{
		aChar = getchar();
		if (aChar != '\n')
		{
			buffer[inputLen] = aChar;
			inputLen++;
		}
	}
	
	// Null terminate the string
	buffer[inputLen] = '\0';
	inputLen++;
	
	return inputLen;
}

char *checkArgs(int argc, char * argv[])
{
	/* check command line arguments  */
	if (argc != 4)
	{
		printf("usage: %s host-name port-number \n", argv[0]);
		exit(1);
	}

	char *handleName = argv[1];
	return handleName;
}
