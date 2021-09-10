#pragma once

#include "httpLib.h"

#define DEFAULT_OUT_DIRECTORY "./out/"

extern const char *contentTypeToExtension[];
extern const int contentTypeToLength[];

/**
 * Increase buffer, using 1Kb increments and reallocating only if needed,
 * to be big enough to contain newSize.
 * @param buffer can be a pointer to an existing array o NULL to allocate a new one
 * @param bufferSize is an integer passed by referene that keeps track of the size
 * @param newSize is an integer indicating the minimum new size
 *
 * @return New buffer pointer
 */
char *increaseBuffer(char *buffer, int *bufferSize, int newSize);

char *lowerString(char *str);

/**
 * Extracts the domain and the path from uri and set the respective fields
 * in req
 *
 * @param uri Pointer to the char array containing the uri to parse
 * @param req Pointer to the request struct that will be populated parsing the uri
 */
void parseUrl(char *uri, httpRequest *req);

char *urlEncode(char *entry);
