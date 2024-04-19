#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <math.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>

#define PORT 8081

// function to create socket
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

// function to connect to server
void connect_to_server(int client_socket, int portNumber)
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
    if (connect(client_socket, (struct sockaddr*)&server_address, sizeof(server_address)) < 0)
    {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }
}

// function to receive tarfiles from server to client
void receiveFileHelper(int client_socket)
{
    char buffer[1024] = { 0 };
    // printf("insdie\n");
    // Reading size of file
    read(client_socket, buffer, 32);
    // converting the binary string to int
    int fileSize = atoi(buffer);
    // printf("Start sequence: %s %d\n", buffer, fileSize);
    memset(buffer, 0, sizeof(buffer));
    // Creating and opening the file for writing
    char path[100];
    snprintf(path, sizeof(path), "%s%s", getenv("HOME"), "/w24project");
    mkdir(path, 0777);
    char outputFilename[500];
    snprintf(outputFilename, sizeof(path), "%s%s", getenv("HOME"), "/w24project/temp.tar.gz");

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

// function to receive string data from server to client
char* receiveDataHelper(int client_socket)
{
    char buffer[1024] = { 0 };

    // Reading size of file
    read(client_socket, buffer, 32);
    // converting the binary string to int
    int fileSize = atoi(buffer);
    // printf("Start sequence: %s %d\n", buffer, fileSize);
    memset(buffer, 0, sizeof(buffer));
    // Creating and opening the file for writing

    // Writing the file data
    char file_data[fileSize + 1];

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

// function to addZeros to given number and makes it to 32 bit format
char* addZeros(int num)
{
    char* num_str = (char*)malloc(33 * sizeof(char)); // Allocate memory for string, including null terminator
    sprintf(num_str, "%032d", num);                    // Format the integer with leading zeros
    return num_str;
}

//  Helper method to remove spaces from a string
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

//function to count spaces
int countSpaces(const char* str) {
    int count = 0;
    while (*str != '\0') {
        if (*str == ' ') {
            count++;
        }
        str++;
    }
    return count;
}

// checking substring . in a string
int hasDot(const char* filename) {
    while (*filename != '\0') {
        if (*filename == '.') {
            return 1;
        }
        filename++;
    }
    return 0;
}

//split string based on a given delimiter 
char** splitString(char* str, char* delimenter)
{
    int count = 0;
    char** tokens = (char**)malloc(10 * sizeof(char*));
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

// check if date format taken as input is valid or not
int isValidDateFormat(char* input) {
    char** date = splitString(input, "-");
    if (date[0] == NULL || date[1] == NULL || date[2] == NULL)
    {
        return 0;
    }
    if(strlen(date[0])!=4){
        return 0;
    }
    if(strlen(date[1])!=2 || atoi(date[1]) < 0 || atoi(date[1]) > 12 ){
        return 0;
    }
    if(strlen(date[2])!=2 || atoi(date[2]) < 0 || atoi(date[2]) > 31 ){
        return 0;
    }
    return 1; // Format is valid
}

// check if input is an integer or not
int isInteger(const char* input) {
    char c;
    int num;
    return sscanf(input, "%d%c", &num, &c) == 1;
}

// client side validation for all commands before sending it into the server side
int checkCommand(char* command) {
    // Tokenize the command to check the first part
    char* token = strdup(command);
    char** result = splitString(token, " ");
    if (strstr(token, "dirlist") != NULL) {
        if(result[2]!=NULL){
            printf("Usage: dirlist [-a|-t]\n");
            return 0;
        }
        else if (strcmp(result[0], "dirlist") != 0) {
            printf("Did you mean dirlist?\n");
        }
        if (result[1] != NULL && strcmp(result[1], "-a") == 0) {
            return 1;
        }
        else if (result[1] != NULL && strcmp(result[1], "-t") == 0) {
            return 1;
        }
        else {
            printf("Usage: dirlist [-a|-t]\n");
            return 0;
        }
    }
    else if (strstr(token, "w24fn") != NULL || strstr(token, "w24fz") != NULL || strstr(token, "w24ft") != NULL || strstr(token, "w24fdb") != NULL || strstr(token, "w24fda") != NULL) {
        if (strstr(token, "w24fn") != NULL) {
            if (strcmp(result[0], "w24fn") != 0) {
                printf("Did you mean w24fn?\n");
            }
            // No file present
            if (result[1] == NULL) {
                printf("Usage: w24fn [filename]; requires atleast 1 file");
                return 0;
            }
            // More than one file present
            if (result[2] != NULL) {
                printf("Usage: w24fn [filename]; more than one file provided");
                return 0;
            }
            // More than one file present
            if (!hasDot(result[1])) {
                printf("Usage: w24fn [filename]; Please Provide Valid Filename");
                return 0;
            }

        }
        else if (strstr(token, "w24fz") != NULL) {
            if (strcmp(result[0], "w24fz") != 0) {
                printf("Did you mean w24fz?\n");
            }
            // No file present
            if (result[1] == NULL) {
                printf("Usage: w24fz [size1 size2]; requires atleast 2 sizes");
                return 0;
            }
            // More than one file present
            if (result[2] == NULL) {
                printf("Usage: w24fz [size1 size2]; requires atleast 2 sizes");
                return 0;
            }
            // More than one file present
            if (result[3] != NULL) {
                printf("Usage: w24fz [size1 size2]; more than 2 file sizes given");
                return 0;
            }
            if (!isInteger(result[1])) {
                printf("Usage: w24fz [size1 size2]; size 1 not in int");
                return 0;
            }
            if (!isInteger(result[2])) {
                printf("Usage: w24fz [size1 size2]; size 2 not in int");
                return 0;
            }
            int size1, size2;
            size1 = atoi(result[1]);
            size2 = atoi(result[2]);
            if (size1 < 0) {
                printf("Usage: w24fz [size1 size2]; size 1 should be greater than 0");
                return 0;
            }
            if (size1 > size2) {
                printf("Usage: w24fz [size1 size2]; size 2 should be greater than size 1");
                return 0;
            }

        }
        else if (strstr(token, "w24ft") != NULL) {
            if (strcmp(result[0], "w24ft") != 0) {
                printf("Did you mean w24ft?\n");
            }
            if (result[1] == NULL) {
                printf("Usage: w24fz [ext 1, ext 2, ext 3]; requires atleast 1 extension");
                return 0;
            }

            // More than 4 extension not allowed
            if (result[4] != NULL) {
                printf("Usage: w24fz [ext 1, ext 2, ext 3]; support only upto 3 extension");
                return 0;
            }
        }

        else if (strstr(token, "w24fdb") != NULL) {
            if (strcmp(result[0], "w24fdb") != 0) {
                printf("Did you mean w24fdb?\n");
            }
            // Only 1 allowed
            if (result[1] == NULL) {
                printf("Usage: w24fdb [date]; No date provided");
                return 0;
            }

            // More than 4 extension not allowed
            if (result[2] != NULL) {
                printf("Usage: w24fdb [date]; support only upto 3 extension");
                return 0;
            }
            if (!isValidDateFormat(result[1])) {
                printf("Usage: w24fdb [date]; enter a valid date in format YYYY-MM-DD");
                return 0;
            }
        }
        else if (strstr(token, "w24fda") != NULL) {
            if (strcmp(result[0], "w24fda") != 0) {
                printf("Did you mean w24fda?\n");
            }
            // Only 1 allowed
            if (result[1] == NULL) {
                printf("Usage: w24fda [date]; No date provided");
                return 0;
            }

            // Only one date allowed
            if (result[2] != NULL) {
                printf("Usage: w24fda [date]; supports only one date");
                return 0;
            }
            if (!isValidDateFormat(result[1])) {
                printf("Usage: w24fda [date]; enter a valid date in format YYYY-MM-DD");
                return 0;
            }
        }

    }
    else if (strcmp(token, "quitc") == 0) {
        return 1;
        // printf("Valid command: %s\n", command);
    }
    else {
        printf("Usage:\n");
        printf("dirlist -a\n");
        printf("dirlist -t\n");
        printf("w24fn filename\n");
        printf("w24fz size1 size2\n");
        printf("w24ft <extension list>\n");
        printf("w24fdb date\n");
        printf("w24fda date\n");
        printf("quitc\n");
        return 0;
    }
    return 1;
}
int main()
{
    int client_socket = create_socket();
    connect_to_server(client_socket, PORT);
    send(client_socket, "0", 1, 0);
    char* newPortStr = (receiveDataHelper(client_socket));
    int newPort = atoi(newPortStr);
    close(client_socket);

    client_socket = create_socket();
    connect_to_server(client_socket, newPort);
    char* serverPort;
    asprintf(&serverPort, "%d", PORT);
    // Only send to server1 to indicate request and not load balancing
    if(strcmp(newPortStr, serverPort)==0)
    {
        send(client_socket, "1", 1, 0);
    }
    char command[1024];
    while (1)
    {
        printf("\nclient24$ ");
        fgets(command, 1024, stdin);
        if (checkCommand(command) == 0) {
            continue;
        }

        // printf("user entered %s\n", command);
        // Send length of the command to be received
        send(client_socket, addZeros(strlen(command)), 32, 0);

        // Send command to server
        send(client_socket, command, strlen(command), 0);
        if (strstr(command, "w24fn") != NULL)
        {
            printf("%s", receiveDataHelper(client_socket));
        }
        else if (strstr(command, "dirlist -t") != NULL)
        {
            printf("%s", receiveDataHelper(client_socket));
        }
        else if (strstr(command, "dirlist -a") != NULL)
        {
            printf("%s", receiveDataHelper(client_socket));
        }
        else if (strstr(command, "quitc") != NULL)
        {
            close(client_socket);
            return 0;
        }
        else if (strstr(command, "w24ft") != NULL)
        {

            if (strcmp(receiveDataHelper(client_socket), "no") == 0)
            {
                printf("No file found\n");
                continue;
            }
            receiveFileHelper(client_socket);
        }
        else if (strstr(command, "w24fz") != NULL)
        {

            if (strcmp(receiveDataHelper(client_socket), "no") == 0)
            {
                printf("No file found\n");
                continue;
            }
            receiveFileHelper(client_socket);
        }
        else if (strstr(command, "w24fda") != NULL)
        {

            if (strcmp(receiveDataHelper(client_socket), "no") == 0)
            {
                printf("No file found\n");
                continue;
            }
            receiveFileHelper(client_socket);
        }
        else if (strstr(command, "w24fdb") != NULL)
        {

            if (strcmp(receiveDataHelper(client_socket), "no") == 0)
            {
                printf("No file found\n");
                continue;
            }
            receiveFileHelper(client_socket);
        }
    }
}
