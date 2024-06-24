/*
 * @file ProgmemFileInformation.h
 * This file contains the header of the ProgmemFileInformation class,
 * which defines data structure to store static files in PROGMEM.
 * by github@flywhc
 * License: GPL v2.1 or later (Same LICENSE as the one in your ESP Arduino libraries)
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
