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

typedef struct process{
    int gpid;
    int state; // 0/running 1/stopped 2/done
    int job_num;
    char * text;
    struct process * next_job;
    struct process * prev_job;
    int bg; // 0/no 1/yes
};

struct process * head = NULL;
struct process * top = NULL;

char * get_input(){
    char * cmd = readline("# ");
    if(!cmd){
        //exit
    }
    return cmd; 
}

void add_process(char * args, int pid1, int pid2, int background){
    struct process * proc = malloc(sizeof(struct process));
    proc->text = args;
    proc->state = 0;
    setpgid(pid1, 0);
    if(pid2 == -1){
        setpgid(pid2, pid1);
    }
    proc->gpid = pid1;
    //if nothing on stack
    if(head == NULL){
        head = proc;
        top = proc;
        head->prev_job = NULL;
        proc->job_num = 1;
    }
    else{
        proc->prev_job = top;
        top->next_job = proc;
        proc->job_num = top->job_num + 1;
        top = proc;
    }
    proc->bg = background;
    
}


void send_to_back(){
    printf("rosa parks");
}

void bring_to_front(){
    printf("front");
}

void print_jobs(){
    if(top == NULL || head == NULL) return;
    const char * RUN_M = "Running";
    const char * STOP_M = "Stopped";
    const char * DONE_M = "Done";
    const char * CUR_M = "+";
    const char * BAK_M = "-";
    const char * GEN_F = "[%d]%s %s     %s\n";
    const char * DONE_F = "[%d]%s %s        %s\n";
    struct process * cur = head;
    while(cur != NULL){
        if(cur->state == 2){
            if(cur->next_job ==NULL){
                printf(DONE_F,cur->job_num,CUR_M,DONE_M,cur->text);
            }else{
                printf(DONE_F,cur->job_num,BAK_M,DONE_M,cur->text);
            }
        }else if(cur->state == 1){
            if(cur->next_job ==NULL){
                printf(GEN_F,cur->job_num,CUR_M,STOP_M,cur->text);
            }else{
                printf(GEN_F,cur->job_num,BAK_M,STOP_M,cur->text);
            }
        }else{
            if(cur->next_job ==NULL){
                printf(GEN_F,cur->job_num,CUR_M,RUN_M,cur->text);
            }else{
                printf(GEN_F,cur->job_num,BAK_M,RUN_M,cur->text);
            }
        }
        cur = cur->next_job;
    }
}

void kill_proc(int pid){
    printf("killing");
}
void kill_all_processes(){
    printf("killing all");
}

void set_operators(char*args[], int arg_count){

    int f_in=-1,f_out=-1,f_err=-1;

    for (int i = 0; i < arg_count - 1; i++) {

        if (strcmp(args[i], REDIR_OUT) == 0) { 
            if (i >= arg_count- 1){
                 continue;
            }
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
            if (i >= arg_count- 1){
                continue;
            }
            
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
            if (i >= arg_count - 1){
                continue;
            }
            args[i] = NULL;
            
            char *file = args[i+1];
            f_err = open(file, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
            
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

void execute_cmd(char * cmd[], int arg_count, char * args, int bg){
    int pid = fork();
    if (pid < 0){
        perror("fork failed");
        // exit(1);
    }else if(pid == 0){
        set_operators(cmd, arg_count);
        execvp(cmd[0], cmd);
    }
    else{
        add_process(args, pid, -1, bg);
        if(!bg){
            wait(NULL);
        }
    }
}

void execute_pipe(char*cmd1[], int arg_count1, char*cmd2[], 
    int arg_count2,char * args, int bg){
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
        _exit(1);
    }
    else{

        add_process(args, cpid1, cpid2, bg);
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
    int background = 0;

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

    //check if last character is & in order to send to background
    if(args[arg_count-2] != NULL && 
        strcmp(args[arg_count-2],BACKGROUND) == 0){
        background = 1;
        args[arg_count - 2] = NULL;
        }

    if(strcmp(args[0], FG) == 0) {
        bring_to_front();
        return;
    }
    else if(strcmp(args[0], BG) == 0){
        send_to_back();
        return;
    } 
    else if(strcmp(args[0], JOBS) == 0){
        print_jobs();
        return;
    }
    else if(strcmp(args[0], EXIT) == 0){
        kill_all_processes();
        _exit(0);
    }

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
            arg_num - pipe_address - 1, cmd, background);
    }
    else{
        execute_cmd(args, arg_count, cmd, background);     
    } 
}

int main(){
    //signal(SIGTTOU, SIG_IGN);
    //signal(SIGINT, SIG_IGN);
    //signal(SIGTSTP, SIG_IGN);
    tcsetpgrp(0,getpid());

    while(1) {
        char * cmd = get_input();
        if (!cmd) continue;
        process(cmd);
    }
}