#include "cloc.h"
int checkRepetition(char* path, char* topTenPaths[], int arrSize){
    for(int i = 0; i < arrSize; i++){
        if(!strcmp(path, topTenPaths[i])){
            return 1;
        }
    }
    return 0;
}
char* cdh() {
    //TODO: clean up
    char cdh[100];
    char buff[1000];

    sprintf(cdh, "%s/.cd_history", getenv("HOME"));

    FILE* cdhr = fopen(cdh, "r");
    int i = 0;
    // gets the number of lines in the file
    while(fgets(buff, 1000 - 1, cdhr) != NULL) i++;
    

    rewind(cdhr);

    char* listOfDirs[i];
    char letters = 'a';
    int j = i;
    // gets the content of the file
    while(fgets(buff, 1000 - 1, cdhr) != NULL) {
        buff[strcspn(buff, "\n")] = 0;
        i--;
        listOfDirs[i] = strdup(buff);
    }

    char* topTen[10];

    int num = 1;
    for(i = 0; i < j; i++) {
        if (!checkRepetition(listOfDirs[i], topTen, num - 1)) {
            topTen[num - 1] = listOfDirs[i];
            printf("%c %d) %s\n", letters++, num++, listOfDirs[i]);
        }
        if (num == 11) break;
    }
    char inputChar;
    int inputIdx;
    printf("Select directory by letter or number: ");
    if (scanf("%c", &inputChar) == -1){
        printf("scan faild");
    }
    if (inputChar == '\n') return NULL;

    inputChar = inputChar > 96 ? inputChar - 49 : inputChar - 1;
    inputIdx = atoi(&inputChar);
	return topTen[inputIdx];
}
