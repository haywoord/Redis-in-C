#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <stdlib.h>
#include <stdbool.h>
#define PORT 8080

int main() {
    WSADATA wsaData;

    int err = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (err != 0) {
        printf("WSAStartup failed with error: %d\n", err);
        return 1;
    }

    //Create a socket
    SOCKET serverSocket;
    serverSocket = INVALID_SOCKET;
    serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        printf("Error at socket: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    //Bind the Socket
    struct sockaddr_in service;
    service.sin_family = AF_INET;
    service.sin_addr.s_addr = inet_addr("127.0.0.1");
    service.sin_port = htons(PORT);

    //Use the bind function
    if (bind(serverSocket,(struct sockaddr *) &service, sizeof(service)) == SOCKET_ERROR) {
        printf("Bind failed: %d\n", WSAGetLastError());
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    //Listen for incoming connections
    if (listen(serverSocket, 1) == SOCKET_ERROR) {
        printf("Error listening on socket: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }
    //Accept incoming connections
    SOCKET acceptSocket;
    acceptSocket = accept(serverSocket, NULL, NULL);
    // Check for succesfull connections
    if (acceptSocket == INVALID_SOCKET) {
        printf("Accept failed: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    while (true) {
        // Recive data from the client
        char receiveBuffer[200];
        int rbyteCount = recv(acceptSocket, receiveBuffer, sizeof(receiveBuffer), 0);
        if (rbyteCount == SOCKET_ERROR) {
            printf("Server recive error: %d\n", WSAGetLastError());
            return 1;
        }
        if (rbyteCount > 0) {
            receiveBuffer[rbyteCount] = '\0';
            printf("Message from a client: %s", receiveBuffer);
            for (int i = 0; i < strlen(receiveBuffer); i++) {
                if (receiveBuffer[i] >= 'a' && receiveBuffer[i] <= 'z') {
                    receiveBuffer[i] = receiveBuffer[i] - 32;
                }
            }
            if (strcmp(receiveBuffer, "QUIT\n") == 0) {
                printf("Client requested to close the connection.\n");
                break;
            }
        }
        else if (rbyteCount == 0) {
            printf("Clien disconnected. \n");
            break;
        }

        // Send a response to a client
        char buffer[200];
        printf("Enter the message (reply): ");
        fgets(buffer, sizeof(buffer), stdin);
        int sbyteCount = send(acceptSocket, buffer, strlen(buffer), 0);
        if (sbyteCount == SOCKET_ERROR) {
            printf("Server send error: %d\n", WSAGetLastError());
            return 1;
        }
    }
    closesocket(acceptSocket);
    closesocket(serverSocket);
    WSACleanup();

    return 0;
}