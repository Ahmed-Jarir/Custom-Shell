#ifndef PSVIS
#define PSVIS
#include <fcntl.h>
#include "global.h"
/* #include "module/mymodule.c" */
struct node {
    int process_pid;
    int parent_pid;
    unsigned long creation_time;
    int eldest_child_index;
    int number_of_children;
    struct node **children;
};

char* find_module(const char* module_name) {
    char command[256];
    snprintf(command, sizeof(command), "find %s -path \"*/project-1---shell-future-gadget-lab/module/%s\"",getenv("HOME"),module_name);

    FILE* fp = popen(command, "r");
    if (fp == NULL) {
        perror("Failed to run find command");
        return NULL;
    }

    static char module_path[256];
    if (fgets(module_path, sizeof(module_path), fp) == NULL) {
        fprintf(stderr, "Module not found\n");
        pclose(fp);
        return NULL;
    }

    size_t len = strlen(module_path);
    if (module_path[len - 1] == '\n') {
        module_path[len - 1] = 0; // Remove newline character
    }

    pclose(fp);
    return module_path;
}
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
    const char* mod_file = "mymodule.ko";
    const char* mod_name = "mymodule";

    char* module_path = find_module(mod_file);
    if (load_module(module_path, pid)) return 1;
    if (unload_module(mod_name)) return 1;

    return 0;
}
#endif
