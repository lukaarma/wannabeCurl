#include "argParser.h"
#include "httpLib.h"
#include "logger.h"
#include "socketUtils.h"
#include "utils.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    socketStruct *socketInfo;
    httpRequest *req;
    httpResponse *res;

    // This are calloc so we don't have to manually set all pointers/lengths to NULL/0
    req = calloc(1, sizeof(httpRequest));
    res = calloc(1, sizeof(httpResponse));
    if (req == NULL || res == NULL)
    {
        logPanic("Could not allocate request/response structure in memory!");
    }

    // DEFAULTS SETTINGS
    req->method = GET;
    req->type   = NONE;

    parseArguments(argc, argv, req);
    generateHeaders(req);

    // setup output directory
    if (mkdir("./out", 0755) == -1)
    {
        if (errno != EEXIST)
        {
            logPanic("Could not create './out' directory!");
        }
    }
    res->filename       = strdup(req->host);
    res->filenameLength = req->hostLength;

    logVerbose("Request info: \n\t"
               "Secure: %s \n\t"
               "Method: %s \n\t"
               "Path: '%s' \n\t"
               "Host: '%s' \n\t"
               "Headers: %p \n\t"
               "ContentType: '%s' \n\t"
               "Text: '%s' \n\t"
               "Form: %p",
               req->secure ? "yes" : "no",
               methodNames[req->method],
               req->path,
               req->host,
               req->headers,
               contentTypeValue[req->type],
               req->text,
               req->form);

    socketInfo = createSocket(req->host, req->secure);

    logInfo("Building and sending HTTP payload...");

    buildRequest(req);
    logFile(DEBUG, res->filename, res->filenameLength,
            ".req.txt", 8, "%.*s", req->payloadSize, req->payload);

    sendMessage(socketInfo, req->payload, req->payloadSize);

    logInfo("Request sent! Size: %d bytes.", req->payloadSize);

    logInfo("Reciving data and saving to file...");

    reciveResponse(socketInfo, res);

    logVerbose("Response info: \n\t"
               "Content type: %s \n\t"
               "Content length: %d",
               contentTypeValue[res->type],
               res->contentLength);

    if (res->contentLength != 0)
    {
        // using panic to ensure that the response is saved even if logger is quiet
        logFile(PANIC, res->filename, res->filenameLength,
                contentTypeToExtension[res->type], contentTypeToLength[res->type],
                "%.*s", res->contentLength, res->content);
        logInfo("Response successfully recived! Size: %d bytes.", res->contentLength);
    }
    else
    {
        logInfo("Response successfully recived! It had no body!");
    }

    // 100s and 200s: success
    if (res->status >= 100 && res->status < 300)
    {
        logInfo("Status %d: %s", res->status, statusCodeDescription(res->status));
    }
    // 300s: redirects
    else if (res->status >= 300 && res->status < 400)
    {
        logWarn("Status %d: %s", res->status, statusCodeDescription(res->status));
    }
    // 400s and 500s: errors
    else if (res->status >= 400 && res->status < 600)
    {
        logError("Status %d: %s", res->status, statusCodeDescription(res->status));
    }

    freeHttp(req, res);
    closeSocket(socketInfo);

    return 0;
}
