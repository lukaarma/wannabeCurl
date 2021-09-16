#include "httpLib.h"
#include "logger.h"
#include "socketUtils.h"
#include "utils.h"
#include <ctype.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

const char *methodNames[] = {
    [GET]     = "GET",
    [HEAD]    = "HEAD",
    [OPTIONS] = "OPTIONS",
    [POST]    = "POST",
    [PUT]     = "PUT",
    [DELETE]  = "DELETE"};

const char *contentTypeValue[] = {
    [NONE]       = "",
    [TEXT_PLAIN] = "text/plain",
    [TEXT_HTML]  = "text/html",
    [FORM]       = "application/x-www-form-urlencoded",
    [JSON]       = "application/json"};

void parseHeaders(httpResponse *res, char *headers);
void reciveBody(socketStruct *socketInfo, httpResponse *res);

/* generate Content-Type, Content-Length and Connection-Close headers */
void generateHeaders(httpRequest *req)
{
    httpForm *formEntry;
    httpHeader *header;

    // Content-Type
    if (req->type == TEXT_PLAIN || req->type == JSON)
    {
        req->contentLength = strlen(req->text);

        header             = malloc(sizeof(httpHeader));
        header->lineLength = snprintf(NULL, 0, "Content-Type: %s", contentTypeValue[req->type]);
        header->line       = malloc(header->lineLength + 1);
        snprintf(header->line, header->lineLength + 1, "Content-Type: %s", contentTypeValue[req->type]);

        header->next = req->headers;
        req->headers = header;
    }
    else if (req->type == FORM)
    {
        for (formEntry = req->form; formEntry != NULL; formEntry = formEntry->next)
        {
            // we need to account the & separator between multiple key:value
            req->contentLength += formEntry->entryLength + (formEntry->next != NULL ? 1 : 0);
        }

        header             = malloc(sizeof(httpHeader));
        header->lineLength = snprintf(NULL, 0, "Content-Type: %s", contentTypeValue[req->type]);
        header->line       = malloc(header->lineLength + 1);
        snprintf(header->line, header->lineLength + 1, "Content-Type: %s", contentTypeValue[req->type]);

        header->next = req->headers;
        req->headers = header;
    }

    // Content-Length
    header             = malloc(sizeof(httpHeader));
    header->lineLength = snprintf(NULL, 0, "Content-Length: %d", req->contentLength);
    header->line       = malloc(header->lineLength + 1);
    snprintf(header->line, header->lineLength + 1, "Content-Length: %d", req->contentLength);

    header->next = req->headers;
    req->headers = header;
}

void buildRequest(httpRequest *req)
{
    httpHeader *header = NULL;
    httpForm *formEntry;
    int bufferSize = 0, cursor = 0;

    // REQUEST LINE AND HOST HEADER
    req->payloadSize = snprintf(NULL, 0,
                                "%s %s HTTP/1.1" CRLF
                                "Host: %s" CRLF,
                                methodNames[req->method], req->path,
                                req->host);

    req->payload = increaseBuffer(req->payload, &bufferSize, req->payloadSize);
    snprintf(req->payload + cursor, bufferSize,
             "%s %s HTTP/1.1" CRLF
             "Host: %s" CRLF,
             methodNames[req->method], req->path,
             req->host);
    cursor = req->payloadSize;

    // HEADERS
    for (header = req->headers; header != NULL; header = header->next)
    {
        // + 2 to accomodate also the CRLF
        req->payloadSize += header->lineLength + 2;

        // + 2 to accomodate also the final blank line and CRLF
        req->payload = increaseBuffer(req->payload, &bufferSize, req->payloadSize + 2);

        snprintf(req->payload + cursor, bufferSize, "%s" CRLF, header->line);
        cursor = req->payloadSize;
    }
    req->payloadSize += 2;
    strcat(req->payload, CRLF);
    cursor = req->payloadSize;

    logDebug("HTTP payload headers done");

    // BODY
    if (req->method == POST || req->method == PUT || req->method == DELETE)
    {
        req->payloadSize += req->contentLength;
        req->payload = increaseBuffer(req->payload, &bufferSize, req->payloadSize);

        if (req->type == TEXT_PLAIN || req->type == JSON)
        {
            snprintf(req->payload + cursor, bufferSize, "%s", req->text);
        }
        else if (req->type == FORM)
        {

            for (formEntry = req->form; formEntry != NULL; formEntry = formEntry->next)
            {
                snprintf(req->payload + cursor, bufferSize,
                         "%s%c",
                         formEntry->entry,
                         formEntry->next != NULL ? '&' : '\0');

                cursor += formEntry->entryLength + 1;
            }
        }

        logDebug("HTTP payload body done");
    }
}

void reciveResponse(socketStruct *socketInfo, httpResponse *res)
{
    char *responseHeaders;

    logInfo("Reciving and parsing response headers..");
    responseHeaders = readHeaders(socketInfo);

    logFile(DEBUG, res->filename, res->filenameLength, "res.txt", 7, "%s", responseHeaders);
    parseHeaders(res, responseHeaders);
    free(responseHeaders);

    logInfo("Done! Now reciving response body..");

    reciveBody(socketInfo, res);
}

char *statusCodeDescription(int code)
{
    // Status codes descriptions from rfc2616 section 6.1.1
    switch (code)
    {
    case 100:
        return "Continue";
    case 101:
        return "Switching Protocols";

    case 200:
        return "OK";
    case 201:
        return "Created";
    case 202:
        return "Accepted";
    case 203:
        return "Non-Authoritative Information";
    case 204:
        return "No Content";
    case 205:
        return "Reset Content";
    case 206:
        return "Partial Content";

    case 300:
        return "Multiple Choices";
    case 301:
        return "Moved Permanently";
    case 302:
        return "Found";
    case 303:
        return "See Other";
    case 304:
        return "Not Modified";
    case 305:
        return "Use Proxy";
    case 307:
        return "Temporary Redirect";

    case 400:
        return "Bad Request";
    case 401:
        return "Unauthorized";
    case 402:
        return "Payment Required";
    case 403:
        return "Forbidden";
    case 404:
        return "Not Found";
    case 405:
        return "Method Not Allowed";
    case 406:
        return "Not Acceptable";
    case 407:
        return "Proxy Authentication Required";
    case 408:
        return "Request Time-out";
    case 409:
        return "Conflict";
    case 410:
        return "Gone";
    case 411:
        return "Length Required";
    case 412:
        return "Precondition Failed";
    case 413:
        return "Request Entity Too Large";
    case 414:
        return "Request-URI Too Large";
    case 415:
        return "Unsupported Media Type";
    case 416:
        return "Requested range not satisfiable";
    case 417:
        return "Expectation Failed";

    case 500:
        return "Internal Server Error";
    case 501:
        return "Not Implemented";
    case 502:
        return "Bad Gateway";
    case 503:
        return "Service Unavailable";
    case 504:
        return "Gateway Time-out";
    case 505:
        return "HTTP Version not supported";

    default:
        return "Unsupported status code!";
    }
}

void freeHttp(httpRequest *req, httpResponse *res)
{
    httpHeader *header;
    httpForm *formEntry;

    logDebug("Freeing allocated memory");

    // REQUEST
    free(req->host);
    free(req->path);
    free(req->text);
    free(req->payload);

    while (req->headers != NULL)
    {
        header       = req->headers;
        req->headers = req->headers->next;

        free(header->line);
        free(header);
    }

    while (req->form != NULL)
    {
        formEntry = req->form;
        req->form = req->form->next;

        free(formEntry->entry);
        free(formEntry);
    }

    free(req);

    logDebug("Request struct freed");

    // RESPONSE
    free(res->content);
    free(res->filename);
    free(res);

    logDebug("Response struct freed");
}

// ==================== LOCAL FUNCTIONS ====================

void parseHeaders(httpResponse *res, char *headers)
{
    int i;
    char *headerLine;

    // STATUS-LINE
    headerLine = strtok(headers, "\r\n");

    while (*headerLine != ' ')
    {
        ++headerLine;
    }
    ++headerLine;
    // now headerLine points to the first digit of the status code
    headerLine[3] = '\0';
    res->status   = strtol(headerLine, NULL, 10);

    // HEADERS
    headerLine = strtok(NULL, "\r\n");
    while (headerLine != NULL)
    {
        if (strncasecmp(headerLine, "Content-Type", 12) == 0)
        {
            // headerLine = stripString(headerLine + 13);
            for (i = 0; i < CONTENT_TYPE_MAX && res->type == NONE; ++i)
            {
                if (strstr(lowerString(headerLine), contentTypeValue[i]))
                {
                    res->type = i;
                }
            }
        }
        else if (strncasecmp(headerLine, "Content-Length", 14) == 0)
        {
            res->contentLength = atoi(headerLine + 15);
        }
        else if (strncasecmp(headerLine, "Transfer-Encoding", 17) == 0 &&
                 strstr(lowerString(headerLine), "chunked"))
        {
            res->contentLength = -1;
        }

        headerLine = strtok(NULL, "\r\n");
    }
}

void reciveBody(socketStruct *socketInfo, httpResponse *res)
{
    int chunkSize;
    char *buffer;

    if (res->contentLength > 0)
    {
        logVerbose("Reciving entire body: size %d", res->contentLength);
        res->content = readSize(socketInfo, res->contentLength);
    }
    // contentLength -1 means chunked
    else if (res->contentLength == -1)
    {
        /* CHUNK STRUCTURE
        Chunked-Body   = *chunk
                         last-chunk
                         trailer
                         CRLF

        chunk          = chunk-size [ chunk-extension ] CRLF
                         chunk-data CRLF
        chunk-size     = 1*HEX
        last-chunk     = 1*("0") [ chunk-extension ] CRLF

        chunk-extension= *( ";" chunk-ext-name [ "=" chunk-ext-val ] )
        chunk-ext-name = token
        chunk-ext-val  = token | quoted-string
        chunk-data     = chunk-size(OCTET)
        trailer        = *(entity-header CRLF)
        */

        logVerbose("Reciving chiunked body");
        res->contentLength = 0;

        while (1)
        {
            // read chunk size
            buffer    = readLine(socketInfo);
            chunkSize = strtol(buffer, NULL, 16);
            free(buffer);

            if (chunkSize == 0)
            {
                break;
            }

            logDebug("Chunk size %d", chunkSize);

            buffer = readSize(socketInfo, chunkSize);

            res->content = realloc(res->content, res->contentLength + chunkSize);
            memcpy(res->content + res->contentLength, buffer, chunkSize);
            res->contentLength += chunkSize;

            free(buffer);
        }
    }
}
