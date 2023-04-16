#ifndef FINDFILE
#define FINDFILE
#include "global.h"

char* find_file(const char* file_name) {
    char command[256];
    // creates a command to find the path to the file name relative to our project directory
    // it searches for it recursively in the home directory
    snprintf(command, sizeof(command), "find %s -path \"*/project-1---shell-future-gadget-lab/%s\"",getenv("HOME"),file_name);

    // runs the command 
    FILE* fp = popen(command, "r");
    if (fp == NULL) {
        perror("Failed to run find command");
        return NULL;
    }

    static char file_path[256];
    // tries to get the output of the command which will be the absolute path to the file
    if (fgets(file_path, sizeof(file_path), fp) == NULL) {
        fprintf(stderr, "file not found\n");
        pclose(fp);
        return NULL;
    }

    size_t len = strlen(file_path);
    // Remove newline character
    if (file_path[len - 1] == '\n') {
        file_path[len - 1] = 0;
    }

    pclose(fp);
    // returns the absolute path to the file
    return file_path;
}
#endif
