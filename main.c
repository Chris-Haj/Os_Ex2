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
#define PATH_LENGTH 100

void loop();
void checkInput(FILE *file, char *input, size_t i, int fromHistory);
int wordCounter(const char *line, size_t i);
void executeCommand(char *argv[], char *line, size_t size);
void readHistory(FILE *file);
void cmdFromHistory(char *line);

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
            fprintf(stderr, "Please enter a command!\n"
                            "Empty input or input of only spaces is not allowed!\n");
            continue;
        }
        int lastChar = (int) strlen(input) - 2;
        if (!(i == 0 && lastChar >= 0 && input[lastChar] != ' ')) {
            fprintf(stderr, "Spaces before or after a command is not allowed!\n");
            continue;
        }
        if (input[0] == '!') {
            int IsANumber = 1;
            for(int j=1;input[j]!='\n';j++){
                if (!('0'<=input[j]&&input[j]<='9'))
                    IsANumber = 0;
            }
            if (IsANumber==0 ) {
                fprintf(stderr, "Please enter only numbers after the ! to execute a past command\n");
            }
            else{
                fclose(file);
                cmdFromHistory(&input[1]);
                file = fopen(FILENAME, "a+");
            }
            continue;
        }
        checkInput(file, input, i, 0);
    }
}
/*
 * checkInput is used to check what kind of command/input was passed through.
 * If one of the commands (done/history/cd) were entered with no other words then the program either terminates/prints out all previously entered commands, or a cd error
 * If the input wasn't any of said commands, then the program will treat the input as a shell command and create an array of pointers of returned size from wordCounter+1 and this array
 * is sent to the executeCommand function.
*/
void checkInput(FILE *file, char *input, size_t i, int fromHistory) {
    if (strncmp(input, EXIT, EXIT_LENGTH) == 0 && input[EXIT_LENGTH] == '\n') {
        fclose(file);
        printf("Num of commands: %d\n", numberOfCommands);
        printf("Total number of words in all commands: %d!\n", totalNumberOfWords);
        running=0;
        return;
    } else if (strncmp(input, HISTORY, HISTORY_LENGTH) == 0 && input[HISTORY_LENGTH] == '\n') {
        readHistory(file);
        numberOfCommands++,totalNumberOfWords++;
    } else if (strncmp(&input[i], CD, CD_LENGTH) == 0 && (input[CD_LENGTH]==' '||input[CD_LENGTH] == '\n')) {
        fprintf(stderr,"Command not supported yet!\n");
        wordCounter(input, i);
        return;
    } else {
        int argc = wordCounter(input, i);
        char *argv[argc + 1];
        argv[argc] = NULL;
        executeCommand(argv, input, argc);
    }
    if(fromHistory==0)
        fprintf(file, "%s", input);
    else
        fprintf(file, "%s\n", input);
}

/*
 * Function used to search for the specific command in the line number entered next to !
 * and execute it if the number entered is less or equal to than the total number of lines in the file
 */
void cmdFromHistory(char *line) {
    FILE *file = fopen(FILENAME, "r");
    char command[LENGTH];
    int lineNumber = (int) strtol(line,NULL,10);
    int cur = 0;
    while (cur < lineNumber && fgets(command, LENGTH, file) ){
        cur++;
    }
    if (cur < lineNumber) {
        fprintf(stderr, "Number of line does not exist yet!\n");
        return;
    }
    printf("%s",command);
    checkInput(file, command, 0, 1);
    fclose(file);
}
/*
 * the wordCounter function is used to count how many words are in the input and add it to the total
 * number of words entered and incrementing total number of commands by 1.
 */

int wordCounter(const char *line, size_t i) {
    int wordAmount = 1;
    while (line[i] != '\n') {
        if (line[i] == ' ') {
            while (line[i + 1] == ' ')
                i++;
            wordAmount++;
        }
        i++;
    }
    numberOfCommands++;
    totalNumberOfWords += wordAmount;
    return wordAmount;
}
//the readHistory function is simple function used to reopen the file in read mode and pass through all lines in the file while printing them to the terminal.
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
*/
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
/*
* after setting up the argv array the fork function is called and the array is sent to the execvp function in the child program, which processes the array as
* if the input was entered in a linux shell
*/
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