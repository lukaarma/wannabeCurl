#pragma once

#include "socketUtils.h"
#include <openssl/ssl.h>

#define HTTP_PORT  "80"
#define HTTPS_PORT "443"

#define CRLF        "\r\n"
#define HEADERS_END CRLF CRLF

// don't forget to update the const array in httpLib.c
typedef enum httpMethods
{
    GET,
    HEAD,
    POST,
    PUT,
    DELETE,
    METHODS_MAX
} httpMethods;

// don't forget to update the const array in httpLib.c
typedef enum contentType
{
    NONE,
    TEXT_PLAIN,
    TEXT_HTML,
    FORM,
    JSON,
    CONTENT_TYPE_MAX
} contentType;

typedef struct httpHeader
{
    char *line;
    int lineLength;
    struct httpHeader *next;
} httpHeader;

typedef struct httpForm
{
    char *entry;
    int entryLength;
    struct httpForm *next;
} httpForm;

typedef struct httpRequest
{
    // INFO
    /** 0 = classic socket
     *  1 = TLS socket */
    int secure;
    char *host;
    int hostLength;

    httpMethods method;
    char *path;
    int pathLength;
    struct httpHeader *headers;

    contentType type;
    /** size of the HTTP body */
    int contentLength;
    char *text;
    struct httpForm *form;

    /** complete HTTP payload */
    char *payload;
    /** size of the complete HTTP message */
    int payloadSize;

} httpRequest;

typedef struct httpResponse
{
    int status;
    contentType type;
    int contentLength;
    char *content;

    char *filename;
    int filenameLength;
} httpResponse;

extern const char *methodNames[];
extern const char *contentTypeValue[];

void generateHeaders(httpRequest *req);
void buildRequest(httpRequest *req);

void reciveResponse(socketStruct *socketInfo, httpResponse *res);

char *statusCodeDescription(int code);
void freeHttp(httpRequest *req, httpResponse *res);
