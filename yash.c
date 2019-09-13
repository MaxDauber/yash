/**
 * EE461S - Project 1
 * Yash Shell
 * Max Dauber (mjd3375)
 */

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <wait.h>
#include <readline/readline.h>
#include <readline/history.h>

char *prompt = "> ";

void process(char* cmd){
    int status;
    int cpid = fork();

    // split cmd into array of strings
    char * cmd_array [100];
    int index = 0;
    cmd_array[index] = strtok(cmd," ");

    while(cmd_array[index] != NULL){
        cmd_array[++index] = strtok(NULL," ");
    }
    cmd_array[++index] = NULL;

    if(cpid == 0){
        execvp(cmd_array[0], cmd_array);
    }
    else{
        wait(&status);
    }
}

int main(){
    //initialize_env();

    while(1) {
        char* cmd = readline(prompt);
        if (!cmd) continue;
        process(cmd);
        //update jobs
    }
}