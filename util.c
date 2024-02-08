#include "util.h"
#include "socketTable.h"


int sendPDU(int clientSocket, uint8_t *dataBuffer, int lengthOfData) {
    uint8_t PDU_buffer[lengthOfData + 2];
    uint16_t lengthOfData_no = htons(lengthOfData); // network order
    memcpy(PDU_buffer, &lengthOfData_no, 2); // copy data for full pdu len from its addr into new buffer
    memcpy(PDU_buffer + 2, dataBuffer, lengthOfData); // copy all data into new buff, leaving first 2 bytes for length in network order
    safeSend(clientSocket, PDU_buffer, lengthOfData + 2, 0);

    return lengthOfData;
}


int recvPDU(int socketNumber, uint8_t *dataBuffer, int bufferSize) {
    uint16_t message_len;
    safeRecv(socketNumber, (uint8_t *)(&message_len), 2, MSG_WAITALL);
    message_len = ntohs(message_len);

    int n = safeRecv(socketNumber, dataBuffer, message_len, MSG_WAITALL);
    if (n != message_len) {
        perror("client has been terminated.\n");
        return 0;
    }
    return n;
}

// void setFlag(uint8_t *dataBuffer) {
//     int flag = dataBuffer[0];
// }

void removeClientSocket(int clientSocket) {
	close(clientSocket);
	removeFromPollSet(clientSocket);
}

// %m and %c have same packet format, diff than %b

int packHandle(uint8_t *dataBuffer, char *handleName) {
    return 0;
}

int unpackHandle(uint8_t *dataBuffer, char *handleName) {
    // return handle name length
    return 0;
}