#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <termios.h> // termios, TCSANOW, ECHO, ICANON
#include <unistd.h>
#include <dirent.h>
#include <regex.h>
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
};

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
 * Show the command prompt
 * @return [description]
 */
int show_prompt() {
	char cwd[1024], hostname[1024];
	gethostname(hostname, sizeof(hostname));
    getcwd(cwd, sizeof(cwd));
	printf("%s@%s:%s %s ~> ", getenv("USER"), hostname, cwd, sysname);
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

void prompt_backspace() {
	putchar(8); // go back 1
	putchar(' '); // write empty over
	putchar(8); // go back 1 again
}

/**
 * Prompt a command from the user
 * @param  buf      [description]
 * @param  buf_size [description]
 * @return          [description]
 */
int prompt(struct command_t *command) {
	size_t index = 0;
	char c;
	char buf[4096];
	static char oldbuf[4096];

	// tcgetattr gets the parameters of the current terminal
	// STDIN_FILENO will tell tcgetattr that it should write the settings
	// of stdin to oldt
	static struct termios backup_termios, new_termios;
	tcgetattr(STDIN_FILENO, &backup_termios);
	new_termios = backup_termios;
	// ICANON normally takes care that one line at a time will be processed
	// that means it will return if it sees a "\n" or an EOF or an EOL
	new_termios.c_lflag &=
		~(ICANON |
		  ECHO); // Also disable automatic echo. We manually echo each char.
	// Those new settings will be set to STDIN
	// TCSANOW tells tcsetattr to change attributes immediately.
	tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);

	show_prompt();
	buf[0] = 0;

	while (1) {
		c = getchar();
		// printf("Keycode: %u\n", c); // DEBUG: uncomment for debugging

		// handle tab
		if (c == 9) {
			buf[index++] = '?'; // autocomplete
			break;
		}

		// handle backspace
		if (c == 127) {
			if (index > 0) {
				prompt_backspace();
				index--;
			}
			continue;
		}

		if (c == 27 || c == 91 || c == 66 || c == 67 || c == 68) {
			continue;
		}

		// up arrow
		if (c == 65) {
			while (index > 0) {
				prompt_backspace();
				index--;
			}

			char tmpbuf[4096];
			printf("%s", oldbuf);
			strcpy(tmpbuf, buf);
			strcpy(buf, oldbuf);
			strcpy(oldbuf, tmpbuf);
			index += strlen(buf);
			continue;
		}

		putchar(c); // echo the character
		buf[index++] = c;
		if (index >= sizeof(buf) - 1)
			break;
		if (c == '\n') // enter key
			break;
		if (c == 4) // Ctrl+D
			return EXIT;
	}

	// trim newline from the end
	if (index > 0 && buf[index - 1] == '\n') {
		index--;
	}

	// null terminate string
	buf[index++] = '\0';

	strcpy(oldbuf, buf);

	parse_command(buf, command);

	/* print_command(command); // DEBUG: uncomment for debugging */

	// restore the old settings
	tcsetattr(STDIN_FILENO, TCSANOW, &backup_termios);
	return SUCCESS;
}

int process_command(struct command_t *command);

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
char* getBinPath(char* commandName){
    // add comments
    char* path = NULL;
    char cwd[1024];
    char** PATHArr;
    char* PATHcpy;
    char* PATHTok;
    char* buffer;
    int i = 0;

    PATHcpy =  strdup(getenv("PATH"));
    PATHArr = (char **)malloc(200 * sizeof(char* ));
    if ( !access(commandName, X_OK) ) {
        path = commandName;
        goto FREE;
    }

    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        buffer = (char*)calloc(strlen(cwd) + strlen(commandName) + 2, 1);

        sprintf(buffer, "%s/%s", cwd, commandName);
        if ( !access(buffer, X_OK) ) {
            path = buffer;
            goto FREE;
        }
    }

    PATHTok = strtok(PATHcpy, ":");

    while( PATHTok != NULL ) {

       PATHArr[i] = PATHTok;

       i++;

       PATHTok = strtok(NULL, ":");
    }

    i = 0;
    while(PATHArr[i] != NULL){

        buffer = (char*)calloc(strlen(PATHArr[i]) + strlen(commandName) + 2, 1);

        sprintf(buffer, "%s/%s", PATHArr[i], commandName);

        if ( !access(buffer, X_OK) ) {
            path = buffer;
            goto FREE;
        }

        free(buffer);
        i++;
    }

    FREE:   
        free(PATHcpy);
        free(PATHTok);
        free(PATHArr);
    return path;
}
void countLines(int* files, int* blank, int* comment, int* code, char* file){
    char line[300];
    FILE *f = fopen(file, "r");
    while(fgets(line, 300, f)) {
            int i = 0;
            int len = strlen(line);
            (*blank)++;
            for (i = 0; i < len; i++) {
                if (line[i] != '\n' && line[i] != '\t' && line[i] != ' ') {
                    (*blank)--;
                    break;
                }
            }
        }
}
void printCloc (int numberOfFilesProcessed, int numberOfFilesIgnored, int numberOfFilesFound,const char* langs[], int files[], int blank[], int code[], int comments[], int sum[]){
    printf("\t%d text files.\n", numberOfFilesFound);
    printf("\t%d unique files.\n", numberOfFilesProcessed);
    printf("\t%d files ignored.\n", numberOfFilesIgnored);
    printf("-------------------------------------------------\n");
    printf("%-20s%-8s%-8s%-8s%-8s\n", "Language", "files", "blank", "comment", "code");
    for(int i = 0; i < 4; i++){
        printf("%-20s%-8d%-8d%-8d%-8d\n", langs[i], files[i], blank[i], comments[i], code[i]);
    }
    printf("-------------------------------------------------\n");
    printf("%-20s%-8d%-8d%-8d%-8d\n", "SUM:", sum[0], sum[1], sum[2], sum[3]);
    printf("-------------------------------------------------\n");


}
void listFiles(const char *path)
{
    const int numberOfLangs = 4;
    regex_t regex;
    struct dirent *files;
    DIR *dir = opendir(".");
    int numberOfFilesProcessed = 0;
    int numberOfFilesIgnored = 0;
    int numberOfFilesFound;
    const char* langs[4] = {"C","C++", "Python", "Text"};
    const char* langExtensions[3] = {".c",".cpp", ".py"};
    const char* langComment[3] = {"//","//", "#"};
    int langFils[4] = {0, 0, 0, 0};
    int langBlnk[4] = {0, 0, 0, 0};
    int langCmnt[4] = {0, 0, 0, 0};
    int langCode[4] = {0, 0, 0, 0};
    int totalSum[4] = {0, 0, 0, 0};

    if (dir == NULL){
       printf("Directory cannot be opened!" );
       return;
    }
    int match = regcomp( &regex, ".\\..", 0);;
    while ((files = readdir(dir)) != NULL){

        match = regexec( &regex, files->d_name, 0, NULL, 0);
        if(!match){
            int i = 3;
            char *extension = strrchr(files->d_name, '.');
            for(int j = 0; j < numberOfLangs - 1; j++) {
                if(!strcmp(extension, langExtensions[j])) {
                    i = j;
                    break;
                }
            }
            countLines(&langFils[i], &langBlnk[i], &langCmnt[i], &langCode[i], files->d_name);
            numberOfFilesProcessed++;
        } else {
            numberOfFilesIgnored++;
        }
    }
    numberOfFilesFound = numberOfFilesProcessed + numberOfFilesIgnored;
    printCloc(numberOfFilesProcessed, numberOfFilesIgnored, numberOfFilesFound, langs, langFils, langBlnk, langCmnt, langCode, totalSum);
    closedir(dir);
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
        char cdh[100];
        char* listOfDirs[10];
        char letters = 'a';
        char buff[1000];

        sprintf(cdh, "%s/.cd_history", getenv("HOME"));
        if (!access(cdh, F_OK)){
            FILE* cdhf = fopen(cdh, "a+");
            int i = 1;
            while(fgets(buff, 1000 - 1, cdhf) != NULL) {
                buff[strcspn(buff, "\n")] = 0;
                printf("%c %d) %s\n", letters++, i, buff);
                listOfDirs[i - 1] = strdup(buff);
                i++;
            }
            char inputChar;
            int inputIdx;
            printf("Select directory by letter or number: ");
            if (scanf("%c", &inputChar) == -1){
                printf("scan faild");
            }

            inputChar = inputChar > 96 ? inputChar - 49 : inputChar - 1;
            inputIdx = atoi(&inputChar);
			r = chdir(listOfDirs[inputIdx]);
			if (r == -1) {
				printf("-%s: %s: %s\n", sysname, command->name,
					   strerror(errno));
			} 
            return SUCCESS;

        }
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
        // TODO
        char buff[100];
        listFiles(getcwd(buff, sizeof(buff)));
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
            args[i + 1] = NULL;

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
