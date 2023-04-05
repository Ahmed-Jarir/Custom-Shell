#include "cloc.h"
char* cdh() {
    // implement the command to get the latest 10 and with no repetition 
    char cdh[100];
    char* listOfDirs[10];
    char letters = 'a';
    char buff[1000];

    sprintf(cdh, "%s/.cd_history", getenv("HOME"));

    FILE* cdhf = fopen(cdh, "a+");
    int i = 0;
    while(fgets(buff, 1000 - 1, cdhf) != NULL) {
        buff[strcspn(buff, "\n")] = 0;
        listOfDirs[i] = strdup(buff);
        i++;
    }
    int num = 1;
    for(int j = i - 1; j > i - 10; j--) {
        printf("%c %d) %s\n", letters++, num++, listOfDirs[j]);
    }
    char inputChar;
    int inputIdx;
    printf("Select directory by letter or number: ");
    if (scanf("%c", &inputChar) == -1){
        printf("scan faild");
    }

    inputChar = inputChar > 96 ? inputChar - 49 : inputChar - 1;
    inputIdx = atoi(&inputChar);
	return listOfDirs[inputIdx];
    
    return NULL;
}
