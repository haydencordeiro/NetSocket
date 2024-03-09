#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#define PORT 8080

char *runPopen(char *str)
{
    FILE *fp;
    char buffer[1024];
    char *result = (char *)malloc(4096);

    fp = popen(str, "r");
    if (fp == NULL)
    {
        printf("Failed to run command\n");
        return 1;
    }

    while (fgets(buffer, sizeof(buffer), fp) != NULL)
    {
        strcat(result, buffer);
    }

    pclose(fp);

    char *temp = strdup(result);
    result;
}

char *addZeros(int num)
{
    char *num_str = (char *)malloc(33 * sizeof(char)); // Allocate memory for string, including null terminator
    sprintf(num_str, "%032d", num);                    // Format the integer with leading zeros
    return num_str;
}

int getFileSize(char *filePath)
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
    char buffer[1024] = {0};

    // Sending Start of file byte sequence
    char *fileName = "./serverDir/1temp.pdf";
    // Calculating file size to send to client
    int fileSize = getFileSize(fileName);
    // Convert the file size to binary string
    char *fileSizeString = addZeros(fileSize);
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
    for (int i = 0; i < fileSize; i++)
    {
        char *tempBuffer;
        read(input_fd, tempBuffer, 1);
        send(client_socket, tempBuffer, 1, 0);
    }
}

// Helper method to send a String from the client to server
void sendString(int client_socket, char *s)
{
    int lenght = strlen(s);
    printf("Lenght of s %d\n", lenght);
    // Conver the lenght to a string of lenght 8 to send to the client
    char *lenghtString = addZeros(lenght);
    // Send Length
    send(client_socket, lenghtString, 32, 0);
    // Send data one byte at a time
    for (int i = 0; i < lenght; i++)
    {
        char *tempBuffer = s[i];
        send(client_socket, &s[i], 1, 0);
    }
}

// Dir list command
void dirList(int client_socket, char *option)
{
    if (option == 'a')
    {
        // Use the ls command
        char *temp = runPopen("ls -l ~/ | grep '^d' | awk '{print $NF}' |sort");
        // Send the output to the client
        sendString(client_socket, temp);
        free(temp);
    }
    else
    {
        // Use the stat command
        char *temp = runPopen(" stat --format='%%n %%W' ~/*/ | sort -rn | awk '{print $1}' | awk -F'/' '{print $(NF-1)}'");
        // Send the output to the client
        sendString(client_socket, temp);
        free(temp);
    }
}

// Helper method to search for a given File
char *searchFiles(char *fileName)
{
    char *command;
    asprintf(&command, "find ~/ -name %s", fileName);
    char *temp = runPopen(command);
    printf("File Found DAta %s %s\n",command, temp);
    if (strlen(temp) == 0)
        return "-1";
    char *token = strtok(temp, "\n");
    return strdup(token);
}

// use the stat command to return details of the file
void getStatOfFile(int client_socket, char *filePath)
{
    char *command;
    asprintf(&command, "stat -c '%%n %%s %%w %%A' %s", filePath);
    printf("Running this command %s\n",command);
    sendString(client_socket, runPopen(command));
}

void printData(char *s)
{
    printf("Inside pd with value %s\n", s);
}
void crequest(int new_socket)
{
    // Function to handle client requests
    char buffer[1024] = {0};
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
        read(new_socket, buffer, sizeofCommand);
        char *command = strdup(buffer);
        memset(buffer, 0, sizeof(buffer));
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
            char *fileName = strchr(command, ' ') + 1;
            char *filePath = searchFiles(fileName);
            if (strcmp("-1", fileName) == 0)
            {
                sendString(new_socket, "Couldnt Find File");
            }
            else
            {
                getStatOfFile(new_socket, filePath);
            }
        }
        else if (strstr(command, "dirlist -a") != NULL)
        {
            char *temp = runPopen("ls -l ~/ | grep '^d' | awk '{print $NF}' |sort");
            sendString(new_socket, temp);
            free(temp);
        }
        else if (strstr(command, "dirlist -t") != NULL)
        {
            char *temp = runPopen(" stat --format='%n %W' ~/*/ | sort -rn | awk '{print $1}' | awk -F'/' '{print $(NF-1)}'");
            sendString(new_socket, temp);
            free(temp);
        }
    }
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
    if (bind(server_fd, (struct sockaddr *)&address,
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
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                                 (socklen_t *)&addrlen)) < 0)
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
