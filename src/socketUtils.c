#include "socketUtils.h"
#include "httpLib.h"
#include "logger.h"
#include "utils.h"
#include <arpa/inet.h>
#include <ctype.h>
#include <netdb.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

socketStruct *createSocket(char *host, int secure)
{
    int error;
    struct addrinfo hints, *result, *DNSresult;
    socketStruct *socketInfo;
    SSL_CTX *tlsContext;

    logInfo("Resolving '%s'...", host);

    socketInfo = calloc(1, sizeof(socketInfo));

    // setup structs for DNS request
    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    DNSresult         = NULL;

    error = getaddrinfo(host, (secure ? HTTPS_PORT : HTTP_PORT), &hints, &DNSresult);
    if (error)
    {
        logPanic("Could not resolve '%s'!", host);
    }

    for (result = DNSresult; result != NULL; result = result->ai_next)
    {
        logVerbose("Resolved '%s' to '%s'",
                   host,
                   inet_ntoa(((struct sockaddr_in *)result->ai_addr)->sin_addr));

        socketInfo->descriptor = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
        if (socketInfo->descriptor == -1)
        {
            logWarn("Could not create socket, trying next address...");

            continue;
        }

        if (connect(socketInfo->descriptor, result->ai_addr, result->ai_addrlen) != -1)
        {
            logInfo("Socket created and connected to '%s'!", host);

            break;
        }

        logWarn("Could not connect to '%s', trying next address...",
                inet_ntoa(((struct sockaddr_in *)result->ai_addr)->sin_addr));

        close(socketInfo->descriptor);
    }

    freeaddrinfo(DNSresult);
    if (socketInfo->descriptor == -1)
    {
        logPanic("Could not create and connect socket to '%s'!", host);
    }

    // now TCP socket is created and connected

    if (secure)
    {
        SSL_load_error_strings();
        ERR_load_crypto_strings();

        // called TlsContext because we only accept TLS 1.2 or up
        tlsContext = SSL_CTX_new(TLS_client_method());
        if (tlsContext == NULL)
        {
            logPanic("Could not initialize secure context!");
        }

        if (!SSL_CTX_set_min_proto_version(tlsContext, TLS1_2_VERSION))
        {
            logPanic("Could not set TLS1.2 as minimum version!");
        }

        socketInfo->tls = SSL_new(tlsContext);
        if (tlsContext == NULL)
        {
            logPanic("Could not initialize secure socket!");
        }

        if (!SSL_set_fd(socketInfo->tls, socketInfo->descriptor))
        {
            logPanic("Could not bind socket descriptor to secure socket!");
        }

        error = SSL_connect(socketInfo->tls);
        if (error < 0)
        {
            logDebug("SSL_connect error: '%s'", ERR_reason_error_string(ERR_get_error()));
            logPanic("Could not connect secure socket!");
        }

        socketInfo->descriptor = -1;
    }

    return socketInfo;
}

void closeSocket(socketStruct *socketInfo)
{
    if (socketInfo->tls != NULL)
    {
        SSL_shutdown(socketInfo->tls);
        SSL_free(socketInfo->tls);
    }
    else
    {
        close(socketInfo->descriptor);
    }

    free(socketInfo);
}

void sendMessage(socketStruct *socketInfo, char *message, int length)
{
    logVerbose("Sending %s \n"
               "%.*s \n"
               "of size %d!",
               socketInfo->descriptor != -1 ? "message" : "secure message",
               length, message,
               length);

    if (socketInfo->descriptor != -1 &&
        send(socketInfo->descriptor, message, length, 0) == -1)
    {
        logDebug("Message send failed! Message: \n%.*s", length, message);
        logPanic("Could not send message!");
    }
    else if (socketInfo->tls != NULL &&
             SSL_write(socketInfo->tls, message, length) <= 0)
    {
        logDebug("Message send failed! Message: \n%.*s", length, message);
        logPanic("Could not send secure message!");
    }
}

char *readSize(socketStruct *socketInfo, int size)
{
    int readSize;
    char *buffer;

    readSize = 0;
    buffer   = malloc(size);
    if (buffer == NULL)
    {
        logPanic("Could not allocate buffer to recive message of %.2f KB", size / 1024);
    }

    if (socketInfo->descriptor != -1)
    {
        readSize = recv(socketInfo->descriptor, buffer, size, MSG_WAITALL);

        if (readSize == -1 || readSize != size)
        {
            logPanic("Error while reading message of %.2f KB from socket!", size / 1024);
        }
    }
    else
    {
        do
        {
            readSize += SSL_read(socketInfo->tls, buffer + readSize, size - readSize);

            logDebug("Read %d/%d from secure socket", readSize, size);

            if (readSize <= 0)
            {
                logPanic("Error while reading message of %.2f KB from socket!", size / 1024);
            }

        } while (readSize != size);
    }

    return buffer;
}

char *readUntilString(socketStruct *socketInfo, char *target, int targetLength, int caseInsensitive)
{
    int bufferSize, readSize, targetOffset, notFound;
    char *buffer;

    buffer     = NULL;
    bufferSize = readSize = 0;
    notFound              = 1;
    do
    {
        // if buffer full we increase it (-1 to leave space for null termination)
        if ((bufferSize - 1) == readSize || bufferSize == 0)
        {
            buffer = increaseBuffer(buffer, &bufferSize, bufferSize + 1);
        }

        if (socketInfo->descriptor != -1 &&
            recv(socketInfo->descriptor, buffer + readSize, sizeof(char), 0) == -1)
        {
            logPanic("Error while reading from socket!");
        }

        else if (socketInfo->tls != NULL &&
                 SSL_read(socketInfo->tls, buffer + readSize, sizeof(char)) <= 0)
        {
            logPanic("Error while reading from secure socket!");
        }

        ++readSize;
        *(buffer + readSize) = '\0';

        targetOffset = readSize - targetLength;

        if (targetOffset > 0)
        {
            if (caseInsensitive)
            {
                notFound = strncasecmp(buffer + targetOffset, target, targetLength);
            }
            else
            {
                notFound = strncmp(buffer + targetOffset, target, targetLength);
            }
        }
    } while (notFound);

    return buffer;
}
