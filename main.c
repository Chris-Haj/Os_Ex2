#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>

void loop();
int countWords(char *commands);

int main() {
    loop();
    return 0;
}

void loop() {
    char commandWords[512];
    pid_t child;
    while(1){
        printf("Please enter your command or done to exit\n");
        fgets(commandWords,512,stdin);

        if (strcmp(commandWords,"done\n")==0)
            break;
        int words = countWords(commandWords);
        char *argv[words+1];
        argv[words]=NULL;
        int start=0,end,index=0;
        for(int i=0;commandWords[i]!='\n';i++){
            if(commandWords[i]==' '||commandWords[i+1]=='\n'){
                end=i;
                if(commandWords[i+1]=='\n')
                    end++;
                argv[index] = (char *) calloc((end-start)+1,sizeof(char));
                strncpy(argv[index],&commandWords[start],end-start);
                start=end+1;
                index++;
            }
        }

        child = fork();
        if (child==0)
            execvp(argv[0], argv);
        wait(NULL);
        for(int i=0;i<words;i++)
            free(argv[i]);
    }
}

int countWords(char *commands){
    int words=1;
    while(*commands!='\n'){
        if(*commands==' ')
            words++;
        commands++;
    }
    return words;
}
