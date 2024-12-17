#include <stdio.h>
#include <winsock2.h>
#include <pthread.h>
#include <string.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 8080
#define SERVER_IP "127.0.0.1"

void* receive_messages(void* socket);

int main() {
    WSADATA wsa;
    SOCKET client_socket;
    struct sockaddr_in server;
    char message[1024], server_reply[1024];
    pthread_t recv_thread;
    char client_name[50];  // ley yi enregistre oussoum hana client

    //wisock yi inintialise
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("WSAStartup failed. Error Code: %d\n", WSAGetLastError());
        return 1;
    }

    // Creation hana sock
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == INVALID_SOCKET) {
        printf("Socket creation failed.\n");
        WSACleanup();  
        return 1;
    }

    // yi chila oussoum hana client
    printf("Enter your name: ");
    fgets(client_name, sizeof(client_name), stdin);
    client_name[strcspn(client_name, "\n")] = 0; // Remove the newline character

    // yi connecter hou fi server
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(SERVER_IP);
    server.sin_port = htons(PORT);

    if (connect(client_socket, (struct sockaddr*)&server, sizeof(server)) < 0) {
        printf("Connection failed.\n");
        closesocket(client_socket);
        WSACleanup();
        return 1;
    }

    // envois oussoum fi server
    send(client_socket, client_name, strlen(client_name), 0);

    printf("Connected to server!\n");

    // yi cree thread hana message
    if (pthread_create(&recv_thread, NULL, receive_messages, (void*)&client_socket) != 0) {
        printf("Failed to create receiving thread.\n");
        closesocket(client_socket);
        WSACleanup();
        return 1;
    }

    // yi rassila message
    while (1) {
        fgets(message, sizeof(message), stdin);
        message[strcspn(message, "\n")] = 0;  // Remove newline
        if (strlen(message) > 0) {
            // yi mourgua message hana nadoumda wa oussouma
            char full_message[1024];
            snprintf(full_message, sizeof(full_message), "%s: %s", client_name, message);
            send(client_socket, full_message, strlen(full_message), 0);
        }
    }

    closesocket(client_socket);
    WSACleanup();
    return 0;
}

void* receive_messages(void* socket) {
    SOCKET sock = *(SOCKET*)socket;
    char server_reply[1024];
    int read_size;

    while ((read_size = recv(sock, server_reply, sizeof(server_reply), 0)) > 0) {
        server_reply[read_size] = '\0';
        printf("%s\n", server_reply);  // yi mourgua message al dja da ma oussouma sawa
    }

    return 0;
}