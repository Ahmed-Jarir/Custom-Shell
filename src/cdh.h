#ifndef CDH
#define CDH
#include "global.h"
#define MAX_PATH_LEN 100
#define MAX_LINE_LEN 1000
#define MAX_DIRS 10
// saves the directory into the ~/.cd_history file like bash uses ~/.bash_history in the home directory
void saveCdh(char pwd[]){
    char cdh[100];
    sprintf(cdh, "%s/.cd_history", getenv("HOME"));
    FILE *cdhf = fopen(cdh, "a+");
    fprintf(cdhf, "%s\n", pwd);
    fclose(cdhf);
}

int CheckRepetition(char* path, char** topDirs, int numDirs) {
    for(int i = 0; i < numDirs; i++) {
        if(strcmp(path, topDirs[i]) == 0) {
            // returns 1 if there is repitition 
            return 1;
        }
    }
    return 0;
}

char* Cdh() {
    // path to the cd history file
    char cdhPath[MAX_PATH_LEN];
    sprintf(cdhPath, "%s/.cd_history", getenv("HOME"));

    // open the cd history file with read flag
    FILE* cdhFile = fopen(cdhPath, "r");
    if(cdhFile == NULL) {
        printf("Failed to open cd history file\n");
        return NULL;
    }

    // reads the file content and stores them into an array
    char* dirs[MAX_DIRS];
    int numLines = 0;
    char buff[MAX_LINE_LEN];
    while(fgets(buff, MAX_LINE_LEN, cdhFile) != NULL) {

        // Remove the newline character at the end of the line
        buff[strcspn(buff, "\n")] = 0;
        dirs[numLines] = strdup(buff);
        numLines++;
    }
    fclose(cdhFile);

    char* latestTenDirs[MAX_DIRS];
    int numLatestDirs = 0;
    for(int i = numLines - 1; i >= 0 && numLatestDirs < MAX_DIRS; i--) {

        // check if the directory is already in the top 10
        if(!CheckRepetition(dirs[i], latestTenDirs, numLatestDirs)) {
            latestTenDirs[numLatestDirs] = dirs[i];
            numLatestDirs++;
        }

    }

    // prints options
    for(int i = 0; i < numLatestDirs; i++) printf("%c %d) %s\n", 'a' + i, i + 1, latestTenDirs[i]);

    // get user input
    printf("Select directory by letter or number: ");
    char* inputStr = (char*)malloc(sizeof(char));
    if (scanf("%s", inputStr) == -1){
        printf("scan faild\n");
    }
    if (strlen(inputStr) > 1 && strcmp(inputStr, "10")) {
        printf("invalid input: %s\n", inputStr);
        return NULL;
    }

    if(inputStr[0] == '\n') {
        return NULL;
    }
    int inputIdx;
    // check if its a letter or not and get the index
    if(inputStr[0] >= 'a' && inputStr[0] < 'a' + numLatestDirs) {
        inputIdx = inputStr[0] - 'a';
    } else {
        inputIdx = atoi(inputStr) - 1;
    }
    // check if the input is valid
    if(inputIdx < 0 || inputIdx >= numLatestDirs) {
        printf("Invalid input\n");
        return NULL;
    } 
    if(latestTenDirs[inputIdx]){
        saveCdh(latestTenDirs[inputIdx]);
    }
    return latestTenDirs[inputIdx];
}
#endif
