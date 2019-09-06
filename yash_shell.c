#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>

char *prompt = "> ";

int main(){
    int cpid, status;
    char *cmd;

    while(cmd =readline(prompt)){
        cpid = fork();
        if(cpid == 0){
            //child
            printf("child cpid: %d",cpid);
            execlp(cmd,cmd,(char*)NULL);
        }
        else{
            //parent
            printf("parent cpid: %d",cpid);
            wait(&status);
        }
    }
}