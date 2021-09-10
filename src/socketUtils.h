#pragma once

#include <openssl/ssl.h>

#define readLine(socketinfo)    readUntilString(socketinfo, CRLF, 2, 0)
#define readHeaders(socketinfo) readUntilString(socketinfo, HEADERS_END, 4, 0)

typedef struct socketStruct
{
    int descriptor;
    SSL *tls;
} socketStruct;

socketStruct *createSocket(char *host, int secure);
void closeSocket(socketStruct *socketInfo);

void sendMessage(socketStruct *socketInfo, char *message, int length);
char *readSize(socketStruct *socketInfo, int size);
char *readUntilString(socketStruct *socketInfo, char *target, int targetLength, int caseInsensitive);
