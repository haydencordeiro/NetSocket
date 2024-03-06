#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <math.h>


#define PORT 8080

// Function to convert a binary string to an integer
int binaryStringToInt(const char* binaryString) {
    int num = 0;
    int len = strlen(binaryString);
    
    // Iterate through each character of the string
    for (int i = 0; i < len; i++) {
        if (binaryString[i] == '1') {
            num |= (1 << (len - i - 1));
        }
    }

    return num;
}

int create_socket() {
    int client_socket;
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    return client_socket;
}

void connect_to_server(int client_socket) {
    struct sockaddr_in server_address;
    if (inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }
}

int main() {
    int client_socket = create_socket();
    connect_to_server(client_socket);

    char buffer[1024] = {0};
    // Request Server for FileintToBinaryString
    char *message = "Send File PleasintToBinaryStringe";
    send(client_socket, message, strlen(message), 0);
    // Reading State of file byte sequence
    read(client_socket, buffer, 8);
    int fileSize = binaryStringToInt(buffer);
    printf("Start sequence: %d\n", binaryStringToInt(buffer));
    memset(buffer, 0, sizeof(buffer));


    for(int i = 0; i< fileSize; i++){
    read(client_socket, buffer, 1);
    printf("Data: %s\n", buffer);
    memset(buffer, 0, sizeof(buffer));
    }

    // Reading data of file


    // Reading End of file of file byte sequence
    // read(client_socket, buffer, 2);
    // printf("eof: %s\n", buffer);
    // memset(buffer, 0, sizeof(buffer));

    close(client_socket);
    return 0;
}
