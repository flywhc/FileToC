/*
 * @file ProgmemWebRequest.h
 * This file contains the header of the ProgmemWebRequest class,
 * which is designed to handle ESP8266WebServer web requests to load static files from PROGMEM
 * and ignore directories that may contain dynamic contents such as CGI, API
 * by github@flywhc
 * License: GPL v2.1 or later (Same as the LICENSE in ESP8266WebServer.h)
*/

#pragma once
#ifndef _PROGMEMFILEINFORMATION_H
#define _PROGMEMFILEINFORMATION_H

#include <pgmspace.h>

typedef struct ProgmemFileInformationStruct
{
    const char * file_path;
    const char * file_content;
    const int file_length;
    const char * content_type;
    const int is_compressed;
} ProgmemFileInformation;


#endif // _PROGMEMFILEINFORMATION_H
