/**
 * @file ProgmemWebRequest.cpp
 * This file contains the implementation of the ProgmemWebRequest class,
 * which is designed to handle ESP8266WebServer web requests to load static files from PROGMEM
 * and ignore directories that may contain dynamic contents such as CGI, API
 * by github@flywhc
 * License: GPL v2.1 or later (Same as the LICENSE in ESP8266WebServer.h)
 */

#include "ProgmemWebRequest.h"

// Constructor for the ProgmemWebRequest class.
// Initializes the ignoredDirectories member with the provided array of directory names.
// 
// @param ignoredDirectories An array of C-string pointers containing directory names to be ignored.
ProgmemWebRequest::ProgmemWebRequest(const ProgmemFileInformation progmemFiles[], const char *ignoredDirectories[]):
    files(progmemFiles), ignoredDirs(ignoredDirectories) {

}

// Handles the web request by sending a response with a status code of 200 and a plain text message.
// If the request method is not GET or the request URI matches an ignored directory, the function returns false.
// Otherwise, it sends a response with a status code of 200 and a plain text message, and returns true.
// 
// @param server The ESP8266WebServer object representing the web server.
// @param requestMethod The HTTP method of the request.
// @param requestUri The URI of the request.
// @return True if the request was handled successfully, false otherwise.
bool ProgmemWebRequest::handle(ESP8266WebServer & server, HTTPMethod requestMethod, const String& requestUri) { 
  if (requestMethod != HTTP_GET || isIgnoredDirectory(requestUri)) {
    return false;
  }
  
  // Handle the root request
  String uri = requestUri;
  if(requestUri.equals("/")) {
      uri = "/index.htm";
  }
  
  int i = 0;
  while (files[i].file_path !=0 ) {
    if (strcmp_P(uri.c_str(), files[i].file_path) == 0) {
      if(files[i].is_compressed) {
        server.sendHeader("Content-Encoding", "gzip");
        server.send_P(200, files[i].content_type, files[i].file_content, files[i].file_length);
      }
      else {
        server.send_P(200, files[i].content_type, files[i].file_content, files[i].file_length);
      }
      return true;
    }
    i++;
  }

  // The default File not found response
  String message;
  message.reserve(200); 
  message = "Not found.\n\nURI: ";
  message += requestUri;
  server.send(404, "text/plain", message);
  return true;
}

// Checks whether the request method is GET and whether the request URI matches an ignored directory.
// If either condition is true, the function returns false. Otherwise, it returns true.
// 
// @param requestMethod The HTTP method of the request.
// @param requestUri The URI of the request.
// @return True if the request can be handled, false otherwise.
bool ProgmemWebRequest::canHandle(HTTPMethod requestMethod, const String& uri) {
  if (requestMethod != HTTP_GET || isIgnoredDirectory(uri)) {
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
bool ProgmemWebRequest::isIgnoredDirectory(const String& uri) {
  for (int i = 0; ignoredDirs[i] != NULL; i++) {
      if (uri.startsWith(ignoredDirs[i])) {
          return true;
      }
  }
  return false;
}
