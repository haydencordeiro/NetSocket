#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#define PORT 8080
// Max Number of Spaces in runCommand
#define MAX_CMD_LEN 1000
// Max File Size of 
#define MAX_OUTPUT_SIZE 100000
// Used in split string function
#define MAX_NO_ARGUMENTS 400



// Helper method to remove spaces from a string
char* stripSpaces(char* word)
{
    // Skip leading spaces
    while (isspace(*word))
        word++;
    if (*word == '\0') // If the string is empty or contains only spaces
        return word;
    // Trim trailing spaces
    char* end = word + strlen(word) - 1;
    while (end > word && isspace(*end))
        end--;
    // Null-terminate the trimmed string
    *(end + 1) = '\0';
    return word;
}


void remove_special_chars(char* str)
{
    int i, j = 0;
    for (i = 0; str[i] != '\0'; i++)
    {
        // If the character is alphanumeric, keep it
        if (isalnum((unsigned char)str[i]))
        {
            str[j++] = str[i];
        }
    }
    str[j] = '\0'; // Null terminate the resulting string
}

// Helper method to split a given string by the specified delimeter
// eg: ("ls -1", " ") -> ["ls","-1"]
char** splitString(char* str, char* delimenter)
{
    int count = 0;
    char** tokens = (char**)malloc(MAX_NO_ARGUMENTS * sizeof(char*));
    if (tokens == NULL)
    {
        printf("Memory allocation error\n");
        return NULL;
    }

    // tokenize the string
    char* token = strtok(str, delimenter);
    while (token != NULL)
    {
        // allocate memory
        tokens[count] = (char*)malloc((strlen(token) + 1) * sizeof(char));
        if (tokens[count] == NULL)
        {
            fprintf(stderr, "Memory allocation error\n");
            for (int i = 0; i < count; i++)
                free(tokens[i]);
            free(tokens);
            return NULL;
        }
        // remove any extra spaces
        strcpy(tokens[count], stripSpaces(token));
        count++;
        token = strtok(NULL, delimenter);
    }
    // add NULL to indicate end
    tokens[count] = NULL;
    return tokens;
}


char** split_string(const char* input)
{
    char** words = malloc(4 * sizeof(char*));
    if (words == NULL)
    {
        printf("Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    char* copy = strdup(input);
    if (copy == NULL)
    {
        printf("Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    char* token = strtok(copy, " ");
    int i = 0;
    while (token != NULL && i < 4)
    {
        // Allocate memory for the word with "*." prefix
        words[i] = malloc(strlen(token) + 3); // 3 for "*." and null terminator
        if (words[i] == NULL)
        {
            printf("Memory allocation failed\n");
            exit(EXIT_FAILURE);
        }

        remove_special_chars(token); // Remove special characters if needed
        strcpy(words[i], "*.");      // Add ".*" before the word
        strcat(words[i], token);     // Concatenate the word itself
        token = strtok(NULL, " ");
        i++;
    }

    // Fill remaining elements with NULL
    while (i < 4)
    {
        words[i] = "";
        i++;
    }

    free(copy);

    return words;
}

void tokenize_extensions(const char* str, char* a, char* b, char* c)
{
    // Tokenize the input string
    char* token;
    char* str_copy = strdup(str); // Create a copy for tokenization
    token = strtok(str_copy, " ");

    // Skip the first token
    token = strtok(NULL, " ");

    // Assign remaining tokens to variables a, b, c
    if (token != NULL)
    {
        strcpy(a, "*.");
        strcat(a, token);
        token = strtok(NULL, " ");
    }
    else
    {
        strcpy(a, "");
    }

    if (token != NULL)
    {
        strcpy(b, "*.");
        strcat(b, token);
        token = strtok(NULL, " ");
    }
    else
    {
        strcpy(b, "");
    }

    if (token != NULL)
    {
        strcpy(c, "*.");
        strcat(c, token);
    }
    else
    {
        strcpy(c, "");
    }

    free(str_copy);
}
void createTheTar(char* temp1)
{
    // printf("%s\n",temp1);
    char* temp;

    asprintf(&temp, "tar -czvf temp.tar.gz --transform='s|.*/||' %s", temp1);
    printf("%s\n", temp);
    system(temp);
}

char* resolve_paths(const char* paths)
{
    int length = strlen(paths);
    char* resolved_paths = (char*)malloc(3 * length + 1); // Allocate memory for resolved paths
    if (resolved_paths == NULL)
    {
        printf("Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    int i = 0, j = 0;
    while (i < length)
    {
        if (paths[i] == ' ' || paths[i] == '(' || paths[i] == ')')
        {
            resolved_paths[j++] = '\\'; // Escape space or parentheses with "\"
            resolved_paths[j++] = paths[i++];
        }
        else if (paths[i] == '\n')
        {
            resolved_paths[j++] = ' '; // Replace newline with space
            i++;
        }
        else
        {
            resolved_paths[j++] = paths[i++]; // Copy other characters as is
        }
    }
    resolved_paths[j] = '\0'; // Null-terminate the resolved paths

    return resolved_paths;
}

// char *runPopenWithArray(char *str)
// {
//      int pipe_fd[2];
//     pid_t pid;
//     char buffer[1024];
//     char *result = (char *)malloc(4096);
//     if (pipe(pipe_fd) == -1) {
//         perror("pipe");
//         exit(EXIT_FAILURE);
//     }

//     pid = fork();
//     if (pid == -1) {
//         perror("fork");
//         exit(EXIT_FAILURE);
//     }

//     if (pid == 0) { // Child process
//         close(pipe_fd[0]); // Close unused read end
//         dup2(pipe_fd[1], STDOUT_FILENO); // Redirect stdout to the pipe
//         close(pipe_fd[1]); // Close write end of the pipe

//         // Execute the command
//         execl("/bin/sh", "sh", "-c", str, NULL);
//         perror("execl");
//         exit(EXIT_FAILURE);
//     } else { // Parent process
//         close(pipe_fd[1]); // Close unused write end
//         int bytes_read;
//         while ((bytes_read = read(pipe_fd[0], buffer, sizeof(buffer))) > 0) {
//             strcat(result, buffer);
//         }
//         close(pipe_fd[0]); // Close read end of the pipe
//     }
//     return result;
// }


// New Function

// Backup
// char *runPopenWithArray(const char* command)
// {
//     // Duplicate the command
//     char* commandDup = strdup(command);
//     char* args[MAX_CMD_LEN];
//     char* token;
//     int i = 0;

//     // Tokenize the input command based on space
//     token = strtok((char*)commandDup, " ");
//     while (token != NULL)
//     {
//         args[i++] = token;
//         token = strtok(NULL, " ");
//     }
//     args[i] = NULL; // Set the last argument to NULL
//     for(int i = 0; args[i] != NULL; i++){
//         printf("Command Data %s\n", args[i]);
//     }
//     // Execute the command using execvp
//     int p[2];
//     if (pipe(p) < 0)
//     {
//         exit(0);
//     }
//     int child = fork();
//     if (child > 0) {

//         // parent
//         waitpid(child, NULL, 0);
//         char *result = (char *)malloc(MAX_OUTPUT_SIZE);

//         // char inbuf[400]; 

//         read(p[0],result, MAX_OUTPUT_SIZE);
//         printf("%s", result);
//         return result;
//     }
//     else {
//         // child
//         dup2(p[1], 1);
//         if (execvp(args[0], args) == -1)
//         {
//             printf("Command '%s' not found\n", commandDup);
//             return 0;
//         }
//     }
// }


char* runPopenWithArray(char* s)
{
    char* args[] = {
        "bash",
        "-c",
        s,
        NULL
    };
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
        char* result = (char*)malloc(MAX_OUTPUT_SIZE);
        read(p[0], result, MAX_OUTPUT_SIZE);
        printf("%s", result);
        return result;
    }
    else {
        // child
        dup2(p[1], 1);
        if (execvp(args[0], args) == -1)
        {

        }
    }
}

char* addZeros(int num)
{
    char* num_str = (char*)malloc(33 * sizeof(char)); // Allocate memory for string, including null terminator
    sprintf(num_str, "%032d", num);                    // Format the integer with leading zeros
    return num_str;
}

int getFileSize(char* filePath)
{
    int input_fd = open(filePath, O_RDONLY);
    if (input_fd == -1)
    {
        perror("Error opening input file");
        exit(EXIT_FAILURE);
    }
    int c = lseek(input_fd, 0, SEEK_END);
    // printf("%d this is \n",c);
    close(input_fd);
    return c;
}

void sendFile(int client_socket)
{
    sleep(2);
    char buffer[1024] = { 0 };

    // Sending Start of file byte sequence
    char* fileName = "./temp.tar.gz";
    // Calculating file size to send to client
    int fileSize = getFileSize(fileName);
    // Convert the file size to binary string
    char* fileSizeString = addZeros(fileSize);
    // Logging size and binary
    printf("%d File size %s", fileSize, fileSizeString);
    // opening the file to read
    int input_fd = open(fileName, O_RDONLY);
    if (input_fd == -1)
    {
        perror("Error opening input file");
        exit(EXIT_FAILURE);
    }
    // Sending the file size
    send(client_socket, fileSizeString, 32, 0);
    // Sending File Data
    char* tempBuffer[1024] = { 0 };
    for (int i = 0; i < fileSize; i++)
    {
        int br = read(input_fd, tempBuffer, 1);
        send(client_socket, tempBuffer, 1, 0);
    }
    close(input_fd);
}

// Helper method to send a String from the client to server
void sendString(int client_socket, char* s)
{
    int lenght = strlen(s);
    printf("Lenght of s %d\n", lenght);
    // Conver the lenght to a string of lenght 8 to send to the client
    char* lenghtString = addZeros(lenght);
    // Send Length
    send(client_socket, lenghtString, 32, 0);
    // Send data one byte at a time
    for (int i = 0; i < lenght; i++)
    {
        char* tempBuffer = s[i];
        send(client_socket, &s[i], 1, 0);
    }
}

// Helper method to search for a given File
char* searchFiles(char* fileName)
{
    char* command;
    asprintf(&command, "find ~/ -name %s", fileName);
    char* temp = runPopenWithArray(command);
    printf("File Found DAta %s %s\n", command, temp);
    if (strlen(temp) == 0)
        return strdup("-1");
    char* token = strtok(temp, "\n");
    return strdup(token);
}

// use the stat command to return details of the file
void getStatOfFile(int client_socket, char* filePath)
{
    char* command;
    asprintf(&command, "stat -c '%%n\n%%s\n%%w\n%%A' %s", filePath);
    printf("Running this command %s\n", command);
    char** result = splitString(runPopenWithArray(command), "\n");
    char* output;
    asprintf(&output, "FilePath: %s\nFile Size(Bytes): %s\nBirth Time: %s\nAccess Rights: %s\n", result[0], result[1], result[2], result[3]);

    sendString(client_socket, output);
}

void printData(char* s)
{
    printf("Inside pd with value %s\n", s);
}
void crequest(int new_socket)
{
    printf("New Client Connected");
    // Function to handle client requests
    char buffer[1024] = { 0 };
    int valread;

    while (1)
    {
        // Clear the buffer
        memset(buffer, 0, sizeof(buffer));

        // Read the lenght of the command that the client is going to send
        read(new_socket, buffer, 32);
        int sizeofCommand = atoi(buffer);
        memset(buffer, 0, sizeof(buffer));

        // Get command from the client
        int n = read(new_socket, buffer, sizeofCommand);
        printf("%d\n", n);
        if (n == 0)
        {
            break;
        }

        char* command = strdup(buffer);
        memset(buffer, 0, sizeof(buffer));
        printf("user entered %s %s\n", command, strstr(command, "test"));
        // Run the appropriate functions based on the command
        if (strcmp(command, "quitc\n") == 0)
        {
            // If the client sends "quitc", exit the loop and close the connection
            printf("Sever Died\n");
            break;
        }
        else if (strstr(command, "w24fn") != NULL)
        {
            // return details of the file if found
            char* fileName = strchr(command, ' ') + 1;
            char* filePath = searchFiles(fileName);
            char* command[strlen(filePath) + 2]; // Adjust the size according to your needs

            // Construct the command with the filepath enclosed in double quotes
            snprintf(command, sizeof(command), "\"%s\"", filePath);
            printf("File Path %s", filePath);
            if (strcmp("-1", filePath) == 0)
            {
                sendString(new_socket, "Couldnt Find File");
            }
            else
            {
                getStatOfFile(new_socket, command);
            }
        }
        else if (strstr(command, "dirlist -a") != NULL)
        {
            char* temp = runPopenWithArray("ls -l ~/ | grep '^d' | awk '{print $NF}' |sort");
            sendString(new_socket, temp);
            free(temp);
        }
        else if (strstr(command, "dirlist -t") != NULL)
        {

            char* temp = runPopenWithArray("stat --format='%n-%W' ~/*/ | sort -rn | awk '{print $1}' | awk -F'/' '{print $(NF-1)}'");
            sendString(new_socket, temp);
            free(temp);
        }
        // test
        else if (strstr(command, "test") != NULL)
        {
            // Try using this splitString function
            char** words = split_string(command);
            // Print the words
            int i = 0;
            while (words[i] != NULL)
            {
                printf("%s\n", words[i]);
                i++;
            }
            char* temp2;
            asprintf(&temp2, "find ~/Desktop/asp/asplab6 -type f \\( -name '%s' -o -name '%s' -o -name '%s'  \\)", words[1], words[2], words[3]);
            // asprintf(&temp2, "find ~/Desktop/asp/asplab6 -type f \\( -name '%s' -o -name '%s' -o -name '%s'  \\)", a, b, c);
            printf("%s\n", temp2);
            char* temp = runPopenWithArray(temp2);
            createTheTar(resolve_paths(temp));
            // sendString(new_socket, temp);
            // free(temp);
            printf("server\n");
            sendFile(new_socket);
        }
    }
    printf("client disconnected\n");
}

int main()
{
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
        &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr*)&address,
        sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        // Accept the incoming connection
        if ((new_socket = accept(server_fd, (struct sockaddr*)&address,
            (socklen_t*)&addrlen)) < 0)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        // Fork a child process to handle the client request
        int pid = fork();

        if (pid < 0)
        {
            perror("fork");
            exit(EXIT_FAILURE);
        }
        else if (pid == 0)
        {
            // Child process
            // Close the listening socket in the child process
            close(server_fd);
            crequest(new_socket);
            exit(0); // Exit the child process
        }
        else
        {
            // Parent process
            // Close the new socket in the parent process
            close(new_socket);
        }
    }
    return 0;
}
