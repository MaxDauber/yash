/**
 * EE461S - Project 1
 * Yash Shell
 * Max Dauber (mjd3375)
 */

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include <wait.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <malloc.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>



#define MAX_LINE_LENGTH 1028
#define MAX_ARGS 100
#define PIPE "|"
#define BACKGROUND "&"
#define REDIR_IN "<"
#define REDIR_OUT ">"
#define REDIR_ERR "2>"


#define JOBS "jobs"
#define RUNNING "Running"
#define STOPPED "Stopped"
#define DONE "Done"

int stdin_c=0,stdout_c=1,stderr_c=2;


void set_operators(char*args[], int arg_count){

    int f_in=-1,f_out=-1,f_err=-1;

    for (int i = 0; i < arg_count - 1; i++) {

        if (strcmp(args[i], REDIR_OUT) == 0) { 
            if (i >= arg_count- 1) continue;
            
             args[i] = NULL;
            
            char *file = strdup(args[i+1]);
            f_out = open(file, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
            
            if (f_out < 0) {
                perror("cannot open file");
                return;
            } else {
                dup2(f_out, 1);
                close(f_out);
                ++i;
            }
        }
        if (strcmp(args[i], REDIR_IN) == 0) { 
            if (i >= arg_count- 1) continue;
            
            args[i] = NULL;
            
            char *file = args[i + 1];
            f_in = open(file, O_RDONLY, S_IRWXU);
            
            if (f_in < 0) {
                perror("cannot find file");
                return;
            } else {
                dup2(f_in, 0);
                close(f_in);
                ++i;
            }
        }
        if (strcmp(args[i], REDIR_ERR) == 0) { 
            if (i >= arg_count - 1) continue;
            
            args[i] = NULL;
            
            char *file = args[i+1];
            f_err = open(file, O_RDONLY | O_CREAT | O_TRUNC, S_IRWXU);
            
            if (f_err < 0) {
                perror("cannot find file");
                return;
            } else {
                dup2(f_err, 2);
                close(f_err);
                ++i;
            }
        } 

    }
}

void execute_cmd(char*cmd[], int arg_count, int bg){
    int pid = fork();
    if (pid < 0){
        perror("fork failed");
        // exit(1);
    }else if(pid == 0){
        set_operators(cmd, arg_count);
        execvp(cmd[0], cmd);
    }
    else{
        if(!bg){
            wait(NULL);
        }
    }
}

void process(char* cmd){
    char * args [MAX_ARGS];
    int arg_count = 0;
    args[arg_count] = strtok(cmd," ");

    while(args[arg_count] != NULL){
        args[++arg_count] = strtok(NULL," ");

    }
    args[++arg_count] = NULL;

    execute_cmd(args, arg_count, 0); 
}

int main(){
    //initialize_env();

    while(1) {
        char* cmd = readline("# ");
        if (!cmd) continue;
        process(cmd);
        //update jobs
    }
}