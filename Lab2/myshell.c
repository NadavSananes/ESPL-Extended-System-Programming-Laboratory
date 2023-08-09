#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/limits.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "LineParser.c"
#include <fcntl.h>
#include <sys/syscall.h>
#include <signal.h>
#include <time.h>

int execute(cmdLine *pCmdLine);
int processSuspend(int processId);
int processWake(int processId);
int processKill(int processId);

int main(int argc, char const *argv[])
{   
    bool minusD = false;
    //find -d
    for (int i = 1; i < argc; i++)
    {
        if(strcmp(argv[i], "-d") == 0){
            minusD = true;
            break;
        }
    }
    
    char c[PATH_MAX];  // string at length PATH_MAX.

    //Infinite loop
    while (1)
    {
        // print current working directory.
        if(!getcwd(c,PATH_MAX)){   // if we recived null(Error).
            printf("Error!\n");
            break;
        }
        printf("%s: ", c);

        // read a line from the user.
        char userLine[2048];
        if(fgets(userLine, 2048, stdin) == NULL){  // RETURN THE STRING ENDS WITH \N.
            printf("Error!\n");
            break;
        }
        if(strcmp(userLine, "quit\n") == 0)
        {
            printf("quiting..\n");
            break;
        }
        
        cmdLine * userCmdLine = parseCmdLines(userLine);

        int pid = fork(); // duplicate the process.


        if(pid > 0){  /// This is the main process/Father.
            if(minusD){ // dubug mode is on. print to stderr.
                fprintf(stderr, "PID: %d\nExecuting command: %s\n", pid, userCmdLine->arguments[0]);
            }
            if(userCmdLine->blocking){
                waitpid(pid, 0, 0);  // wait for the child process to end.
            }
            else{
                usleep(10000);  // wait a moument for clarity.
            }
            if((strcmp(userCmdLine->arguments[0], "cd") == 0)){
                int i = chdir(userCmdLine->arguments[1]);
                if(i == -1){
                    perror("Error");
                    freeCmdLines(userCmdLine);
                    _exit(0);
                }
            }
        }
        else if(pid == 0){  // this is the child process.

            if(userCmdLine->inputRedirect != NULL){  // check and redirect the input if needed.
                int file = open(userCmdLine->inputRedirect, O_RDONLY);
                dup2(file, STDIN_FILENO);
            }
            if(userCmdLine->outputRedirect != NULL){ // check and redirect the output if needed.
                int file = open(userCmdLine->outputRedirect, O_WRONLY | O_CREAT);
                dup2(file, STDOUT_FILENO);
            }

            if (execute(userCmdLine) == -1) // RUN the function and check if was any errors.
            {
                perror("Error");
                freeCmdLines(userCmdLine);
                _exit(0);
            }
            freeCmdLines(userCmdLine);
            break;
        }
        freeCmdLines(userCmdLine);

    }

}
int execute(cmdLine *pCmdLine){
    if(strcmp(pCmdLine->arguments[0], "suspend") == 0){
        return processSuspend(atoi(pCmdLine->arguments[1]));
    }
    if(strcmp(pCmdLine->arguments[0], "wake") == 0){
        return processWake(atoi(pCmdLine->arguments[1]));
    }
    if(strcmp(pCmdLine->arguments[0], "kill") == 0){
        return processKill(atoi(pCmdLine->arguments[1]));
    }
    if((strcmp(pCmdLine->arguments[0], "cd") == 0)){
        return 1;
    }
    return execvp(pCmdLine->arguments[0], pCmdLine->arguments);
}

int processSuspend(int processId){
    return kill(processId, SIGTSTP);
}
int processWake(int processId){
    return kill(processId, SIGCONT);
}
int processKill(int processId){
    return kill(processId, SIGINT);
}
