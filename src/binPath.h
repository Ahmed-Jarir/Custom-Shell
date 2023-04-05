#include "global.h"
char* getBinPath(char* commandName){
    char cwd[1024];
    char* PATH;
    char* PATHTok;
    char* path;

    // checking for full path
    if ( !access(commandName, X_OK) ) {
        return commandName;
    }

    // check for executable in directory
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        path = (char*)calloc(strlen(cwd) + strlen(commandName) + 2, 1);

        // concats the path and the name into buffer
        sprintf(path, "%s/%s", cwd, commandName);
        // checks if the path is accessable 
        if ( !access(path, X_OK) ) {
             return path;
        }
    }

    // gets a copy of the PATH string so that we dont edit the 
    PATH = strdup(getenv("PATH"));
    PATHTok = strtok(PATH, ":");
    // checking for command in PATH
    while( PATHTok != NULL ) {

        path = (char*)calloc(strlen(PATHTok) + strlen(commandName) + 2, 1);
        // concats the path and the name into buffer
        sprintf(path, "%s/%s", PATHTok, commandName);

        // checks if the path is accessable 
        if ( !access(path, X_OK) ) {
             free(PATH);
             return path;
        }

        free(path);

        // gets the next token
        PATHTok = strtok(NULL, ":");
    }

    return NULL;
}
