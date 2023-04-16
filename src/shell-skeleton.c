#include "global.h"
#include "command.h"
#include "prompt.h"
#include "findFile.h"
int main() {
    while (1) {
        struct command_t *command = malloc(sizeof(struct command_t));

        // set all bytes to 0
        memset(command, 0, sizeof(struct command_t));

        int code;
        code = prompt(command);
        if (code == EXIT) {
            break;
        }

        code = process_command(command);
        if (code == EXIT) {
            break;
        } else if (code == SUCCESS){
            if(fork() == 0){
                char *file = find_file("src/cmdut.py");
                char cmd[1000];
                snprintf(cmd, sizeof(cmd), "python %s m -a \"%s\"",file, command->full_command);
                if (system(cmd)) {
                    printf("system call failed");
                    exit(1);
                }
                exit(0);
            }
        }

        free_command(command);
    }

    printf("\n");
    return 0;
}
