#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <pwd.h>

// Max Number of Spaces in runCommand
#define MAX_CMD_LEN 1000
// Max File Size of 
#define MAX_OUTPUT_SIZE 100000



char *runPopen(const char* command)
{
    // Duplicate the command
    char* commandDup = strdup(command);
    char* args[MAX_CMD_LEN];
    char* token;
    int i = 0;

    // Tokenize the input command based on space
    token = strtok((char*)commandDup, " ");
    while (token != NULL)
    {
        args[i++] = token;
        token = strtok(NULL, " ");
    }
    args[i] = NULL; // Set the last argument to NULL

    // Execute the command using execvp
    int p[2];
    if (pipe(p) < 0)
    {
        exit(0);
    }
    int child = fork();
    if (child > 0) {

        // parent
        waitpid(child, NULL, 0);
        char *result = (char *)malloc(MAX_OUTPUT_SIZE);

        // char inbuf[400]; 

        read(p[0],result, MAX_OUTPUT_SIZE);
        printf("%s", result);
        return result;
    }
    else {
        // child
        dup2(p[1], 1);
        if (execvp(args[0], args) == -1)
        {
            printf("Command '%s' not found\n", commandDup);
            return 0;
        }
    }
}

int main() {
runCommand("ls -1");
}