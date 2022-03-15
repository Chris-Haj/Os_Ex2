#ifndef OS_EX2_EX1_FUNCTIONS_H
#define OS_EX2_EX1_FUNCTIONS_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define LENGTH 514
#define FILENAME "file.txt"
#define HISTORY "history"
#define EXIT "done"
#define CD "cd"
#define CD_LENGTH strlen(CD)
#define EXIT_LENGTH strlen(EXIT)
#define HISTORY_LENGTH strlen(HISTORY)

int numberOfCommands = 1;
int totalNumberOfWords = 0;

void executeCommand(char *argv[], int size, char *line);

void error();

void readHistory();

int operations(size_t i, int mode, FILE *file, char *line);

void loop();

void cmdFromHistory(char *line);

void cmdFromHistory(char *line) {
    FILE *file = fopen(FILENAME, "r");
    char command[LENGTH];
    int lineNumber = atoi(line); // NOLINT(cert-err34-c)
    int cur = -1;
    while (fgets(command, LENGTH, file) && cur < lineNumber)
        cur++;
    if (cur < lineNumber) {
        fprintf(stderr, "Number of line does not exist yet!");
        return;
    }
    int argc = operations(0, 0, file, command);
    char *cmd[argc + 1];
    cmd[argc] = NULL;
    executeCommand(cmd, argc, command);
}

//Main loop function to keep asking user for input and call other functions according to what is passed in to the string buffer
void loop() {
    int size = 100;
    char location[size];
    strcpy(location, getcwd(location, size));
    //Length is equal to 512 because the last index contains \0 and the before last index contains
    // \n from stdin so the input will fit exactly 510 characters!
    FILE *file = fopen(FILENAME, "a+");
    if (file == NULL)
        error();
    char input[LENGTH] = "";
    while (1) {
        printf("%s>\t", location);
        fgets(input, LENGTH, stdin);
        fflush(stdin);
        size_t i = 0;
        if (input[0] == '!') {
            if (input[1] == '\n') {
                fprintf(stderr, "Please enter a number after the !\n");
                continue;
            }
            fclose(file);
            cmdFromHistory(&input[1]);
            file = fopen(FILENAME, "a+");
            continue;
        }
        while (input[i] == ' ') i++; //skip all spaces at the start from input
        if (input[i] == '\0' || input[i] == '\n') {

            fprintf(stderr, "Please enter a command!\n"
                            "Empty input or input of only spaces is not allowed!\n");
            continue;
        }
        /*
         * After skipping all the spaces that were located at the beginning (if there were any)
         * we check if the first 4 letters are exit or if the first 7 letters are history
         * if one of them is true we call the operations function in mode 1 or 2 to check if only exit or history are in the input (ignoring spaces)
         * if that is true we either read the history or exit from the program.
         */
        if (strncmp(input, EXIT, 4) == 0 && i == 0 && input[4] == '\n') {
            fclose(file);
            printf("Num of commands: %d\n", numberOfCommands);
            printf("Total number of words in all commands: %d!\n", totalNumberOfWords);
            exit(0);
        } else if (strncmp(&input[i], HISTORY, HISTORY_LENGTH) == 0) {
            i += HISTORY_LENGTH;
            operations(i, 2, file, input);
        } else if (strncmp(&input[i], CD, CD_LENGTH) == 0) {
            i += CD_LENGTH;
            operations(i, 3, file, input);
        } else {
            int lastChar = (int) strlen(input) - 2;
            if (i == 0 && lastChar>=0&& input[lastChar]!=' ') {
                int argc = operations(i, 0, file, input);
                char *argv[argc + 1];
                argv[argc] = NULL;
                executeCommand(argv, argc, input);
            }
            else
                fprintf(stderr,"Spaces before or after a command is not allowed!\n");
        }
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
    if (mode == 1)
        lettersAmount += EXIT_LENGTH;
    else if (mode == 2)
        lettersAmount += HISTORY_LENGTH;
    //After skipping all the spaces if the first index found was not a new line feed or the terminal character then that means we found 1 word
    if (line[i] != '\n' && line[i] != '\0')
        wordAmount++;

    while (line[i] != '\n') {
        if (line[i] != ' ') {
            lettersAmount++;
            exitOrHistory = 0; //if exit or history was found in the input and after skipping them we find another letter we switch the variable to 0 (False)
        } else if (line[i] == ' ') {
            while (line[i + 1] == ' ')
                i++;
            wordAmount++;
        }
        i++;
    }
    if (exitOrHistory == 1) {
        if (mode == 2) {
            //if "history" is passed in we have to close the file, so it can save its contents, so they can be read.
            fclose(file);
            readHistory();
//           After reading the contents of the file, we reopen the file writer in append mode to continue writing in the file.
            file = fopen(FILENAME, "a+");

            if (file == NULL)
                error();

            return 0;
        }
        if (mode == 3) {
            printf("command not supported (Yet)\n");
            if (line[i - 1] == ' ')
                wordAmount--;
            numberOfCommands++;
            totalNumberOfWords += wordAmount;
            return 0;
        }
    }
    if (lettersAmount != 0) {
        numberOfCommands++;
        if (line[i - 1] == ' ')
            wordAmount--;
        if (line[i] == '\n')
            fprintf(file, "%s", line);
        else
            fprintf(file, "%s\n", line);
        totalNumberOfWords += wordAmount;
    }
    return wordAmount;
}

//the readHistory function is simple function used to reopen the file in read mode and pass through all lines in the file while printing them to the terminal.
void readHistory() {
    FILE *file = fopen(FILENAME, "r");
    if (file != NULL) {
        char currentLine[LENGTH];
        int counter = 0;
        while (fgets(currentLine, LENGTH, file))
            printf("%d: %s", counter++, currentLine);

        fclose(file);
    } else
        error();
}

void executeCommand(char *argv[], int size, char *line) {
    int start = 0, end = 0, index = 0;
    for (int i = 0; line[i] != '\n'; i++) {
        if (start > end)
            end = i = start;
        if (line[i] == ' ' || line[i + 1] == '\n') {
            end = i;
            if (line[i + 1] == '\n')
                end++;
            argv[index] = (char *) calloc((end - start) + 1, sizeof(char));
            strncpy(argv[index], &line[start], end - start);
            start = end + 1;
            while (line[start] == ' ') start++;
            if (line[start] == '\n')
                break;
            index++;
        }
    }
    pid_t child = fork();
    if (child == 0) {
        if (execvp(argv[0], argv) == -1) {
            perror("execvp error");
        }
        exit(1);
    }
    wait(NULL);
    for (int i = 0; i < size; i++)
        free(argv[i]);
}

void error() {
    fprintf(stderr, "Error occurred trying to open file\n");
    exit(1);
}

#endif //OS_EX2_EX1_FUNCTIONS_H