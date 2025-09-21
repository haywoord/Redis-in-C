#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <stdlib.h>
#include <stdbool.h>
#include "linlist.h"
#define PORT 8080

//char *strremove(char *str, const char *sub);
//int sendResult(SOCKET sock, const char *msg);

struct Entry* head = NULL;

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
        char receiveBuffer[200] = {0}; //important, initialize to all zeros, because recv function doesn't clear the buffer
        //it just writes new data on top of what's already there. If the new data is shorter than the old data, remnants of the previous message will remain at the end of the buffer, causing subsequent strcmp checks to fail.
        int rbyteCount = recv(acceptSocket, receiveBuffer, sizeof(receiveBuffer) - 1, 0);
        if (rbyteCount == SOCKET_ERROR) {
            printf("Server recived error: %d\n", WSAGetLastError());
            break;
        }
        if (rbyteCount > 0) {
            receiveBuffer[rbyteCount] = '\0';
            receiveBuffer[strcspn(receiveBuffer,"\r\n")] = '\0'; //Remove trainling newline and carriage return
            printf("Message from client: '%s'\n", receiveBuffer);
            if (strncmp(receiveBuffer, "ECHO", 4) == 0) {
                if (strlen(receiveBuffer) > 4 && receiveBuffer[4] == ' ') {
                    //Point to the content after echo
                    char *echoContent = receiveBuffer + 5;
                    //Create a buffer to hold the content + new line
                    char responseBuffer[200];
                    //Use snprintf to safely copy the string and add a new line with null-terminator
                    int writCopy = snprintf(responseBuffer, sizeof(responseBuffer),"%s\n", echoContent);
                    if (writCopy >= sizeof(responseBuffer)) {
                        printf("Warning: response truncated\n");
                    }
                    printf("ECHO command: %s\n", echoContent);
                    int sendResult = send(acceptSocket, responseBuffer, strlen(responseBuffer), 0);
                    if (sendResult == SOCKET_ERROR) {
                        printf("Send error: %d\n", WSAGetLastError());
                        break;
                    }
                }
                else {
                    printf("ECHO command received. No arguments to echo.\n");
                    const char* response = "ECHO command received. No arguments to echo.\n";
                    int sendResult = send(acceptSocket, response, strlen(response), 0);
                    if (sendResult == SOCKET_ERROR) {
                        printf("Send error: %d\n", WSAGetLastError());
                        break;
                    }
                }
            }
            else if (strcmp(receiveBuffer, "PING") == 0) {
                printf("PONG\r\n");
                int sendResult = send(acceptSocket,"PONG\n", strlen("PONG\n"), 0);
                if (sendResult == SOCKET_ERROR) {
                    printf("Send error: %d\n", WSAGetLastError());
                    break;
                }
            }
            else if (strncmp(receiveBuffer, "SET ", 4) == 0) {
                char *arg = receiveBuffer + 4;
                char *key = arg;
                char *value = strchr(arg, ' ');
                if (value != NULL) {
                    *value = '\0';
                    value++;
                    if (checkTheKey(head, key) == true) {
                        int sendResult = send(acceptSocket, "The key is already in use\n", strlen("The key is already in use\n"), 0);
                        if (sendResult == SOCKET_ERROR) {
                            printf("Send error: %d\n", WSAGetLastError());
                            break;
                    }
                    }
                    else {
                        insertAtEnd(&head, key, value); 
                        int sendResult = send(acceptSocket, "OK\n", strlen("OK\n"), 0);
                        if (sendResult == SOCKET_ERROR) {
                            printf("Send error: %d\n", WSAGetLastError());
                            break;
                        }
                    }
                }
                else {
                    int sendResult = send(acceptSocket, "Invalid SET command\n", strlen("Invalid SET command\n"), 0);
                    if (sendResult == SOCKET_ERROR) {
                        printf("Send error: %d\n", WSAGetLastError());
                        break;
                    }
                }
            }
            else if (strncmp(receiveBuffer, "GET ", 4) == 0) {
                char *key = receiveBuffer + 4;
                char *tokResult = printFromPosition(head, key);
                if (tokResult != NULL) {
                    char responseBuffer[200]; //use Malloc later
                    snprintf(responseBuffer, sizeof(responseBuffer), "%s\n", tokResult);
                    int sendResult = send(acceptSocket, responseBuffer, strlen(responseBuffer), 0);
                    if (sendResult == SOCKET_ERROR) {
                        printf("Send error: %d\n", WSAGetLastError());
                        break;
                    }
                }
                else {
                    int sendResult = send(acceptSocket, "Key not found\n", strlen("Key not found\n"), 0);
                    if (sendResult == SOCKET_ERROR) {
                        printf("Send error: %d\n", WSAGetLastError());
                        break;
                    }
                }
            }
            else if (strncmp(receiveBuffer, "DEL ", 4) == 0) {
                char *key = receiveBuffer + 4;
                if (checkTheKey(head, key) != false) {
                    deleteAtPosition(&head, key);
                    int sendResult = send(acceptSocket, "OK\n", strlen("OK\n"), 0);
                    if (sendResult == SOCKET_ERROR) {
                        printf("Send error: %d\n", WSAGetLastError());
                        break;
                    }
                }
                else {
                    int sendResult = send(acceptSocket, "Key does not exist or is out of range\n", strlen("Key does not exist or is out of range\n"), 0);
                    if (sendResult == SOCKET_ERROR) {
                        printf("Send error: %d\n", WSAGetLastError());
                        break;
                    }
                }
            }
            else if (strcmp(receiveBuffer, "QUIT") == 0) {
                printf("Client requested to close the connection.\n");
                int sendResult = send(acceptSocket, "Goodbye!\n", strlen("Goodbye!\n"), 0);
                if (sendResult == SOCKET_ERROR) {
                    printf("Send error: %d\n", WSAGetLastError());
                    break;
                }
                break;
            }
            else {
                printf("Message from a client: %s\n", receiveBuffer);
                int sendResult = send(acceptSocket, "Unrecognized command.\n", strlen("Unrecognized command.\n"), 0);
                if (sendResult == SOCKET_ERROR) {
                    printf("Send error: %d\n", WSAGetLastError());
                    break;
                }
            }  
        }
        else if (rbyteCount == 0) {
            printf("Client disconnected. \n");
            break;
        }
    }
    closesocket(acceptSocket);
    closesocket(serverSocket);
    WSACleanup();

    return 0;
}

/*char *strremove(char *str, const char *sub) {
    char *p, *q, *r; 
    if (*sub && (q = r = strstr(str, sub)) != NULL) { 
        size_t len = strlen(sub); 
        while ((r = strstr(p = r + len, sub)) != NULL) {
            while (p < r) { 
                *q++ = *p++;
             } 
        } 
        while ((*q++ = *p++) != '\0') {
            continue; 
        }
    } 
    return str; 
}
int sendResult(SOCKET sock, const char *msg) {
    int result = send(sock, msg, strlen(msg), 0);
    if (result == SOCKET_ERROR) {
        printf("Send error: %d\n", WSAGetLastError());
        return -1;
    }
    return 0;
}*/