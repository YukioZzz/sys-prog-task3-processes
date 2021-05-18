#include "parse.h"
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <errno.h>
char background;
/**
 * used for exec a single cmd with redirection
 */
int CmdWithRedir(struct command* cmd){
    if(cmd->input_redir!=NULL)
        freopen(cmd->input_redir, "r", stdin);
    if(cmd->output_redir!=NULL)
        freopen(cmd->output_redir, "w", stdout);
    execvp(cmd->argv[0],cmd->argv);
    exit(errno);
}
/** 
 * This is an iterative implementation of pipeline; 
 */
void CmdWithPipe(struct command* cmd){
    int result = 0, status, fds[2];
    pipe(fds);//pipe2(fds,O_NONBLOCK); 
    pid_t pid = fork();
    
    switch(pid){
        case -1: perror("fork"); exit(1);
        case 0:
            ///< Child PID: dup pipout 2 stdout
            close(fds[0]);
            if(cmd->next != NULL)dup2(fds[1],STDOUT_FILENO);///< if not last cmd, then replace stdout
            close(fds[1]);
            result = CmdWithRedir(cmd);
            exit(result); ///< maybe do not need here
        default:
            ///< Parent PID: iteratively exec next cmd
            if(cmd->next == NULL&&background==0)waitpid(pid, &status, 0);///< if last and bg, waitpid
            close(fds[1]);
            if(cmd->next != NULL)dup2(fds[0], STDIN_FILENO);///< if not last cmd, then replace stdin
            close(fds[0]);
            if(cmd->next != NULL)CmdWithPipe(cmd->next); 
    }
}

/* TODO: implement this */
void run_pipeline(struct pipeline *p) {
    background = p->background;
    CmdWithPipe(&p->first_command);
}

/* TODO: implement this */
void run_builtin(enum builtin_type builtin, char *builtin_arg) {
    int arg;
    if(builtin_arg != NULL) arg = atoi(builtin_arg);
    switch(builtin){
        case BUILTIN_EXIT:
            if(builtin_arg == NULL)exit(0);
            else {exit(arg);}
            break;
        case BUILTIN_WAIT:
            if(builtin_arg == NULL)waitpid(-1,0,0);
            else waitpid(arg,0,0);
            break;
        case BUILTIN_KILL:
            if(builtin_arg == NULL)printf("Please specify the target PID!\n");
            else kill(arg,SIGTERM);
            break;
        default:
            printf("Please type in cmd exit/wait/kill.\n");
    }
}
