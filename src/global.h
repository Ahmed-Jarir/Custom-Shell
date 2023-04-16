#ifndef GLOB
#define GLOB

#include <sys/socket.h>
#include <linux/netlink.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>
#include <dirent.h>
#include <regex.h>
#include <fcntl.h>
#include <graphviz/cgraph.h>
#include <time.h>

const char *sysname = "mishell";

enum return_codes {
    SUCCESS = 0,
    EXIT = 1,
    UNKNOWN = 2,
};

struct command_t {
    char *name;
    bool background;
    bool auto_complete;
    int arg_count;
    char **args;
    char *redirects[3]; // in/out redirection
    struct command_t *next; // for piping
    char* full_command; // this was added so that the custom command can easily access the full command and store it 
                        // it was not used anywhere else
};
#endif
