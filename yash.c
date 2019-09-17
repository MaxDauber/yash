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

#define MAX_LEN 1028
#define MAX_ARGS 100
#define PIPE "|"
#define BACKGROUND "&"
#define REDIR_IN "<"
#define REDIR_OUT ">"
#define REDIR_ERR "2>"
#define JOBS "jobs"
#define BG "bg"
#define FG "fg"
#define EXIT "exit"
#define RUNNING "Running"
#define STOPPED "Stopped"
#define DONE "Done"

int stdin_c=0,stdout_c=1,stderr_c=2;

typedef struct process{
    int gpid;
    int state;
    char* args;
    struct job * next_job;
    struct job * prev_job;
};

typedef struct process_runner{
    struct process process_list[20];
    int num_proc;
};

char * get_input(){
    char * cmd = readline("# ");
    if(!cmd){
        //killprocess
        //_exit(0)
    }
    if(cmd[strlen(cmd)-1] == '\n') cmd[strlen(cmd)-1] == '\0';
    return cmd; 
}

void send_to_back(){

}

void bring_to_front(){

}

void print_jobs(){

}

void kill_proc(){

}

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

void execute_pipe(char*cmd1[], int arg_count1, char*cmd2[], 
    int arg_count2,int bg){
    int pfd[2];
    int cpid1, cpid2;
    pipe(pfd);
    cpid1 = fork();
    if (cpid1 == 0){
        dup2(pfd[1],1);
        close(pfd[0]);
        set_operators(&cmd1[0], arg_count1);
        execvp(cmd1[0], cmd1);
    }
    cpid2 = fork();
    if (cpid2 == 0){
        dup2(pfd[0],0);
        close(pfd[1]);
        set_operators(&cmd2[0], arg_count2);
        execvp(cmd2[0], cmd2);
    }
    if (cpid1 < 0 || cpid2 < 0){
        perror("fork failed");
        // exit(1);
    }
    else{
        if(!bg){
            wait(NULL);
        }
    }
}

void process(char* cmd){
    if (cmd == NULL) return;
    char * args [MAX_ARGS];
    int arg_count = 0;
    int pipe_address = -1;
    args[arg_count] = strtok(cmd," ");

    while(args[arg_count] != NULL){
        
        //append next token to command array
        args[++arg_count] = strtok(NULL," ");

        //check if command is a pipe
        if(args[arg_count] != NULL && 
            strcmp(args[arg_count], PIPE) == 0){
            pipe_address = arg_count;
        }
    }

    args[++arg_count] = NULL;

    if(pipe_address > 0){
        int arg_num = arg_count - 1;

        // populate cmd1 with first command before pipe
        char * cmd1 [pipe_address+1];
        for(int i = 0; i < pipe_address; i++){
            cmd1[i] = args[i];
        }
        cmd1[pipe_address] = NULL;

        // populate cmd2 with command after pipe
        char * cmd2 [arg_num - pipe_address];
        for(int i = 0; i < arg_num - pipe_address - 1; i++){
            cmd2[i] = args[pipe_address + 1 + i];
        }
        cmd2[arg_num - pipe_address - 1] = NULL;
        

        //execute pipe command
        execute_pipe(cmd1, pipe_address, cmd2, 
            arg_num - pipe_address - 1, 0);
    }else if(strcmp(args[0], FG) == 0) bring_to_front();
    else if(strcmp(args[0], BG) == 0) send_to_back();
    else if(strcmp(args[0], JOBS) == 0) print_jobs();
    else if(strcmp(args[0], EXIT) == 0){
        kill_proc();
        _exit(0);
    }
    else{
        //if no pipe execute regular command
        execute_cmd(args, arg_count, 0);
    } 
}

int main(){
    signal(SIGTTOU, SIG_IGN);
    signal(SIGINT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);

    tcsetpgrp(0,getpid());

    while(1) {
        char* cmd = get_input();
        if (!cmd) continue;
        process(cmd);
        //update jobs
    }
}