#include "utils.h"
#include "httpLib.h"
#include "logger.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

const char *contentTypeToExtension[] = {
    [NONE]       = "txt",
    [TEXT_PLAIN] = "txt",
    [TEXT_HTML]  = "html",
    [FORM]       = "txt",
    [JSON]       = "json"};

const int contentTypeToLength[] = {
    [NONE]       = 4,
    [TEXT_PLAIN] = 4,
    [TEXT_HTML]  = 5,
    [FORM]       = 4,
    [JSON]       = 5};

char *increaseBuffer(char *buffer, int *bufferSize, int newSize)
{
    char *oldBuffer = buffer;

    while (newSize > *bufferSize)
    {
        *bufferSize += 1024;
        buffer = realloc(buffer, *bufferSize);
        if (buffer == NULL)
        {
            logPanic("Could not allocate new buffer of %d KB!", *bufferSize / 1024);
        }
    }
    if (oldBuffer != buffer)
    {
        logDebug("Reallocated memory from %p to %p, requested at least %d bytes (current size %d)",
                 oldBuffer, buffer, newSize, *bufferSize);
    }
    else
    {
        logDebug("Requested at least %d bytes (current size %d), no change",
                 newSize, *bufferSize);
    }

    return buffer;
}

// change all uppercase to lowercase in place
char *lowerString(char *str)
{
    int i;

    for (i = 0; str[i] != '\0'; i++)
    {
        str[i] = tolower(str[i]);
    }

    return str;
}

void parseUrl(char *uri, httpRequest *req)
{
    char *hostStart;
    int protoLength, hostLength;

    protoLength = hostLength = 0;

    // PROTOCOL
    while (*(uri + protoLength) != ':')
    {
        ++protoLength;
    }

    if (protoLength == 4 && !strncasecmp(uri, "http", protoLength))
    {
        req->secure = 0;
    }
    else if (protoLength == 5 && !strncasecmp(uri, "https", protoLength))
    {
        req->secure = 1;
    }
    else
    {
        logPanic("'%.s' invalid protocol! Only http/https allowed!", protoLength, uri);
    }

    logDebug("proto => %.*s", protoLength, uri);

    // HOST
    // proto + ://
    hostStart = uri + protoLength + 3;
    while (*(hostStart + hostLength) != '/' && *(hostStart + hostLength) != '?' && *(hostStart + hostLength) != '\0')
    {
        ++hostLength;
    }

    logDebug("host  => %.*s (%d)", hostLength, hostStart, hostLength);

    req->hostLength = hostLength;
    req->host       = strndup(hostStart, hostLength);

    // PATH
    req->pathLength = strlen(uri) - (protoLength + 3 + hostLength);
    if (req->pathLength == 0)
    {
        req->pathLength = 1;
        req->path       = strdup("/");
    }
    else
    {
        req->path = strdup(hostStart + hostLength);
    }
}

char *urlEncode(char *entry)
{
    int i, foundEqual, cursor;
    char *buffer;

    buffer = malloc(strlen(entry) * 3 + 1);
    if (buffer == NULL)
    {
        logPanic("Could not allocate initial url encoding buffer!");
    }

    for (i = cursor = foundEqual = 0; i < strlen(entry); ++i, ++cursor)
    {
        if ((entry[i] >= '1' && entry[i] <= '9') ||
            (entry[i] >= 'A' && entry[i] <= 'Z') ||
            (entry[i] >= 'a' && entry[i] <= 'z') ||
            (!foundEqual && entry[i] == '='))
        {
            if (entry[i] == '=')
            {
                foundEqual = 1;
            }
            sprintf(buffer + cursor, "%c", entry[i]);
        }
        else
        {
            sprintf(buffer + cursor, "%%%02X", entry[i]);
            // 2 extra char for the hex encoding
            cursor += 2;
        }
    }

    buffer = realloc(buffer, sizeof(char) * (cursor + 1));
    if (buffer == NULL)
    {
        logPanic("Could not reallocate url encoding buffer!");
    }

    return buffer;
}
