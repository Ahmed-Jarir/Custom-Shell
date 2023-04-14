#ifndef PSVIS
#define PSVIS
#include <fcntl.h>
#include "global.h"
#include "findFile.h"
/* #include "module/mymodule.c" */
struct node {
    int process_pid;
    int parent_pid;
    unsigned long creation_time;
    int eldest_child_index;
    int number_of_children;
    struct node **children;
};

int load_module(char* module_path, int pid)
{
    if (system(NULL) == 0) {
        fprintf(stderr, "No shell is available to execute the command\n");
        return 1;
    }

    char command[256];
    snprintf(command, sizeof(command), "sudo insmod %s pid=%d", module_path, pid);

    int result = system(command);
    if (result != 0) {
        return 1;
    }
    /* else { */
    /*     printf("Kernel module loaded successfully\n"); */
    /* } */

    return 0;
}
int unload_module(const char* module_name) {
    if (system(NULL) == 0) {
        fprintf(stderr, "No shell is available to execute the command\n");
        return 1;
    }

    char command[256];
    snprintf(command, sizeof(command), "sudo rmmod %s", module_name );

    int result = system(command);
    if (result != 0) {
        return 1;
    }

    return 0;
}

int psvis(int pid) {
    const char* mod_file = "module/mymodule.ko";
    const char* mod_name = "mymodule";

    char* module_path = find_file(mod_file);
    if (load_module(module_path, pid)) return 1;
    if (unload_module(mod_name)) return 1;

    return 0;
}
#endif
