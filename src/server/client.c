#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <stdlib.h>
#include <stdbool.h>
#include "resp.h"
#define PORT 6379
//use getaddrinfo for modern, cross platform hostname resolution

int __cdecl main() {
    WSADATA wsaData;
    //Initialize Winsock
    int err = WSAStartup(MAKEWORD(2,2), &wsaData); //Use the MAKEWORD(lowbyte, highbyte) macro declared in Windef.h 
    if (err != 0) {
        printf("WSAStartup failed with error: %d\n", err);  //Tell the user that we could not find a usable Winsock DLL
        //WSAStart up directly returns the extended error code in the return value for this function. 
        return 1;   
    }

    //Create socket
    SOCKET sock;
    sock = socket(AF_INET, SOCK_STREAM, 0); // AF_INET - IPV4, SOCK_Stream - TCP protocol
    if(sock == INVALID_SOCKET) {
        printf("Could not create a socket: %d\n", WSAGetLastError());
        return 1;
    }

    //Server details
    struct sockaddr_in server;
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT); //Default Redis port - 6379, change it later
    
    //Conect to server
    if(connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        printf("Connection failed with error code: %d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    printf("Connected to a server! Type \"QUIT\", if you want to close a connection. \n");
    // The Winsock DLL is acceptable. Proceed to use it. 
    while (true) {
        // Send a response to a client
        char mesg[200];
        printf("Enter the message (reply): ");

        if (fgets(mesg, sizeof(mesg), stdin) == NULL) {
            printf("Input error occured. \n");
            break;
        };

        char processBuffer[200];
        strcpy(processBuffer, mesg);
        processBuffer[strcspn(processBuffer, "\r\n")] = '\0'; // Remove trailing newline for processing, but keep original for sending

        char sendArr[10][100];
        char parsingBuffer[sizeof(processBuffer)];
        strcpy(parsingBuffer, processBuffer);

        int numArgs = 0;
        char *token = strtok(parsingBuffer, " ");
        if (strcmp(token, "QUIT") == 0) {
            strcpy(sendArr[0], "QUIT");
            numArgs = 1;
            sendCommand(sock, numArgs, sendArr);
            printf("You closed connection\n");
            break;
        }
        int i = 0;
        while (token != NULL) {
            strcpy(sendArr[i], token);
            numArgs++;
            i++;
            token = strtok(NULL, " ");
        }

        int sbyteCount = sendCommand(sock, numArgs, sendArr);
        if (sbyteCount == SOCKET_ERROR) {
            printf("Client send error: %d\n", WSAGetLastError());
            return 1;
        }

        // Recieve reply
        if (parseResponse(sock) == -1) {
            printf("Error passing server response\n");
            break;
        }
    }

    // then call WSACleanup when done using the Winsock dll 
    closesocket(sock);
    WSACleanup();
    return 0;
}