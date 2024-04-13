#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#define PORT 8080
#define MAX_CMD_LEN 1000
#define MAX_OUTPUT_SIZE 1000000
#define MAX_NO_ARGUMENTS 400

char* runPopenWithArray(char* s) {
    char* args[] = {"bash", "-c", s, NULL};
    
    // Create a pipe
    int p[2];
    if (pipe(p) < 0) {
        perror("Pipe creation failed");
        exit(EXIT_FAILURE);
    }
    
    // Fork a child process
    pid_t pid = fork();
    if (pid < 0) {
        perror("Fork failed");
        exit(EXIT_FAILURE);
    } else if (pid == 0) { // Child process
        close(p[0]); // Close reading end of pipe
        dup2(p[1], STDOUT_FILENO); // Redirect stdout to the pipe write end
        close(p[1]); // Close the write end of the pipe in the child
        if (execvp(args[0], args) == -1) {
            perror("Execution failed");
            exit(EXIT_FAILURE);
        }
    } else { // Parent process
        close(p[1]); // Close the write end of the pipe in the parent
        char* result = (char*)malloc(MAX_OUTPUT_SIZE);
        ssize_t total_read = 0;
        ssize_t bytes_read;
        while ((bytes_read = read(p[0], result + total_read, MAX_OUTPUT_SIZE - total_read)) > 0) {
            total_read += bytes_read;
        }
        if (bytes_read < 0) {
            return "\0";
        }
        close(p[0]); // Close the read end of the pipe in the parent
        // Ensure null-termination
        result[total_read] = '\0';
        return result;
    }
}

int main() {
    char* output = runPopenWithArray("find ~/ -type d -not -path '*/.*' -exec stat --format='%W-{}' {} \\; | sort -t '-' -k 1 -r | awk -F '-' '{print $2}'");
    printf("%s", output);
    free(output); // Free the allocated memory
    return 0;
}
