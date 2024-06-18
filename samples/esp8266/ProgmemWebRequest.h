/*
 * @file ProgmemWebRequest.h
 * This file contains the header of the ProgmemWebRequest class,
 * which is designed to handle ESP8266WebServer web requests to load static files from PROGMEM
 * and ignore directories that may contain dynamic contents such as CGI, API
 * by github@flywhc
 * License: GPL v2.1 or later (Same as the LICENSE in ESP8266WebServer.h)
*/
#pragma once
#ifndef _PROGMEMWEBREQUEST_H
#define _PROGMEMWEBREQUEST_H

#include <ESP8266WebServer.h>
#include "ProgmemFileInformation.h"

/*
* @class ProgmemWebRequest
* This class is designed to handle ESP8266WebServer web requests to load static files from PROGMEM
*/
class ProgmemWebRequest: public RequestHandler {
public:
    // Constructor with ignored directories (for CGI, API etc.)
    ProgmemWebRequest(const ProgmemFileInformation progmemFiles[], const char *ignoredDirectories[]);
    // Handle web request. Implementation of RequestHandler
    bool handle(ESP8266WebServer & server, HTTPMethod requestMethod, const String& requestUri) override;
    // Can handle any request. Implementation of RequestHandler.
    bool canHandle(HTTPMethod method, const String& uri) override;
private:
    bool isIgnoredDirectory(const String& uri);
    const ProgmemFileInformation *files;
    const char **ignoredDirs;
};

#endif // _PROGMEMWEBREQUEST_H
