#ifndef HEADER_H_
#define HEADER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#define MAX_FILES 100
#define MAX_FILENAME 256

typedef struct {
    char c_file[MAX_FILENAME];
    char h_file[MAX_FILENAME];
} SourceFile;

#endif