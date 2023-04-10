#include "global.h"
#include "command.h"
#include "prompt.h"
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
        }

        free_command(command);
    }

    printf("\n");
    return 0;
}
