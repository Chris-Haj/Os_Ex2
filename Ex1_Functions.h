
#ifndef OS_EX2_EX1_FUNCTIONS_H
#define OS_EX2_EX1_FUNCTIONS_H
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define LENGTH 512
#define FILENAME "file.txt"
#define HISTORY "history"
#define EXIT "done"
#define CD "cd"
#define CD_LENGTH strlen(CD)
#define EXIT_LENGTH strlen(EXIT)
#define HISTORY_LENGTH strlen(HISTORY)

int numberOfCommands=1;

void error();
void readHistory();
int operations(size_t i, int mode, FILE *file, char *line);
void loop();

//Main loop function to keep asking user for input and call other functions according to what is passed in to the string buffer
void loop() {
    int size = 100;
    char location[size];
    strcpy(location, getcwd(location,size));
    char commandWords[512];
    pid_t child;
    //Length is equal to 512 because the last index contains \0 and the before last index contains
    // \n from stdin so the input will fit exactly 510 characters!
    FILE *file = fopen(FILENAME, "a+");
    if(file == NULL)
        error();
    char input[LENGTH] = "";
    while (1) {
        printf("%s>\t",location);
        fgets(input,LENGTH,stdin);

        size_t i=0;
        while (input[i]==' ') i++; //skip all spaces at the start from input

        /*
         * After skipping all the spaces that were located at the beginning (if there were any)
         * we check if the first 4 letters are exit or if the first 7 letters are history
         * if one of them is true we call the operations function in mode 1 or 2 to check if only exit or history are in the input (ignoring spaces)
         * if that is true we either read the history or exit from the program.
         */
        if (strncmp(&input[i], EXIT,EXIT_LENGTH) == 0) {
            i+=EXIT_LENGTH;
            if (operations(i,1,file,input)==1){
                printf("NUm of commands: %d\n",numberOfCommands);
                break;
            }
        }
        else if (strncmp(&input[i], HISTORY,HISTORY_LENGTH) == 0) {
            i+=HISTORY_LENGTH;
            operations(i,2,file,input);
        }
        else if(strncmp(&input[i], CD,CD_LENGTH) == 0){
            i+=CD_LENGTH;
            operations(i,3,file,input);
        }
        else
            operations(i, 0, file, input);
    }
}
/*
 * the operations function is used to control everything related to counting the
 * letters and the words of the input of the user and storing them in the history file.
 * mode 0 goes through everything normally.
 * mode 1 is used if exit was located in the input
 * mode 2 is used if history was located in the input
 */

int operations(size_t i, int mode, FILE *file, char *line) {
    int lettersAmount = 0;
    int wordAmount = 0;
    int exitOrHistory = 1;
    if(mode == 1)
        lettersAmount+= EXIT_LENGTH;
    else if(mode == 2)
        lettersAmount+= HISTORY_LENGTH;
    //After skipping all the spaces if the first index found was not a new line feed or the terminal character then that means we found 1 word
    if (line[i] != '\n' && line[i] != '\0')
        wordAmount++;

    while(line[i] != '\n') {
        if (line[i] != ' '){
            lettersAmount++;
            exitOrHistory=0; //if exit or history was found in the input and after skipping them we find another letter we switch the variable to 0 (False)
        }
        else if (line[i] == ' ') {
            while (line[i + 1] == ' ')
                i++;
            wordAmount++;
        }
        i++;
    }
    if(exitOrHistory == 1){
        if(mode == 1){
            printf("Program finished.");
            fclose(file);
            return 1;
        }
        if(mode==2){
            //if "history" is passed in we have to close the file, so it can save its contents, so they can be read.
            fclose(file);
            readHistory();
//           After reading the contents of the file, we reopen the file writer in append mode to continue writing in the file.
            file = fopen(FILENAME, "a+");

            if(file == NULL)
                error();

            return 0;
        }
        if(mode==3){
            printf("command not supported (Yet)");
            return 0;
        }
    }
    if(lettersAmount!=0) {
        numberOfCommands++;
        if(line[i-1]==' ')
            wordAmount--;
        printf("%d words\n%d chars\n", wordAmount, lettersAmount);
        if (line[i] == '\n')
            fprintf(file, "%s", line);
        else
            fprintf(file, "%s\n", line);
    }
    return 0;
}

//the readHistory function is simple function used to reopen the file in read mode and pass through all lines in the file while printing them to the terminal.
void readHistory(){
    FILE *file = fopen(FILENAME, "r");
    if(file != NULL) {
        char currentLine[LENGTH];
        int counter = 0;
        while (fgets(currentLine, LENGTH, file))
            printf("%d: %s", counter++, currentLine);

        fclose(file);
    }
    else
        error();
}

void error(){
    printf("Error occurred trying to open file\n");
    exit(1);
}
#endif //OS_EX2_EX1_FUNCTIONS_H
