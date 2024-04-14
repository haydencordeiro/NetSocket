#define _GNU_SOURCE
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
#include <time.h>

#define PORT 8083
// Max Number of Spaces in runCommand
#define MAX_CMD_LEN 1000
// Max File Size of 
#define MAX_OUTPUT_SIZE 10000000
// Used in split string function
#define MAX_NO_ARGUMENTS 400
int numberOfClients = 0;




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

// helper method to remove special characters
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

// helper method to split strings
char** split_string(const char* input, char* option)
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
        if (strcmp(option,"fileExtension")==0)
        {
        strcpy(words[i], "*.");      // Add ".*" before the word
        }
            if (strcmp(option,"fileSize")==0)
        {
        strcpy(words[i], "");      // Add ".*" before the word
        }    
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

// helper method to tokenize extensions for w24ft
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

// method to create tarfile in serverside with unique id
void createTheTar(char* temp1, char* tarFile)
{
    // printf("%s\n",tarFile);
    // printf("%s\n",temp1);
    char* temp;

    asprintf(&temp, "tar -czvf %s --transform='s|.*/||' %s",tarFile, temp1);
    printf("%s\n", temp);
    system(temp);
}

// helper method for escape sequence space to cleanup the string
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

// helper method to multiple piping commands.
char* commandHelper(char* s) {
    char* args[] = { "bash", "-c", s, NULL };

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
    }
    else if (pid == 0) { // Child process
        close(p[0]); // Close reading end of pipe
        dup2(p[1], STDOUT_FILENO); // Redirect stdout to the pipe write end
        close(p[1]); // Close the write end of the pipe in the child
        if (execvp(args[0], args) == -1) {
            perror("Execution failed");
            exit(EXIT_FAILURE);
        }
    }
    else { // Parent process
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
        return strdup(result);
    }
}

// Update Log
void updateLog() {
    char* temp;
    asprintf(&temp, "echo %d > mirror2.txt", numberOfClients);
    commandHelper(temp);
}

// helper method to to make 32bit integer
char* addZeros(int num)
{
    char* num_str = (char*)malloc(33 * sizeof(char)); // Allocate memory for string, including null terminator
    sprintf(num_str, "%032d", num);                    // Format the integer with leading zeros
    return num_str;
}

// helper method to get fileSize
int getFileSize(char* filePath)
{
    int input_fd = open(filePath, O_RDONLY);
    if (input_fd == -1)
    {
        perror("Error opening input file");
        exit(EXIT_FAILURE);
    }
    int c = lseek(input_fd, 0, SEEK_END);
    close(input_fd);
    return c;
}

// helper method to send file from server to client
void sendFile(int client_socket, char* tarFile)
{
    // printf("SEND FILE TARFILE NAME  :  %s \n",tarFile);
    sleep(2);
    char buffer[1024] = { 0 };

    // Sending Start of file byte sequence
    // char* fileName = "./temp.tar.gz";
    char* fileName = tarFile;
    // Calculating file size to send to client
    int fileSize = getFileSize(fileName);
    // Convert the file size to binary string
    char* fileSizeString = addZeros(fileSize);
    // Logging size and binary
    // printf("%d File size %s", fileSize, fileSizeString);
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
    unlink(tarFile);
}

// Helper method to send a String from the client to server
void sendString(int client_socket, char* s)
{
    int lenght = strlen(s);
    // printf("Lenght of s %d\n", lenght);
    // Conver the lenght to a string of lenght 8 to send to the client
    char* lenghtString = addZeros(lenght);
    // Send Length
    send(client_socket, lenghtString, 32, 0);
    // Send data one byte at a time
    for (int i = 0; i < lenght; i++)
    {
        send(client_socket, &s[i], 1, 0);
    }
}

// Helper method to search for a given File
char* searchFiles(char* fileName)
{
    char* command;
    asprintf(&command, "find ~/ -name %s", fileName);
    char* temp = commandHelper(command);
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
    char** result = splitString(commandHelper(command), "\n");
    char* output;
    asprintf(&output, "FilePath: %s\nFile Size(Bytes): %s\nBirth Time: %s\nAccess Rights: %s\n", result[0], result[1], result[2], result[3]);

    sendString(client_socket, output);
}

// helper method to check null condition
int checkCondition(char * command, char * subString){
    char* temp = strdup(command);
    return strstr(temp, subString)!= NULL;
}

// helper method to check client request for further processing
void crequest(int new_socket)
{
    printf("New Client Connected \n");
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
        if (n == 0)
        {
            break;
        }

        char* command = strdup(buffer);
        printf("Client Request : %s \n", command);
        memset(buffer, 0, sizeof(buffer));
        // Run the appropriate functions based on the command
        if (strcmp(command, "quitc\n") == 0)
        {
            // If the client sends "quitc", exit the loop and close the connection
            printf("Sever Died\n");
            break;
        }
        else if (checkCondition(command, "w24fn"))
        {
            // return details of the file if found
            char* fileName = strchr(command, ' ') + 1;
            char* filePath = searchFiles(fileName);
            char* command; // Adjust the size according to your needs

            // Construct the command with the filepath enclosed in double quotes
            asprintf(&command, "\"%s\"", filePath);

            // printf("File Path %s", filePath);
            if (strcmp("-1", filePath) == 0)
            {
                sendString(new_socket, "Couldnt Find File");
            }
            else
            {
                getStatOfFile(new_socket, command);
            }
        }
        else if (strcmp(command, "dirlist -a\n") == 0)
        {
            char* temp = commandHelper("find ~/ -maxdepth 1 -type d -not -path '*/.*' -print0 | xargs -0 -I{} basename {} | sort");
            sendString(new_socket, temp);
            free(temp);
        }
        else if (strcmp(command, "dirlist -t\n") == 0)
        {

            char* temp = commandHelper("find ~/ -maxdepth 1 -type d -not -path '*/.*' -print0 | xargs -0 -I{} stat --format='%W*{}' {} | sort -t '*' -k 1 | awk -F '*' '{print $2}' | xargs -I{} basename {}");
            sendString(new_socket, temp);
            free(temp);
        }
        else if (checkCondition(command, "w24ft")) 
        {    
            char** result = split_string(command,"fileExtension");
            char *temp2;
            asprintf(&temp2, "find ~/ -type f -not -path '*/.*' \\( -name '%s' -o -name '%s' -o -name '%s'  \\)", result[1], result[2], result[3]);
            // printf("\n Final Command to Run is %s \n", temp2);
            if(strlen(commandHelper(strdup(temp2))) == 0){
               sendString(new_socket, "no");
               continue;
            }
            sendString(new_socket, "yes");
            
            srand(time(NULL));
            char* unique_string = (char*)malloc(100 * sizeof(char));
            // Generate random number
            int random_number = 100000000 + rand() % 900000000;
             // Concatenate random number with the specified format
            snprintf(unique_string, 100, "%d.tar.gz", random_number);

            createTheTar(resolve_paths(commandHelper(strdup(temp2))),strdup(unique_string));
            sendFile(new_socket,strdup(unique_string));
            // free(unique_string);
        }
        else if (checkCondition(command, "w24fz")) 
        {    
            char** result = split_string(command,"fileSize");
            char *temp2;
            asprintf(&temp2, "find ~/ -type f -not -path '*/.*' -size +%sc -size -%sc", result[1], result[2]);
            // asprintf(&temp2, "find ~/ -type f -not -path '*/.*' \\( -name '%s' -o -name '%s' -o -name '%s'  \\)", result[1], result[2], result[3]);
            // printf("\n Final Command to Run is %s \n", temp2);
            if(strlen(commandHelper(strdup(temp2))) == 0){
               sendString(new_socket, "no");
               continue;
            }
            sendString(new_socket, "yes");
            
            srand(time(NULL));
            char* unique_string = (char*)malloc(100 * sizeof(char));
            // Generate random number
            int random_number = 100000000 + rand() % 900000000;
             // Concatenate random number with the specified format
            snprintf(unique_string, 100, "%d.tar.gz", random_number);

            createTheTar(resolve_paths(commandHelper(strdup(temp2))),strdup(unique_string));
            sendFile(new_socket,strdup(unique_string));
            // free(unique_string);
        }
        
        else if (checkCondition(command, "w24fda")) 
        {    
            char** result = split_string(command,"fileSize");
            char *temp2;
            asprintf(&temp2, "find ~/ -type f ! -path  '*/.*' | xargs -I{}  stat -c '%%W*{}' {} | awk -v date=$(date -d %s +%%s) -F'*' '$1 > date' | awk -F '*' '{print $2}'", result[1]);
            // asprintf(&temp2, "find ~/ -type f -not -path '*/.*' \\( -name '%s' -o -name '%s' -o -name '%s'  \\)", result[1], result[2], result[3]);
            // printf("\n Final Command to Run is %s \n", temp2);
            if(strlen(commandHelper(strdup(temp2))) == 0){
               sendString(new_socket, "no");
               continue;
            }
            sendString(new_socket, "yes");
            
            srand(time(NULL));
            char* unique_string = (char*)malloc(100 * sizeof(char));
            // Generate random number
            int random_number = 100000000 + rand() % 900000000;
             // Concatenate random number with the specified format
            snprintf(unique_string, 100, "%d.tar.gz", random_number);

            createTheTar(resolve_paths(commandHelper(strdup(temp2))),strdup(unique_string));
            sendFile(new_socket,strdup(unique_string));
            // free(unique_string);
        }
        else if (checkCondition(command, "w24fdb")) 
        {    
            char** result = split_string(command,"fileSize");
            char *temp2;
            asprintf(&temp2, "find ~/ -type f ! -path  '*/.*' | xargs -I{}  stat -c '%%W*{}' {} | awk -v date=$(date -d %s +%%s) -F'*' '$1 < date' | awk -F '*' '{print $2}'", result[1]);
            // asprintf(&temp2, "find ~/ -type f -not -path '*/.*' \\( -name '%s' -o -name '%s' -o -name '%s'  \\)", result[1], result[2], result[3]);
            // printf("\n Final Command to Run is %s \n", temp2);
            if(strlen(commandHelper(strdup(temp2))) == 0){
               sendString(new_socket, "no");
               continue;
            }
            sendString(new_socket, "yes");
            
            srand(time(NULL));
            char* unique_string = (char*)malloc(100 * sizeof(char));
            // Generate random number
            int random_number = 100000000 + rand() % 900000000;
             // Concatenate random number with the specified format
            snprintf(unique_string, 100, "%d.tar.gz", random_number);

            createTheTar(resolve_paths(commandHelper(strdup(temp2))),strdup(unique_string));
            sendFile(new_socket,strdup(unique_string));
            // free(unique_string);
        }
        else{
            printf("No matches found\n");
        }
    }
    kill(getppid(), SIGFPE);
    printf("client disconnected\n");
}

// siginthandler
void sigChildHandler() {
    numberOfClients -= 1;
    updateLog();
}


int main()
{
    signal(SIGFPE, sigChildHandler);
    int server_fd, new_socket, valread;
    // Socket Address Object(Struct)
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Creating socket file descriptor
    // Aruments(Address Family [AF_INET/AP_UNIX] , TCP/UDP [SOCK_STREAM/ SOCK_DGRAM], 0 -> Default protocol of next layer)
    // Create Raw TCP Socket
    // Changed from == to 0 to < 0
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }


    // SKI{}
    // Forcefully attaching socket to the port 8081
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
        &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // Before using bind we need to make changes to socket address object
    // Setting Socket Family
    address.sin_family = AF_INET;
    // Setting the ip address
    // INADDR_ANY -> Gives me the current IP address of the system
    // Convert Host Byte order to Network Byte order 
    // Host to network long
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    // Setting the port Number
    // Host to network Short (because 16 bits)
    address.sin_port = htons(PORT);

    //  Bind
    // ARGS (listening socket fd, server address object, sizeof the address object)
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    // The server is now ready to listen to client connection
    // ARGS (listening socket description, queue lenght)
    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        // Accept the incoming connection
        // ARGS (listening socket FD,)
        // Need to debug this
        if ((new_socket = accept(server_fd, (struct sockaddr*)&address,
            (socklen_t*)&addrlen)) < 0)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        numberOfClients += 1;
        updateLog();
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
