#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <math.h>

#include <unistd.h>
#include <fcntl.h>

#define PORT 8080

// Function to convert a binary string to an integer
int binaryStringToInt(const char *binaryString)
{
    int num = 0;
    int len = strlen(binaryString);

    // Iterate through each character of the string
    for (int i = 0; i < len; i++)
    {
        if (binaryString[i] == '1')
        {
            num |= (1 << (len - i - 1));
        }
    }

    return num;
}

int create_socket()
{
    int client_socket;
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    return client_socket;
}

void connect_to_server(int client_socket)
{
    struct sockaddr_in server_address;
    if (inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr) <= 0)
    {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }
}

void receiveFileHelper(int client_socket)
{
    char buffer[1024] = {0};

    // Reading size of file
    read(client_socket, buffer, 32);
    // converting the binary string to int
    int fileSize = binaryStringToInt(buffer);
    printf("Start sequence: %s %d\n", buffer, fileSize);
    memset(buffer, 0, sizeof(buffer));
    // Creating and opening the file for writing
    char *outputFilename = "./outputFilename.pdf";
    int output_fd = open(outputFilename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (output_fd == -1)
    {
        perror("Error creating output file");
        exit(EXIT_FAILURE);
    }
    // Writing the file data
    for (int i = 0; i < fileSize; i++)
    {
        read(client_socket, buffer, 1);
        printf("Data: %s\n", buffer);
        write(output_fd, buffer, 1);
        memset(buffer, 0, sizeof(buffer));
    }
    close(output_fd);
}

void receiveDataHelper(int client_socket)
{
    char buffer[1024] = {0};

    // Reading size of file
    read(client_socket, buffer, 32);
    // converting the binary string to int
    int fileSize = binaryStringToInt(buffer);
    // printf("Start sequence: %s %d\n", buffer, fileSize);
    memset(buffer, 0, sizeof(buffer));
    // Creating and opening the file for writing

    // Writing the file data
    for (int i = 0; i < fileSize; i++)
    {
        read(client_socket, buffer, 1);
        printf("%s", buffer);
        memset(buffer, 0, sizeof(buffer));
    }
}

char* add_zeros(int num) {
    char* num_str = (char*)malloc(33 * sizeof(char)); // Allocate memory for string, including null terminator
    sprintf(num_str, "%032d", num); // Format the integer with leading zeros
    return num_str;
}

int main()
{
    int client_socket = create_socket();
    connect_to_server(client_socket);

    char buffer[1024] = {0};
    // char *commnad = "dirlist -a";
    // char *commnad = "dirlist -t";
    char *command = "w24fn temp.txt";

    // Send lenght of the command to be received
    send(client_socket, add_zeros(strlen(command)), 32, 0);

    // Send command to server
    send(client_socket, command, strlen(command), 0);
    if (strstr(command, "w24fn") != NULL)
    {
        printf("here\n");
        receiveDataHelper(client_socket);
    }

    // receiveFileHelper(client_socket);
    close(client_socket);
    return 0;
}
