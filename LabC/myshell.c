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

typedef struct process{
    cmdLine* cmd; /* the parsed command line*/
    pid_t pid; /* the process id that is running the command*/
    int status; /* status of the process: RUNNING/SUSPENDED/TERMINATED */
    struct process *next; /* next process in chain */
 } process;

#define TERMINATED -1
#define RUNNING 1
#define SUSPENDED 0
#define HISTLEN 20

process* p = NULL;

int executeCmd(cmdLine *pCmdLine);
void processSuspend(process** processList, int processId);
void processWake(process** process_list, int processId);
void processKill(process** process_list, int processId);
int executePipeCommand(cmdLine *pCmdLine);
void addProcess(process** process_list, cmdLine* cmd, pid_t pid);
void printProcessList(process** process_list);
void printProcess(process* process_list, int index);
void freeProcessList(process* process_list);
void freeProcess(process* process);
void updateProcessList(process **process_list);
void updateProcessStatus(process* process_list, int pid, int status);
void deleteTerminatedProcess(process** process_list);
void lookForCmdLineAndDelete(process** process_list, cmdLine* cmd);
void redirectIO(cmdLine* cmd);
void clearAllMemoryAndKillAllProcess(process** process_list);
void KillAllProcess(process** process_list);
void execute(cmdLine* userCmdLine, process** process_list, bool minusD);
void enqueue(char* toEnqueue, char** arguments, int* newest, int* oldest, int* size);
char* peek(char** arguments, int* newest, int* oldest, int* size);
char* peekAt(int index, char** arguments, int* newest, int* oldest, int* size);
void printHistory(char** arguments, int* newest, int* oldest, int* size);
void clearQueue(char** arguments, int* newest, int* oldest, int* size);


int main(int argc, char const *argv[])
{   
    char* arguments[HISTLEN];
    int newest = -1;
    int oldest = -1;
    int size = 0;
    
    bool minusD = false;
    //look for the dubug flag, "-d".
    for (int i = 1; i < argc; i++)
    {
        if(strcmp(argv[i], "-d") == 0){
            minusD = true;
            break;
        }
    }
    
    char c[PATH_MAX];  // string at length PATH_MAX.
    process** process_list = &p;
    //Infinite loop
    while (1)
    {
        // print current working directory.
        if(!getcwd(c,PATH_MAX)){   // if we recived null(Error).
            printf("Error!\n");
            break;
        }
        printf("%s: ", c);  // print the current working directory.

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
        if(strcmp(userLine, "history\n") == 0){
            char* userLineCopy = strdup(userLine);
            enqueue(userLineCopy, arguments, &newest, &oldest, &size);

            printHistory(arguments, &newest, &oldest, &size);
        }
        else if(strcmp(userLine, "!!\n") == 0){
            char* command = peek(arguments, &newest, &oldest, &size);
            if(strcmp(command, "") != 0){
                if(strcmp(command, "history\n") == 0){
                    printHistory(arguments, &newest, &oldest, &size);
                }
                else{
                    cmdLine * userCmdLine = parseCmdLines(command);
                    execute(userCmdLine, process_list, minusD);
                }
            }
        }
        else if(strncmp(userLine, "!", 1) == 0){
            int index = atoi(userLine + 1);
            char* command = peekAt(index, arguments, &newest, &oldest, &size);
            if(strcmp(command, "") != 0){
                if(strcmp(command, "history\n") == 0){
                    printHistory(arguments, &newest, &oldest, &size);
                }
                else{
                    cmdLine * userCmdLine = parseCmdLines(command);
                    execute(userCmdLine, process_list, minusD);
                }
            }
            else{
                printf("Error\n");
            }
        }
        else{
            char* userLineCopy = strdup(userLine);
            enqueue(userLineCopy, arguments, &newest, &oldest, &size);
            cmdLine * userCmdLine = parseCmdLines(userLine);
            execute(userCmdLine, process_list, minusD);
        }


    }
    clearQueue(arguments, &newest, &oldest, &size);
    clearAllMemoryAndKillAllProcess(process_list);// when exit, clean all memory and kill al process.
}

void execute(cmdLine* userCmdLine, process** process_list, bool minusD){
        int pid = fork(); // duplicate the process.

        if(pid > 0){  /// This is the main process/Father.

            if(minusD){ // dubug mode is on. print to stderr.
                fprintf(stderr, "PID: %d\nExecuting command: %s\n", pid, userCmdLine->arguments[0]);
            }
            if(userCmdLine->blocking){
                waitpid(pid, 0, 0);  // wait for the child process to end.
            }
            else{
                usleep(10000);  // wait a moment for clarity.
            }
            if((strcmp(userCmdLine->arguments[0], "cd") == 0)){
                int i = chdir(userCmdLine->arguments[1]);
                if(i == -1){
                    perror("Error");
                }
            }
            // print process command.
            if((strcmp(userCmdLine->arguments[0], "procs") == 0)){
                printProcessList(process_list);
            }
            // suspend a process command.
            if(strcmp(userCmdLine->arguments[0], "suspend") == 0){
                processSuspend(process_list, atoi(userCmdLine->arguments[1]));
            }
            // wake a process command.
            if(strcmp(userCmdLine->arguments[0], "wake") == 0){
                processWake(process_list, atoi(userCmdLine->arguments[1]));
            }
            // kill a process command.
            if(strcmp(userCmdLine->arguments[0], "kill") == 0){
                processKill(process_list, atoi(userCmdLine->arguments[1]));
            }
            // if this is a command we run at the child add to the process list.
            if(! ((strcmp(userCmdLine->arguments[0], "cd") == 0) || (strcmp(userCmdLine->arguments[0], "procs") == 0) || (strcmp(userCmdLine->arguments[0], "suspend") == 0) || (strcmp(userCmdLine->arguments[0], "wake") == 0) || (strcmp(userCmdLine->arguments[0], "kill") == 0))){
                addProcess(process_list, userCmdLine, pid);
            }
            else{
                freeCmdLines(userCmdLine);
            }

        }
        else if(pid == 0){  // this is the child process.

            redirectIO(userCmdLine);  // redirect input/output.

            if (executeCmd(userCmdLine) == -1) // RUN the function and check if was any errors.
            {
                freeProcessList(*process_list); // when exit, clean all memory.
                freeCmdLines(userCmdLine);
                perror("Error");
                exit(1);
            }
            freeProcessList(*process_list); // when exit, clean all memory.
            freeCmdLines(userCmdLine);
            exit(0);  // Child process finish.
        }
}

/**
 * This function execute the command recived to the shell.
*/
int executeCmd(cmdLine *pCmdLine){

    // if we allready execute the comman at the father process.
    if((strcmp(pCmdLine->arguments[0], "suspend") == 0) || (strcmp(pCmdLine->arguments[0], "wake") == 0) || (strcmp(pCmdLine->arguments[0], "kill") == 0) || (strcmp(pCmdLine->arguments[0], "cd") == 0) || (strcmp(pCmdLine->arguments[0], "procs") == 0)){
        return 1;
    }
    if(pCmdLine->next != NULL){  // There is more then one command to execute.
        return executePipeCommand(pCmdLine);
    }
    // regular command.
    return execvp(pCmdLine->arguments[0], pCmdLine->arguments);
}
/**
 * This function suspend a given process, and update the process List.
*/
void processSuspend(process** process_list, int processId){
    updateProcessStatus(*process_list, processId, SUSPENDED);
    int i = kill(processId, SIGTSTP);
    if(i == -1){
        perror("Error");
    }
}
/**
 * This function wake a given process, and update the process List.
*/
void processWake(process** process_list, int processId){
    updateProcessStatus(*process_list, processId, RUNNING);
    int i = kill(processId, SIGCONT);
    if(i == -1){
        perror("Error");
    }
}
/**
 * This function kill a given process.
*/
void processKill(process** process_list, int processId){
    updateProcessStatus(*process_list, processId, TERMINATED);
    int i = kill(processId, SIGINT);
    if(i == -1){
        perror("Error");
    }
}
/**
 * This function xecute the pipe-command recived to the shell.
*/
int executePipeCommand(cmdLine *pCmdLine){
    if (pCmdLine->outputRedirect != NULL || pCmdLine->next->inputRedirect != NULL){ // Error! ilegal redirecting.
        errno = EINVAL;
        return -1;
    }

    int pipefd[2];
    pipe(pipefd);   // create a pipe.

    int pid = fork();  //first fork.

    if(pid == -1){ // Error.
        return -1;
    }
    else if(pid == 0){   // The Child1 process.

        dup2(pipefd[1], STDOUT_FILENO);  // close stdout, and redirect it to pipefd[1].

        close(pipefd[1]);  //close the file descriptor that was duplicate.

        return execvp(pCmdLine->arguments[0], pCmdLine->arguments);  // execute the first command.

    }
    else{  // The Father frocess.

        close(pipefd[1]); // Close the write end of the pipe

        int pid2 = fork(); //fork number two.

        if(pid2 == -1){  // Error.
            return -1;
        }
        else if(pid2 == 0){ // The Child2 process.
            redirectIO(pCmdLine->next);  // fix the second command file redirection.

            dup2(pipefd[0], STDIN_FILENO);  // close stdin, and redriect it to pipefd[0]

            close(pipefd[0]);  //close the file descriptor that was duplicate.

            return execvp(pCmdLine->next->arguments[0], pCmdLine->next->arguments);  // execute the second command.
        }
        else{  // The Father frocess.
            close(pipefd[0]);  // close the read end of the pipe.
            waitpid(pid, 0, 0);  // wait for both child process to end.
            waitpid(pid2, 0, 0);
            return 1;
        }
    }
}

/**
 * This function add a process to our process list or creat new list if our list is empty.
*/
void addProcess(process** process_list, cmdLine* cmd, pid_t pid){
    process* p = (process*)(malloc(sizeof(process)));
    p->cmd = cmd;
    p->pid = pid;
    p->status = RUNNING;
    p->next = NULL;
    if(*process_list == NULL){  // The list in empty.
        *process_list = p;
    }
    else{   // The list is not empty, add first.
        p->next = (*process_list);
        *process_list = p;
    }
}

/**
 * This function print the process list according to the given format.
*/
void printProcessList(process** process_list){
    updateProcessList(process_list);

    if(*process_list != NULL){
        printProcess((*process_list), 1);
    }

    deleteTerminatedProcess(process_list);
}

void printProcess(process* process_list, int index){
    if(process_list != NULL){
        printf("%d ", index);

        printf("%d ", process_list->pid);

        char* s;
        if(process_list->status == RUNNING){
            s = "Running";
        }
        else if(process_list->status == TERMINATED){
            s = "Terminated";
        }
        else{
            s = "Suspended";
        }
        printf("%s ", s);

        for(size_t i = 0; i < process_list->cmd->argCount; i++)
        {
            printf("%s ", process_list->cmd->arguments[i]);
        }
        if(process_list->cmd->next != NULL){
            printf("| ");
            for(size_t i = 0; i < process_list->cmd->next->argCount; i++)
            {
                printf("%s ", process_list->cmd->next->arguments[i]);
            }
        }
        printf("\n");

        printProcess(process_list->next , index + 1);
    }

}
/**
 * free all memory allocated for the process list.
*/
void freeProcessList(process* process_list){
    if(process_list){
        freeCmdLines(process_list->cmd);
        freeProcessList(process_list->next);
        free(process_list);
    }
}
/**
 * free memory of single process.
*/
void freeProcess(process* process){
    if(process){
    freeCmdLines(process->cmd);
    free(process);
    }
}
/**
 * go over the process list, and for each process check if it is done, and update the list.
 *
 **/
void updateProcessList(process **process_list){

    process* currentProcess = *process_list;
    while(currentProcess)
    {
        int status;
        int terminatedPid = waitpid(currentProcess->pid, &status, WNOHANG | WCONTINUED | WUNTRACED);
        if(terminatedPid == -1)  // The process is done.
        {
            currentProcess->status = TERMINATED;
        }
        else if(terminatedPid != 0){
            if(WIFEXITED(status) || WIFSIGNALED(status)){  // The child process ends normaly or by signal.
                currentProcess->status = TERMINATED;
            }
            else if(WIFSTOPPED(status)){   // The child was stoped by a signal.
                currentProcess->status = SUSPENDED;
            }
            else if(WIFCONTINUED(status) && currentProcess->status == SUSPENDED){  // The child has ben wake by signal.
                currentProcess->status = RUNNING;
            }

        }

        currentProcess = currentProcess->next;
    }
}


/**
 * find the process with the given id in the process_list and change its status to the received status.
*/
void updateProcessStatus(process* process_list, int pid, int status){

    process* currentProcess = process_list;
    while(currentProcess != NULL)
    {
        if(currentProcess->pid == pid)
        {
            currentProcess->status = status;
        }

        currentProcess = currentProcess->next;
    }
    
}
/**
 * Look for terminated process, and delete them from the list.
*/
void deleteTerminatedProcess(process** process_list){
    process* currentProcess = *process_list;
    process* prevProcess = NULL;

    while(currentProcess)
    {
        if(currentProcess->status == TERMINATED)
        {
            if(prevProcess == NULL)   // PREV = NULL, we are at the start of the list.
            {
                (*process_list) = currentProcess->next;
                freeProcess(currentProcess);
                currentProcess = *process_list;
            }
            else{
                prevProcess->next = currentProcess->next;
                freeProcess(currentProcess);
                currentProcess = prevProcess->next;
            }

        }
        else{
            prevProcess = currentProcess;
            currentProcess = currentProcess->next;
        }


    }

}
/**
 * This funciton meant to take care of typo, and command we dont need to save as process.
*/
void lookForCmdLineAndDelete(process** process_list, cmdLine* cmd){
    process* currentProcess = *process_list;
    process* prevProcess = NULL;


    while(currentProcess)
    {
        if(currentProcess->cmd == cmd)
        {

            if(!prevProcess)   // PREV = NULL, we are at the start of the list.
            {
                (*process_list) = currentProcess->next;
                freeProcess(currentProcess);
                currentProcess = *process_list;
            }
            else{
                prevProcess->next = currentProcess->next;
                freeProcess(currentProcess);
                currentProcess = prevProcess->next;
            }
            break;

        }
        else{
            prevProcess = currentProcess;
            currentProcess = currentProcess->next;
        }

    }
}
/**
 * This function redirect the stdin/stdout if needed.
*/
void redirectIO(cmdLine* cmd){
    if(cmd->inputRedirect != NULL){  // check and redirect the input if needed.
        int file = open(cmd->inputRedirect, O_RDONLY);
        dup2(file, STDIN_FILENO);
    }
    if(cmd->outputRedirect != NULL){ // check and redirect the output if needed.
        int file = open(cmd->outputRedirect, O_WRONLY | O_CREAT);
        dup2(file, STDOUT_FILENO);
    }
}
/**
 * This function kill all non TERMINATED process and free all memory.
*/
void clearAllMemoryAndKillAllProcess(process** process_list){
    KillAllProcess(process_list);
    freeProcessList(*process_list); 
}
/**
 * This function kill all non TERMINATED process.
*/
void KillAllProcess(process** process_list){
    updateProcessList(process_list);

    process* currentProcess = *process_list;
    while(currentProcess)
    {
        if(currentProcess->status != TERMINATED)
        {
            processKill(process_list, currentProcess->pid);
        }
        currentProcess = currentProcess->next;
    }
}

/**
 * add an element to our queue.
*/
void enqueue(char* toEnqueue, char** arguments, int* newest, int* oldest, int* size){
    if(*size == HISTLEN){
        char* toFree = arguments[*oldest];
        arguments[*oldest] = toEnqueue;
        (*newest) = (*oldest);
        (*oldest) = ((*oldest) + 1) % 20;
        free(toFree);
    }
    else if(*size == 0){
        arguments[0] = toEnqueue;
        (*newest) = 0;
        (*oldest) = 0;
        (*size) = 1;
    }
    else{
        (*newest) = ((*newest) + 1) % 20;
        arguments[(*newest)] = toEnqueue;
        (*size) = (*size) + 1;
    }
}

/**
 * return the newest element of the queue.
*/
char* peek(char** arguments, int* newest, int* oldest, int* size){
    if((*size) == 0){
        return "";
    }
    else{
        return arguments[*newest];
    }
}

/**
 * return the element at location index.
*/
char* peekAt(int index, char** arguments, int* newest, int* oldest, int* size){
    if(index < 1 || index > 20 || index > (*size)){
        return "";
    }
    int currectIndex = (*newest + 1 - index);
    if(currectIndex >= 0){
        return arguments[currectIndex];
    }
    else{
        currectIndex += 20;
        return arguments[currectIndex];
    }

}

/**
 * print the history queue .
*/
void printHistory(char** arguments, int* newest, int* oldest, int* size){
    int index = *newest;
    int historySize = *size;
    while(historySize > 0){
        printf("%s\n", arguments[index]);
        historySize = historySize - 1;
        index = (index - 1) % 20;

        if(index == -1){
            index = 19;
        }
    }
}

/**
 * clear the queue.
*/
void clearQueue(char** arguments, int* newest, int* oldest, int* size){
    int index = *newest;
    int historySize = *size;
    while(historySize > 0){
        free(arguments[index]);
        historySize = historySize - 1;
        index = (index - 1) % 20;

        if(index == -1){
            index = 19;
        }
    }
}
