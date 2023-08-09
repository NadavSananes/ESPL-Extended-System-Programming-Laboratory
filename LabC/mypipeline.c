#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char const *argv[])
{
    int pipefd[2];
    pipe(pipefd);   // 1.create a pipe.

    fprintf(stderr, "(parent_process>forking…)\n");  // dubug- before forking.

    int pid = fork();  // 2. first fork.

    if(pid == -1){ // Error.
        perror("Error");
        exit(0);
    }
    else if(pid == 0){   // The Child1 process.

        fprintf(stderr, "(child1>redirecting stdout to the write end of the pipe…)\n");  // debug.

        // close(p[0]); // close the file descriptor, we do not use it in the child process.

        close(STDOUT_FILENO);  // 3.1 close stdout.

        int newFd = dup(pipefd[1]); // 3.2 duplicate the write end of the pipe.

        close(pipefd[1]);  // 3.3 close the file descriptor that was duplicate.

        char *s[] = { "ls", "-l", 0 };

        fprintf(stderr, "(child1>going to execute cmd: ls -l)\n");  // debug.

        execvp(s[0], s);  // 3.4 Execute "ls -l".

        close(newFd);  // close the file, done using it.
    }
    else{  // The Father frocess.
        fprintf(stderr, "(parent_process>created process with id: %d)\n", pid);  // dubug- father process after forking.

        fprintf(stderr, "(parent_process>closing the write end of the pipe…)\n");  // debug.

        close(pipefd[1]); // 4. Close the write end of the pipe

        int pid2 = fork(); // 5. fork number two.

        if(pid2 == -1){  // Error.
            perror("Error");
            exit(0);
        }
        else if(pid2 == 0){ // The Child2 process.

            fprintf(stderr, "(child2>redirecting stdin to the read end of the pipe…)\n");  // debug.

            close(STDIN_FILENO);   // 6.1 close stdin.

            int newFd2 = dup(pipefd[0]); // 6.2 duplicate the read end of the pipe.

            close(pipefd[0]);  // 6.3 close the file descriptor that was duplicate.

            char *s[] = {"tail", "-n 2", 0};
            fprintf(stderr, "(child2>going to execute cmd: tail -n 2)\n");  // debug.

            execvp(s[0], s);  // 6.4 Execute "tail -n 2".

            close(newFd2);  // close the file, done using it.
        }
        else{  // The Father frocess.
            fprintf(stderr, "(parent_process>closing the read end of the pipe…)\n");  // debug.

            close(pipefd[0]);  // 7. close the read end of the pipe.

            fprintf(stderr, "(parent_process>waiting for child processes to terminate…)\n");  // debug.

            waitpid(pid, 0, 0);  // 8. wait for both child process to end.
            waitpid(pid2, 0, 0);
            fprintf(stderr, "(parent_process>exiting…)\n");  // debug.

        }

    }

}
