#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char const *argv[])
{
    int p[2]; 
    pipe(p);

    int pid = fork(); 

    if (pid == 0)  // Child process.
    {
        close(p[0]); // close the file descriptor, we do not use it in the child process.
        char * message = "Hello World!\n";
        write(p[1], message, sizeof(char) * strlen(message));

        close(p[1]); // close the file descriptor, we done use it.
    }
    else if (pid > 0) // Father process.
    {
        close(p[1]); // close the file descriptor, we do not use it in the father process.
        char c;
        waitpid(pid, 0, 0);  // wait for the child process to end.
        while (read(p[0], &c, 1) != 0 )  // while we did not finish read.
        {
            write(1, &c, 1);
            // printf("%c", c);
        }
        close(p[0]); // close the file descriptor, we done use it.
    }
    else{  // Error.
        return 1;
    }
    return 0;
}