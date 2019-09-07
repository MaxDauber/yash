#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>

int main(){
    char *prompt = "> ";
    int cpid, status;
    char *cmd;

    while(cmd = readline(prompt)){
        cpid = fork();
        if(cpid == 0){
            //child
            execlp(cmd,cmd,(char*)NULL);
        }
        else{
            //parent
            //wait(&status);
        }
    }
}