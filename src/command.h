#include "cdh.h"
/**
 * Prints a command struct
 * @param struct command_t *
 */
void print_command(struct command_t *command) {
	int i = 0;
	printf("Command: <%s>\n", command->name);
	printf("\tIs Background: %s\n", command->background ? "yes" : "no");
	printf("\tNeeds Auto-complete: %s\n",
		   command->auto_complete ? "yes" : "no");
	printf("\tRedirects:\n");

	for (i = 0; i < 3; i++) {
		printf("\t\t%d: %s\n", i,
			   command->redirects[i] ? command->redirects[i] : "N/A");
	}

	printf("\tArguments (%d):\n", command->arg_count);

	for (i = 0; i < command->arg_count; ++i) {
		printf("\t\tArg %d: %s\n", i, command->args[i]);
	}

	if (command->next) {
		printf("\tPiped to:\n");
		print_command(command->next);
	}
    printf("\tArgCount: %d\n", command->arg_count);
}

/**
 * Release allocated memory of a command
 * @param  command [description]
 * @return         [description]
 */
int free_command(struct command_t *command) {
	if (command->arg_count) {
		for (int i = 0; i < command->arg_count; ++i)
			free(command->args[i]);
		free(command->args);
	}

	for (int i = 0; i < 3; ++i) {
		if (command->redirects[i])
			free(command->redirects[i]);
	}

	if (command->next) {
		free_command(command->next);
		command->next = NULL;
	}

	free(command->name);
	free(command);
	return 0;
}

/**
 * Parse a command string into a command struct
 * @param  buf     [description]
 * @param  command [description]
 * @return         0
 */
int parse_command(char *buf, struct command_t *command) {
	const char *splitters = " \t"; // split at whitespace
	int index, len;
	len = strlen(buf);

	// trim left whitespace
	while (len > 0 && strchr(splitters, buf[0]) != NULL) {
		buf++;
		len--;
	}

	while (len > 0 && strchr(splitters, buf[len - 1]) != NULL) {
		// trim right whitespace
		buf[--len] = 0;
	}

	// auto-complete
	if (len > 0 && buf[len - 1] == '?') {
		command->auto_complete = true;
	}

	// background
	if (len > 0 && buf[len - 1] == '&') {
		command->background = true;
	}

	char *pch = strtok(buf, splitters);
	if (pch == NULL) {
		command->name = (char *)malloc(1);
		command->name[0] = 0;
	} else {
		command->name = (char *)malloc(strlen(pch) + 1);
		strcpy(command->name, pch);
	}

	command->args = (char **)malloc(sizeof(char *));

	int redirect_index;
	int arg_index = 0;
	char temp_buf[1024], *arg;

	while (1) {
		// tokenize input on splitters
		pch = strtok(NULL, splitters);
		if (!pch)
			break;
		arg = temp_buf;
		strcpy(arg, pch);
		len = strlen(arg);

		// empty arg, go for next
		if (len == 0) {
			continue;
		}

		// trim left whitespace
		while (len > 0 && strchr(splitters, arg[0]) != NULL) {
			arg++;
			len--;
		}

		// trim right whitespace
		while (len > 0 && strchr(splitters, arg[len - 1]) != NULL) {
			arg[--len] = 0;
		}

		// empty arg, go for next
		if (len == 0) {
			continue;
		}

		// piping to another command
		if (strcmp(arg, "|") == 0) {
			struct command_t *c = malloc(sizeof(struct command_t));
			int l = strlen(pch);
			pch[l] = splitters[0]; // restore strtok termination
			index = 1;
			while (pch[index] == ' ' || pch[index] == '\t')
				index++; // skip whitespaces

			parse_command(pch + index, c);
			pch[l] = 0; // put back strtok termination
			command->next = c;
			continue;
		}

		// background process
		if (strcmp(arg, "&") == 0) {
			// handled before
			continue;
		}

		// handle input redirection
		redirect_index = -1;
		if (arg[0] == '<') {
			redirect_index = 0;
		}

		if (arg[0] == '>') {
			if (len > 1 && arg[1] == '>') {
				redirect_index = 2;
				arg++;
				len--;
			} else {
				redirect_index = 1;
			}
		}

		if (redirect_index != -1) {
			command->redirects[redirect_index] = malloc(len);
			strcpy(command->redirects[redirect_index], arg + 1);
			continue;
		}

		// normal arguments
		if (len > 2 &&
			((arg[0] == '"' && arg[len - 1] == '"') ||
			 (arg[0] == '\'' && arg[len - 1] == '\''))) // quote wrapped arg
		{
			arg[--len] = 0;
			arg++;
		}

		command->args =
			(char **)realloc(command->args, sizeof(char *) * (arg_index + 1));

		command->args[arg_index] = (char *)malloc(len + 1);
		strcpy(command->args[arg_index++], arg);
	}
	command->arg_count = arg_index;

	// increase args size by 2
	command->args = (char **)realloc(
		command->args, sizeof(char *) * (command->arg_count += 2));

	// shift everything forward by 1
	for (int i = command->arg_count - 2; i > 0; --i) {
		command->args[i] = command->args[i - 1];
	}

	// set args[0] as a copy of name
	command->args[0] = strdup(command->name);

	// set args[arg_count-1] (last) to NULL
	command->args[command->arg_count - 1] = NULL;

	return 0;
}

int process_command(struct command_t *command) {
	int r;

	if (strcmp(command->name, "") == 0) {
		return SUCCESS;
	}

	if (strcmp(command->name, "exit") == 0) {
		return EXIT;
	}

	if (strcmp(command->name, "cd") == 0) {
        // add comments
		if (command->arg_count > 0) {
            char pwd[1024];
            getcwd(pwd, sizeof(pwd));
			r = chdir(command->args[1]);
			if (r == -1) {
				printf("-%s: %s: %s\n", sysname, command->name,
					   strerror(errno));
			} 
            else {
                char cdh[100];
                sprintf(cdh, "%s/.cd_history", getenv("HOME"));
                FILE *cdhf = fopen(cdh, "a+");
                fprintf(cdhf, "%s\n", pwd);
                fclose(cdhf);
            }

			return SUCCESS;
		}
	}

	if (!strcmp(command->name, "cdh")) {
        // TODO: implement the top 10 limit and remove duplicate directories
        // add comments
        char* path = Cdh();
        if (!path) return SUCCESS;
        r = chdir(path);
		if (r == -1) {
			printf("-%s: %s: %s\n", sysname, command->name,
				   strerror(errno));
		}
        return SUCCESS;
    }
	if (!strcmp(command->name, "roll")) {
        // add comments
        char* argTok;

        argTok = strtok(command->args[1], "d");

        int numberOfRolls;
        int maxOut;

        int args[2] = {0, 0};
        int i = 0;
        while( argTok != NULL  && i < 2) {
            args[i] = atoi(argTok);

            i++;
            argTok = strtok(NULL, "d");
        }

        numberOfRolls = i < 2 ? 1 : args[0];
        maxOut = args[i == 1?  0 : 1] + 1;

        char *printRolls = malloc(numberOfRolls * 3);
        int rollOut[numberOfRolls];
        int sum = 0;
        for (int j = 0; j < numberOfRolls; j++){

            rollOut[j] = rand() % maxOut;
            j == 0 ? sprintf(printRolls, "%d", rollOut[j]) :sprintf(printRolls, "%s + %d", strdup(printRolls), rollOut[j]);
            sum += rollOut[j];

        }
        if(numberOfRolls == 1)
        {
            printf("Rolled %d\n", sum);
        }else {
            printf("Rolled %d (%s)\n", sum, printRolls);
        }
        return SUCCESS;

    }
	if (!strcmp(command->name, "cloc")) {
        char buff[100];
        getcwd(buff, sizeof(buff));
        char* arg = command->args[1];
        if(!arg){
            arg = ".";
        }

        char *path = malloc(strlen(buff) + strlen(arg) + 2);
        sprintf(path, "%s/%s", buff, arg);
        handleFiles(path);
        return SUCCESS;

    }

    int fileDesc = -1;
    int stdoutCpy = dup(STDOUT_FILENO);
    int stdinCpy = dup(STDIN_FILENO);
    FILE *fil;
    char* flags[3] = {"r", "w", "a"};
    // TODO: fix the redirect to stdin
    for (int i = 0; i < 3; i++) {
        if (command->redirects[i]){
            fil = fopen(command->redirects[i], flags[i]);
            fileDesc = fileno(fil);
            if(fileDesc < 0) printf("error opening the file\n");
            dup2(fileDesc, i == 0 ? STDIN_FILENO : STDOUT_FILENO);
        }
    }
	pid_t pid = fork();
	// child
	if (pid == 0) {
		/// This shows how to do exec with environ (but is not available on MacOs)
		// extern char** environ; // environment variables
		// execvpe(command->name, command->args, environ); // exec+args+path+environ

		/// This shows how to do exec with auto-path resolve
		// add a NULL argument to the end of args, and the name to the beginning
		// as required by exec

		// TODO: do your own exec with path resolving using execv()
		// do so by replacing the execvp call below

        // to test `command &` using ls
        /* if(!strcmp(command->name,"ls")) { */
        /*     sleep(200); */
        /* } */
        // add comments

        char* path = getBinPath(command->name);
        if (path != NULL){
            char* args[command->arg_count + 2];
            args[0] = path;
            int i;
            for(i = 1; i < command->arg_count ; i++){
                args[i] = command->args[i];
            }
            /* args[i + 1] = NULL; */

            execv(path, args);

        }
		/* execvp(command->name, command->args); // exec+args+path */
		exit(0);
	} else {
		// TODO: implement background processes here
        // TODO: fix what happens after a process is put to the background
        if (!command->background) wait(0); // wait for child process to finish
        if(fileDesc > 0) {
            close(fileDesc); 
            dup2(stdoutCpy, 1);
            dup2(stdinCpy, 1);
        }

        return SUCCESS;
	}

	// TODO: your implementation here

	printf("-%s: %s: command not found\n", sysname, command->name);
	return UNKNOWN;
}
