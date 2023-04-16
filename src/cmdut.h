#ifndef CMDUT
#define CMDUT

#include "global.h"
#include "findFile.h"
void runCmdUt(struct command_t *command){
    char *file = find_file("src/cmdut.py");
    char cmd[strlen(command->full_command) + strlen(file) + 6] ;
    sprintf(cmd, "python %s",file);

    char **args = &command->args[1];
    int i = 0;
    while(args[i] != NULL) {

        // concatinates the command and the arguments 
        sprintf(cmd, "%s %s", strdup(cmd), args[i]);
        i++;
    }

    system(cmd);
}


#endif
