#pragma once

#define MAGENTA "\x1b[35m"
#define CYAN    "\x1b[36m"
#define GREEN   "\x1b[32m"
#define YELLOW  "\x1b[33m"
#define RED     "\x1b[31m"
#define GRAY    "\x1b[90m"
#define RESET   "\x1b[0m"

#define DEFAULT_LOG_DIR        "./out"
#define DEFAULT_LOG_DIR_LENGTH 5

#define logDebug(fmt, ...)   logger(DEBUG, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)
#define logVerbose(fmt, ...) logger(VERBOSE, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)
#define logInfo(fmt, ...)    logger(INFO, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)
#define logWarn(fmt, ...)    logger(WARNING, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)
#define logError(fmt, ...)   logger(ERROR, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)
#define logPanic(fmt, ...)      logger(PANIC, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)

typedef enum
{
    PANIC,
    ERROR,
    WARNING,
    INFO,
    VERBOSE,
    DEBUG
} logLevel;

void logger(logLevel level, char *filename, int fileLine, const char *funcName, char *fmt, ...);
void logFile(logLevel level, char *name, int nameLength, const char *extension, int extLength, char *fmt, ...);

void increaseLogLevel();
void silenceLogger();
void initLogTime();
