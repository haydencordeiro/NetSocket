#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <math.h>

#include <unistd.h>
#include <fcntl.h>

#define PORT 8081


int create_socket()
{
    // Create socket (Same family and socket type as server)
    int client_socket;
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    return client_socket;
}

void connect_to_server(int client_socket,int portNumber)
{
    // Address object
    struct sockaddr_in server_address;
    // Update server IP in address object
    if (inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr) <= 0)
    {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }
    // setting address family and port
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(portNumber);
    // ARGS (Socket descriptor, address object, size of the address object)
    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }
}

void receiveFileHelper(int client_socket)
{
    char buffer[1024] = {0};
    printf("insdie\n");
    // Reading size of file
    read(client_socket, buffer, 32);
    // converting the binary string to int
    int fileSize = atoi(buffer);
    printf("Start sequence: %s %d\n", buffer, fileSize);
    memset(buffer, 0, sizeof(buffer));
    // Creating and opening the file for writing
    unlink("./temp.tar.gz");
    char *outputFilename = "./temp.tar.gz";
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
        // printf("Data: %s %d %d\n", buffer,i,fileSize);
        write(output_fd, buffer, 1);
        memset(buffer, 0, sizeof(buffer));
    }
    printf("done\n");
    close(output_fd);
}

char * receiveDataHelper(int client_socket)
{
    char buffer[1024] = {0};

    // Reading size of file
    read(client_socket, buffer, 32);
    // converting the binary string to int
    int fileSize = atoi(buffer);
    // printf("Start sequence: %s %d\n", buffer, fileSize);
    memset(buffer, 0, sizeof(buffer));
    // Creating and opening the file for writing
    
    // Writing the file data
    char file_data[fileSize +1]; 

    // 
    for (int i = 0; i < fileSize; i++)
    {
        read(client_socket, buffer, 1);
        // printf("%s", buffer);
        file_data[i] = buffer[0];
        memset(buffer, 0, sizeof(buffer));

    }
    file_data[fileSize] = '\0';
    // printf("%s\n", file_data);
    return strdup(file_data);

}

char *addZeros(int num)
{
    char *num_str = (char *)malloc(33 * sizeof(char)); // Allocate memory for string, including null terminator
    sprintf(num_str, "%032d", num);                    // Format the integer with leading zeros
    return num_str;
}

int main()
{
    int client_socket = create_socket();
    connect_to_server(client_socket,PORT);
    send(client_socket, "0", 1, 0);
    int newPort = atoi(receiveDataHelper(client_socket));
    close(client_socket);

    client_socket = create_socket();
    connect_to_server(client_socket,newPort);
    send(client_socket, "1", 1, 0);
    char command[1024];
    while (1)
    {
        // char buffer[1024] = {0};
        // scanf("%[^\n]s", command);
        fgets(command, 1024, stdin);

        printf("user entered %s\n", command);
        // char *command = "dirlist -t";
        // char *command = "w24fn temp.txt";

        // Send lenght of the command to be received
        send(client_socket, addZeros(strlen(command)), 32, 0);

        // Send command to server
        send(client_socket, command, strlen(command), 0);
        if (strstr(command, "w24fn") != NULL)
        {
            printf("%s",receiveDataHelper(client_socket));
        }
        else if (strstr(command, "dirlist -t") != NULL)
        {
            printf("%s",receiveDataHelper(client_socket));
        }
        else if (strstr(command, "dirlist -a") != NULL)
        {
            printf("%s",receiveDataHelper(client_socket));
        }
        else if (strstr(command, "quitc") != NULL)
        {
            close(client_socket);
            return 0;
        }
        else if (strstr(command, "w24ft") !=NULL)
        {
            
            printf("client\n");
            if (strcmp(receiveDataHelper(client_socket),"no")==0)
            {
                printf("No file Found\n");
                continue;
            }
            receiveFileHelper(client_socket);
        }
        else if (strstr(command, "w24fz") !=NULL)
        {
            
            printf("client\n");
            if (strcmp(receiveDataHelper(client_socket),"no")==0)
            {
                printf("No file Found\n");
                continue;
            }
            receiveFileHelper(client_socket);
        }
         else if (strstr(command, "w24da") !=NULL)
        {
            
            printf("client\n");
            if (strcmp(receiveDataHelper(client_socket),"no")==0)
            {
                printf("No file Found\n");
                continue;
            }
            receiveFileHelper(client_socket);
        }
        else if (strstr(command, "w24db") !=NULL)
        {
            
            printf("client\n");
            if (strcmp(receiveDataHelper(client_socket),"no")==0)
            {
                printf("No file Found\n");
                continue;
            }
            receiveFileHelper(client_socket);
        }
    }
}
