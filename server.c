#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080

int create_socket()
{
    int server_socket;
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    return server_socket;
}

void bind_socket(int server_socket)
{
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);
    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) < 0)
    {
        perror("Setsockopt failed");
        exit(EXIT_FAILURE);
    }
    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }
}

void listen_for_clients(int server_socket)
{
    if (listen(server_socket, 3) < 0)
    {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }
}

int accept_client(int server_socket)
{
    int client_socket;
    struct sockaddr_in client_address;
    int addrlen = sizeof(client_address);
    if ((client_socket = accept(server_socket, (struct sockaddr *)&client_address, (socklen_t *)&addrlen)) < 0)
    {
        perror("Accept failed");
        exit(EXIT_FAILURE);
    }
    return client_socket;
}

int main()
{
    int server_socket = create_socket();
    bind_socket(server_socket);
    listen_for_clients(server_socket);

    int client_socket = accept_client(server_socket);
    printf("Client connected.\n");

    // Client sends the first request
    char buffer[1024] = {0};
    read(client_socket, buffer, 1024);
    printf("Message from client: %s\n", buffer);

    // Server responds with message

    // Sending Start of file byte sequence
    char *message = "+1";
    send(client_socket, message, 2, 0);
    // Sending File Data
    char *message2 = "This is file data";
    send(client_socket, message2, 1, 0);
    // End of file byte
    char *message3 = "-1";
    send(client_socket, message3, 2, 0);

    close(client_socket);
    close(server_socket);
    return 0;
}
