#ifndef CLOC
#define CLOC
#include "global.h"
// counts lines
void countLines(int* blank, int* comment, int* code, const char* langComment, char* langBlockComment, char* langBlockCommentRev, char* file){
    char line[300];
    // opens files in read mode
    FILE *f = fopen(file, "r");
    int cmntLock = 0;
    while(fgets(line, 300, f)) {

        // if a block comment is detected it counts the lines until it finds another occurence of the block comment closing
        if(cmntLock){
            regex_t checkBlockComment;
            char patternBlockComment[20];
            int matchBlockCmnt = -1;
            // checks if there is a closing of the block comment at the end of the line 
            sprintf(patternBlockComment, "^[[:space:]]*.*%s", langBlockCommentRev);
            matchBlockCmnt = regcomp(&checkBlockComment, patternBlockComment, 0);
            matchBlockCmnt = regexec(&checkBlockComment, line, 0, NULL, 0);
            (*comment)++;

            if(!matchBlockCmnt) cmntLock = 0;

        }
        else{
            regex_t checkBlank;
            // checks if the line is a blank line 
            int matchBlank = regcomp(&checkBlank, "^[[:space:]]*$", 0);;
            matchBlank = regexec(&checkBlank, line, 0, NULL, 0);
            regfree(&checkBlank);
            

            regex_t checkComment;
            char patternComment[20];
            int matchCmnt = -1;
            // checks if there is a comment for the language since there is none for Text
            if(langComment){
                // check if there is a comment at the start of the line with optional spaces before
                sprintf(patternComment, "^[[:space:]]*%s.*", langComment);
                matchCmnt = regcomp(&checkComment, patternComment, 0);
                matchCmnt = regexec(&checkComment, line, 0, NULL, 0);
                regfree(&checkComment);
            } 

            regex_t checkBlockComment;
            char patternBlockComment[40];
            int matchBlockCmnt1 = -1;
            int matchBlockCmnt2 = -1;
            // checks if the language has block comments
            if (langBlockComment){
                // checks if the block comment is at the start of the line
                sprintf(patternBlockComment, "^[[:space:]]*%s.*", langBlockComment);
                matchBlockCmnt1 = regcomp(&checkBlockComment, patternBlockComment, 0);
                matchBlockCmnt1 = regexec(&checkBlockComment, line, 0, NULL, 0);

                // checks if the block comment  is on the same line
                sprintf(patternBlockComment, "^[[:space:]]*%s.*%s.*", langBlockComment, langBlockCommentRev);
                matchBlockCmnt2 = regcomp(&checkBlockComment, patternBlockComment, 0);
                matchBlockCmnt2 = regexec(&checkBlockComment, line, 0, NULL, 0);
                // if there is a block comment at the start only then it locks the counter so that it counts the lines 
                // until it finds the closing of the block comments
                if(matchBlockCmnt2 && !matchBlockCmnt1) cmntLock = 1;
                regfree(&checkBlockComment);
            }


            // increments variables depending of the matches
            if(!matchBlockCmnt1 || !matchBlockCmnt2 || !matchCmnt){
                (*comment)++;
            } 
            else if (!matchBlank){
                (*blank)++;
            } else {
                (*code)++;
            }
        }
    }
}
void printCloc (int numberOfFilesProcessed, int numberOfFilesIgnored, int numberOfFilesFound, const char* langs[], int files[], int blank[], int code[], int comments[], int sum[]){

    // prints the results
    printf("\t%d text files.\n", numberOfFilesFound);
    printf("\t%d unique files.\n", numberOfFilesProcessed);
    printf("\t%d files ignored.\n", numberOfFilesIgnored);
    printf("-----------------------------------------------------------\n");
    printf("%-20s%-10s%-10s%-10s%-10s\n", "Language", "files", "blank", "comment", "code");
    for(int i = 0; i < 4; i++){
        printf("%-20s%-10d%-10d%-10d%-10d\n", langs[i], files[i], blank[i], comments[i], code[i]);
    }
    printf("-----------------------------------------------------------\n");
    printf("%-20s%-10d%-10d%-10d%-10d\n", "SUM:", sum[0], sum[1], sum[2], sum[3]);
    printf("-----------------------------------------------------------\n");


}
#define NUMOFLANGS 4

void handleFiles(char* path)
{
    regex_t regex;
    struct dirent *files;
    DIR *dir = opendir(path);

    int numberOfFilesProcessed = 0;
    int numberOfFilesIgnored = 0;
    int numberOfFilesFound;

    // a list of supported langs
    const char* langs[NUMOFLANGS] = {"C","C++", "Python", "Text"};
    // a list of the extension of the supported langs
    const char* langExtensions[NUMOFLANGS - 1] = {".c",".cpp", ".py"};
    // a list of the comments for their respective file type
    const char* langComment[NUMOFLANGS] = {"//", "//", "#", NULL};
    // a list of the starting block comments for their respective file type
    char* langBlockComment[NUMOFLANGS] = {"/\\*", "/\\*", "\"\"\"", NULL};
    // a list of the closing block comments for their respective file type
    char* langBlockCommentRev[NUMOFLANGS] = {"\\*/", "\\*/", "\"\"\"", NULL};

    // line types we are counting with 4 slots to each respective language
    int langFils[NUMOFLANGS] = {0, 0, 0, 0};
    int langBlnk[NUMOFLANGS] = {0, 0, 0, 0};
    int langCmnt[NUMOFLANGS] = {0, 0, 0, 0};
    int langCode[NUMOFLANGS] = {0, 0, 0, 0};
    int totalSum[NUMOFLANGS] = {0, 0, 0, 0};

    if (dir == NULL){
       printf("Directory cannot be opened!" );
       return;
    }
    int match = regcomp( &regex, ".\\..", 0);;
    while ((files = readdir(dir)) != NULL){

        //checks if the file is . or .. and skips if true
        if(!strcmp(files->d_name, ".")||!strcmp(files->d_name, "..")) continue;
        // checks if the file doesnt have an extension 
        match = regexec( &regex, files->d_name, 0, NULL, 0);
        if(!match){
            int i = NUMOFLANGS - 1;

            // gets the extension of the file
            char *extension = strrchr(files->d_name, '.');
            for(int j = 0; j < NUMOFLANGS - 1; j++) {
                // matches the 
                if(!strcmp(extension, langExtensions[j])) {
                    i = j;
                    break;
                }
            }
            // increments the files
            langFils[i]++;
            char* filePath = (char* )malloc(strlen(path) + strlen(files->d_name) + 2);
            sprintf(filePath, "%s/%s", path, files->d_name);
            countLines(&langBlnk[i], &langCmnt[i], &langCode[i], langComment[i], langBlockComment[i], langBlockCommentRev[i], filePath);
            numberOfFilesProcessed++;
        } else {
            // increments if the file was ignored
            numberOfFilesIgnored++;
        }
    }
    // gets the total number of files found
    numberOfFilesFound = numberOfFilesProcessed + numberOfFilesIgnored;
    // calculates the total sum of all the line types we are counting
    for(int i = 0; i < NUMOFLANGS; i++) {
        totalSum[0] += langFils[i];
        totalSum[1] += langBlnk[i];
        totalSum[2] += langCmnt[i];
        totalSum[3] += langCode[i];
    }
    printCloc(numberOfFilesProcessed, numberOfFilesIgnored, numberOfFilesFound, langs, langFils, langBlnk, langCode, langCmnt, totalSum);
    closedir(dir);
}
#endif
