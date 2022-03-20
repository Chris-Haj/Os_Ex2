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
#define PATH_LENGTH 100

void loop();
void checkInput(FILE *file, char *input, size_t i);
int wordCounter(FILE *file, const char *line, size_t i, int mode);
void executeCommand(char *argv[], char *line, size_t size);
void readHistory(FILE *file);


int numberOfCommands = 1;
int totalNumberOfWords = 0;
int running=1;

int main() {
    loop();
    return 0;
}

//Main loop function to keep asking user for input and call other functions according to what is passed into the stdin stream
void loop() {
    char location[PATH_LENGTH];
    const char *const PATH = getcwd(location, PATH_LENGTH);

    FILE *file = fopen(FILENAME, "a+");
    if (file == NULL)
        fprintf(stderr, "Error trying to open file!\n");
    char input[LENGTH] = "";
    while (running) {
        printf("%s> ", PATH);
        fgets(input, LENGTH, stdin);
        fflush(stdin);
        size_t i = 0;
        while (input[i] == ' ') i++;
        if (input[i] == '\0' || input[i] == '\n') {
            fprintf(stderr,
                          "Please enter a command!\n"
                                 "Empty input or input of only spaces is not allowed!\n");
            continue;
        }
        int lastChar = (int) strlen(input) - 2;
        if (!(i == 0 && lastChar >= 0 && input[lastChar] != ' ')) {
            fprintf(stderr, "Spaces before or after a command is not allowed!\n");
            continue;
        }
        /*
         * After skipping all the spaces that were located at the beginning (if there were any)
         * we check if the first 4 letters are exit or if the first 7 letters are history
         * if one of them is true we call the wordCounter function in mode 1 or 2 to check if only exit or history are in the input (ignoring spaces)
         * if that is true we either read the history or exit from the program.
         */
        checkInput(file, input, i);
    }
}
/*
 * checkInput is used to check what kind of command/input was passed through.
 * If one of the commands (done/history/cd) were entered with no other words then the program either terminates/prints out all previously entered commands, or a cd error
 * If the input wasn't any of said commands, then the program will treat the input as a shell command and create an array of pointers of returned size from wordCounter+1 and this array
 * is sent to the executeCommand function.
*/
void checkInput(FILE *file, char *input, size_t i) {
    if (strncmp(input, EXIT, EXIT_LENGTH) == 0 && input[EXIT_LENGTH] == '\n') {
        fclose(file);
        printf("Num of commands: %d\n", numberOfCommands);
        printf("Total number of words in all commands: %d!\n", totalNumberOfWords);
        running=0;
        return;
    } else if (strncmp(input, HISTORY, HISTORY_LENGTH) == 0 && input[HISTORY_LENGTH] == '\n') {
        i += HISTORY_LENGTH;
        wordCounter(file, input, i, 1);
    } else if (strncmp(&input[i], CD, CD_LENGTH) == 0 && (input[CD_LENGTH]==' '||input[CD_LENGTH] == '\n')) {
        fprintf(stderr,"Command not supported yet!\n");
        wordCounter(file, input, i, 0);
    } else {
        int argc = wordCounter(file, input, i, 0);
        char *argv[argc + 1];
        argv[argc] = NULL;
        executeCommand(argv, input, argc);
    }
    fprintf(file, "%s", input);
}

/*
 * the wordCounter function is used to control everything related to counting the
 * letters and the words of the input of the user and storing them in the history file.
 * mode 0 goes through everything normally.
 * mode 1 is used if exit was located in the input
 * mode 2 is used if history was located in the input
 */

int wordCounter(FILE *file, const char *line, size_t i, int mode) {
    int wordAmount = 1;
    int isHistory = 1;
    //After skipping all the spaces if the first index found was not a new line feed or the terminal character then that means we found 1 word

    while (line[i] != '\n') {
        if (line[i] != ' ') {
            isHistory = 0; //if history was found in the input and after skipping them we find another letter we switch the variable to 0 (False)
        } else if (line[i] == ' ') {
            while (line[i + 1] == ' ')
                i++;
            wordAmount++;
        }
        i++;
    }
    numberOfCommands++;
    totalNumberOfWords += wordAmount;

    if (isHistory == 1 && mode == 1) { //If the word "history" was entered by itself, then the program will print out the contents of file.txt which stores previously entered commands.
        if (file == NULL)
            fprintf(stderr, "Error trying to open file!\n");
        else
            readHistory(file);
    }
    return wordAmount;
}

//the readHistory function is simple function used to reopen the file in read mode and pass through all lines in the file while printing them to the terminal in a numbered fashion.
void readHistory(FILE *file) {
    rewind(file);
    if (file != NULL) {
        char currentLine[LENGTH];
        int counter = 1;
        while (fgets(currentLine, LENGTH, file))
            printf("%d: %s", counter++, currentLine);
    }
    else
        fprintf(stderr,"Error receiving file from function");
}
/*After receiving the array of pointers, this function goes through the input again,
 * while placing each word in a separate index
 * */
void executeCommand(char *argv[], char *line, size_t size) {
    int start = 0, end = 0, index = 0;
    for (int i = 0; line[i] != '\n'; i++) {
        if (start > end)
            end = i = start;
        if (line[i] == ' ' || line[i + 1] == '\n') {
            end = i;
            if (line[i + 1] == '\n')
                end++;
            int CurWordSize = (end - start) + 1;
            argv[index] = (char *) calloc(CurWordSize, sizeof(char));
            if (!argv[index]) {
                fprintf(stderr, "Error allocating memory!\n");
            }
            strncpy(argv[index], &line[start], end - start);
            start = end + 1;
            while (line[start] == ' ') start++;
            if (line[start] == '\n')
                break;
            index++;
        }
    }
    /*after setting up the argv array the fork function is called and the array is sent to the execvp function in the child program, which processes the array as
     * if the input was entered in a linux shell*/
    pid_t child = fork();
    if (child<0){
        perror("fork error");
    }
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