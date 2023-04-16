#ifndef FINDFILE
#define FINDFILE
#include "global.h"

char* find_file(const char* file_name) {
    char command[256];
    snprintf(command, sizeof(command), "find %s -path \"*/project-1---shell-future-gadget-lab/%s\"",getenv("HOME"),file_name);

    FILE* fp = popen(command, "r");
    if (fp == NULL) {
        perror("Failed to run find command");
        return NULL;
    }

    static char file_path[256];
    if (fgets(file_path, sizeof(file_path), fp) == NULL) {
        fprintf(stderr, "file not found\n");
        pclose(fp);
        return NULL;
    }

    size_t len = strlen(file_path);
    if (file_path[len - 1] == '\n') {
        file_path[len - 1] = 0; // Remove newline character
    }

    pclose(fp);
    return file_path;
}
#endif
