#include <stdio.h>
#include <winsock2.h>
#include <pthread.h>
#include <string.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 8080
#define MAX_CLIENTS 10

SOCKET client_sockets[MAX_CLIENTS];
char client_names[MAX_CLIENTS][50];  // Array hana oussoum dolda
int client_count = 0;

void* handle_client(void* client_socket);
void send_to_all_clients(char* message, SOCKET sender_sock);

int main() {
    WSADATA wsa;
    SOCKET server_socket, client_socket;
    struct sockaddr_in server, client;
    int client_size = sizeof(client);

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("WSAStartup failed. Error Code: %d\n", WSAGetLastError());
        return 1;
    }

    // Create socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET) {
        printf("Socket creation failed.\n");
        WSACleanup();
        return 1;
    }

    // Bind socket
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr*)&server, sizeof(server)) < 0) {
        printf("Bind failed.\n");
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    listen(server_socket, 3);
    printf("Server listening on port %d...\n", PORT);

    // Accept incoming connections
    while ((client_socket = accept(server_socket, (struct sockaddr*)&client, &client_size))) {
        printf("New client connected!\n");

        // yi chila oussoum hana client
        char client_name[50];
        int name_length = recv(client_socket, client_name, sizeof(client_name), 0);
        if (name_length > 0) {
            client_name[name_length] = '\0';
        }

        // yi enregistre soket hana client da wa isima
        client_sockets[client_count] = client_socket;
        strncpy(client_names[client_count], client_name, sizeof(client_names[client_count]) - 1);
        client_count++;

        // yi sewi alert ley nas al akharine da yi waasifoum nadoum al connecter da yatou
        char join_message[1024];
        snprintf(join_message, sizeof(join_message), "%s has joined the chat!", client_name);
        send_to_all_clients(join_message, client_socket);

        //yi creee thread ley chaq client
        pthread_t client_thread;
        pthread_create(&client_thread, NULL, handle_client, (void*)&client_socket);
    }

    closesocket(server_socket);
    WSACleanup();
    return 0;
}

void* handle_client(void* client_socket) {
    SOCKET sock = *(SOCKET*)client_socket;
    char buffer[1024];
    int read_size;

    // yi gere message aldjayi dolda djayi min yatou 
    while ((read_size = recv(sock, buffer, sizeof(buffer), 0)) > 0) {
        buffer[read_size] = '\0';
        
        // message jayi min yatou yi waasifa
        printf("Message from client: %s\n", buffer);

        // yi publier message da le nas akharine koula
        send_to_all_clients(buffer, sock);
    }

    // kam desabonner da
    //yalgua nadoum al desabonner wa yi khabira al akharin  dol
    char disconnect_message[1024];
    for (int i = 0; i < client_count; i++) {
        if (client_sockets[i] == sock) {
            snprintf(disconnect_message, sizeof(disconnect_message), "%s has left the chat.", client_names[i]);
            send_to_all_clients(disconnect_message, sock);
            closesocket(sock);
            // sallin hana client kam desabonner
            for (int j = i; j < client_count - 1; j++) {
                client_sockets[j] = client_sockets[j + 1];
                strcpy(client_names[j], client_names[j + 1]);
            }
            client_count--;
            break;
        }
    }
    return 0;
}

void send_to_all_clients(char* message, SOCKET sender_sock) {
    // message ley client dol bala sender
    for (int i = 0; i < client_count; i++) {
        if (client_sockets[i] != sender_sock) {
            send(client_sockets[i], message, strlen(message), 0);
        }
    }
}