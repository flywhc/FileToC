/*
 * @file ProgmemWebRequest.h
 * This file contains the header of the ProgmemWebRequest class,
 * which is designed to handle ESPAsyncWebServer web requests to load static files from PROGMEM
 * and ignore directories that may contain dynamic contents such as CGI, API
 * by github@flywhc
 * License: GPL v2.1 or later (Same as the LICENSE in ESPAsyncWebServer.h)
*/
#pragma once
#ifndef _PROGMEMASYNCWEBHANDLER_H
#define _PROGMEMASYNCWEBHANDLER_H

#include <ESPAsyncWebServer.h>
#include "ProgmemFileInformation.h"

/*
* @class ProgmemAsyncWebHandler
* This class is designed to handle ESPAsyncWebServer web requests to load static files from PROGMEM
*/
class ProgmemAsyncWebHandler: public AsyncWebHandler {
public:
    // Constructor with ignored directories (for CGI, API etc.)
    ProgmemAsyncWebHandler(const ProgmemFileInformation progmemFiles[], const char *ignoredDirectories[], size_t numOfIgnoredDirectories);
    // Handle web request. Implementation of AsyncWebHandler
    virtual void handleRequest(AsyncWebServerRequest *request) override final;
    // Can handle any request. Implementation of AsyncWebHandler.
    virtual bool canHandle(AsyncWebServerRequest *request) const override final;
    virtual bool isRequestHandlerTrivial() const override final {return false;}
    ~ProgmemAsyncWebHandler();
private:
    bool isIgnoredDirectory(const String& uri) const;
    const ProgmemFileInformation *files;
    char **ignoredDirs;
    size_t numOfIgnoredDirs;
};

#endif // _PROGMEMASYNCWEBHANDLER_H
