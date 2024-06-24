/**
 * @file ProgmemAsyncWebHandler.cpp
 * This file contains the implementation of the ProgmemAsyncWebHandler class,
 * which is designed to handle ESPAsyncWebServer web requests to load static files from PROGMEM
 * and ignore directories that may contain dynamic contents such as CGI, API
 * by github@flywhc
 * License: GPL v2.1 or later (Same as the LICENSE in ESPAsyncWebServer.h)
 */

#include "ProgmemAsyncWebHandler.h"

// Constructor for the ProgmemAsyncWebHandler class.
// Initializes the ignoredDirectories member with the provided array of directory names.
// @param progmemFiles: An array of ProgmemFileInformation structs containing information about the files to be served.
// @param ignoredDirectories: An array of C-string pointers containing directory names to be ignored.
// @param numOfIgnoredDirectories: The number of directory names in the ignoredDirectories array.
ProgmemAsyncWebHandler::ProgmemAsyncWebHandler(const ProgmemFileInformation progmemFiles[], const char *ignoredDirectories[],size_t numOfIgnoredDirectories):
  files(progmemFiles), numOfIgnoredDirs(numOfIgnoredDirectories) {
  // deep copy of ignoredDirectories
  ignoredDirs = new char*[numOfIgnoredDirs];
  for (size_t i = 0; i < numOfIgnoredDirs; i++) {
    ignoredDirs[i] = new char[strlen(ignoredDirectories[i]) + 1];
    strcpy(ignoredDirs[i], ignoredDirectories[i]);
  }
}

// Destructor for the ProgmemAsyncWebHandler class to clean up allocated memory.
ProgmemAsyncWebHandler::~ProgmemAsyncWebHandler() {
  for (size_t i = 0; i < numOfIgnoredDirs; i++) {
    delete[] ignoredDirs[i];
  }
  delete[] ignoredDirs;
}

// Handles the web request by sending a response with content of PROGMEM file, a status code of 200 and
//  a plain text message, if the request is a GET request and the requested file is in the PROGMEM file list.
// 
// @param request: The AsyncWebServerRequest object representing the incoming web request.
void ProgmemAsyncWebHandler::handleRequest(AsyncWebServerRequest *request) {
  String uri = request->url();
  if (request->method() != HTTP_GET || isIgnoredDirectory(uri)) {
    return;
  }
  
  // Handle the root request
  if(uri.equals("/")) {
      uri = "/index.htm";
  }

  int i = 0;
  while (files[i].file_path !=0 ) {
    if (strcmp_P(uri.c_str(), files[i].file_path) == 0) {
      AsyncWebServerResponse *response = request->beginResponse_P(200, files[i].content_type, (const uint8_t *)files[i].file_content, files[i].file_length);
      if(files[i].is_compressed) {
        response->addHeader("Content-Encoding", "gzip");
      }
      request->send(response);
      return;
    }
    i++;
  }

  // The default File not found response
  String message;
  message.reserve(200); 
  message = "Not found.\n\nURI: ";
  message += request->url();
  request->send(404, "text/plain", message);
}

// Checks whether the request method is GET and whether the request URI matches an ignored directory.
// If either condition is true, the function returns false. Otherwise, it returns true.
// 
// @param request: The AsyncWebServerRequest object representing the incoming web request.
// @return True if the request can be handled, false otherwise.
bool ProgmemAsyncWebHandler::canHandle(AsyncWebServerRequest *request) {
  if (request->method() != HTTP_GET || isIgnoredDirectory(request->url())) {
    return false;
  }
  return true;
}

// Checks whether the request URI matches an ignored directory.
// Iterates through the array of ignored directories and checks if the request URI starts with any of them.
// If a match is found, the function returns true. Otherwise, it returns false.
// 
// @param uri The URI of the request.
// @return True if the request URI matches an ignored directory, false otherwise.
bool ProgmemAsyncWebHandler::isIgnoredDirectory(const String& uri) {
  for (int i = 0; i < numOfIgnoredDirs; i++) {
      if (uri.startsWith(ignoredDirs[i])) {
          return true;
      }
  }
  return false;
}
