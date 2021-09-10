#include "logger.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

logLevel loggerLevel = INFO;
char *logTime        = NULL;

void logger(logLevel level, char *filename, int fileLine, const char *funcName, char *fmt, ...)
{
    va_list args;

    if (level <= loggerLevel)
    {
        switch (level)
        {
        case DEBUG:
            printf(MAGENTA "[DEBUG] " RESET);

            break;

        case VERBOSE:
            printf(CYAN "[VERBOSE] " RESET);

            break;

        case INFO:
            printf(GREEN "[INFO] " RESET);

            break;

        case WARNING:
            printf(YELLOW "[WARNING] " RESET);

            break;

        case ERROR:
            printf(RED "[ERROR] " RESET);

            break;

        case PANIC:
            printf(RED "[FATAL ERROR] " GRAY "%s:%d [%s] " RESET,
                   filename, fileLine, funcName);
            vprintf(fmt, args);
            printf("\n");

            exit(1);

        default:
            logPanic("Invalid logger level! \n");

            break;
        }

        if (loggerLevel >= DEBUG)
        {
            printf(GRAY "%s:%d [%s] " RESET,
                   filename, fileLine, funcName);
        }

        va_start(args, fmt);
        vprintf(fmt, args);
        printf("\n");
        va_end(args);
    }
}

void logFile(logLevel level, char *name, int nameLength, const char *extension, int extensionLength, char *fmt, ...)
{
    char *filename;
    int filenameLength;
    FILE *fp;
    va_list args;

    if (level <= loggerLevel)
    {
        if (logTime == NULL)
        {
            initLogTime();
            logDebug("TIME %s", logTime);
        }

        // LOG_DIR     + '/' + name       + ' - ' + logTime + '.' + extension
        // LOG_DIR_LEN + 1   + nameLength + 3     + 19      + 1   + extensionLength
        filenameLength = DEFAULT_LOG_DIR_LENGTH + 1 + nameLength + 3 + 19 + 1 + extensionLength;
        filename       = malloc(filenameLength + 1);

        if (filename == NULL)
        {
            logPanic("Could not allocate filename of length %d", filenameLength);
        }
        else
        {
            snprintf(filename, filenameLength + 1,
                     "%s/%s - %s.%s",
                     DEFAULT_LOG_DIR, name, logTime, extension);
        }
        fp = fopen(filename, "w+");

        if (fp == NULL)
        {
            logError("Could not open '%s' to write!", filename);
        }

        va_start(args, fmt);
        vfprintf(fp, fmt, args);
        va_end(args);

        free(filename);
        fclose(fp);
    }
}

void increaseLogLevel()
{
    if (loggerLevel < DEBUG)
    {
        ++loggerLevel;
    }
}

void silenceLogger()
{
    loggerLevel = PANIC;
}

void initLogTime()
{
    time_t epochTime;
    struct tm *currentTime;

    if (logTime == NULL)
    {
        // YYYY-MM-DD HH-MM-SS => 19 + 1
        logTime = malloc(19 + 1);
        if (logTime == NULL)
        {
            logPanic("Could not allocate time string!");
        }

        time(&epochTime);
        currentTime = localtime(&epochTime);
        strftime(logTime, 19 + 1, "%F %T", currentTime);
    }
}
