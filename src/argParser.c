#include "httpLib.h"
#include "logger.h"
#include "utils.h"
#include <argp.h>
#include <stdlib.h>
#include <string.h>

error_t optionParser(int key, char *arg, struct argp_state *state)
{
    int i;
    httpRequest *req = state->input;
    httpForm *newForm;
    httpHeader *newHeader;

    switch (key)
    {
    case 'v':
        increaseLogLevel();

        break;

    case 'q':
        silenceLogger();

        break;

    case 'm':
        logDebug("(--method) %s", arg);

        for (i = 0; i < METHODS_MAX; ++i)
        {
            if (!strcasecmp(arg, methodNames[i]))
            {
                req->method = i;

                return 0;
            }
        }

        logPanic("'%s' mthod not yet supported, sorry!", arg);

    case 'h':
        logDebug("(--header) %s", arg);

        newHeader = malloc(sizeof(httpHeader));

        newHeader->lineLength = strlen(arg);
        newHeader->line       = strdup(arg);

        newHeader->next = req->headers;
        req->headers    = newHeader;

        break;

    case 'f':
        if (req->type == NONE || req->type == FORM)
        {
            newForm = malloc(sizeof(httpForm));

            newForm->entry       = urlEncode(arg);
            newForm->entryLength = strlen(newForm->entry);
            newForm->next        = req->form;
            req->type            = FORM;
            req->form            = newForm;

            logDebug("(--form) %s => %s", arg, newForm->entryLength);
        }
        else
        {
            logWarn("You cannot declare multiple types of body content! Ignoring form body");
        }

        break;

    case 't':
        logDebug("--text %s", arg);

        if (req->type == NONE)
        {
            req->type = TEXT_PLAIN;
            req->text = strdup(arg);
        }
        else
        {
            logWarn("You cannot declare multiple types of body content! Ignoring text body");
        }

        break;

    case 'j':
        logDebug("--json %s", arg);

        if (req->type == NONE)
        {
            req->type = JSON;
            req->text = strdup(arg);
        }
        else
        {
            logWarn("You cannot declare multiple types of body content! Ignoring json body");
        }

        break;

    case ARGP_KEY_ARG:
        logDebug("(non option arg) %s", arg);

        if (state->arg_num >= 1)
        {
            logWarn("Only one url can be requested! Ignoring '%s'", arg);

            break;
        }

        parseUrl(arg, req);

        break;

    case ARGP_KEY_END:
        if (req->hostLength == 0 || req->pathLength == 0)
        {
            logError("Missing/Invalid url!");
            argp_usage(state);
        }

        break;

    default:

        return ARGP_ERR_UNKNOWN;
    }

    return 0;
}

void parseArguments(int argc, char **argv, httpRequest *req)
{
    struct argp_option options[] = {
        {"verbose", 'v', 0, 0, "Enable verbose console output"},
        {"quiet", 'q', 0, 0, "Suppress all console output except errors"},
        {"method", 'm', "METHOD", 0, "Choose the method of the HTTP/S request. \n"
                                     "Methods available GET (default), HEAD, OPTIONS, POST, PUT, DELETE"},
        {"header", 'h', "'name: value'", 0, "Add the name value pair as header to the request, can be used multiple times."},
        {"form", 'f', "'key=value'", 0, "Add an html form body, can be used multiple times to add multiple key value pairs"},
        {"text", 't', "'content'", 0, "Add a text body to the request"},
        {"json", 'j', "'json string'", 0, "Add a json body to the request.\n"
                                          "It also add the header with the correct encoding."},
        {0}};

    struct argp argp = {options, optionParser, "URL"};

    argp_parse(&argp, argc, argv, 0, 0, req);
}
