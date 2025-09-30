#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <stdlib.h>
#include <stdbool.h>
#include "linlist.h"
#include "resp.h"
#define PORT 6379

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
        int argc = 0;
        char argv[10][100]; // Max 10 arguments, 100 chars each, later add dynamic memory alloc

        //Use RESP parser
        int parse_result = parseArrayFromSocket(acceptSocket, &argc, argv);
        if (parse_result == -1) {
            printf("Client disconnected or sent invalid data.\n");   // This could be a client disconnect or a protocol error
            break;
        }
        if (argc == 0) { // No command parsed, continue waiting
            continue;
        }

        // Process the parsed command from argv[0]
        printf("Received command: %s\n", argv[0]);
        if (strcmp(argv[0], "PING") == 0) {
            sendSimpleString(acceptSocket, "PONG");
        }
        else if (strcmp(argv[0], "ECHO") == 0) {
            if (argc != 2) {
                sendSimpleError(acceptSocket, "ERR wrong number of arguments for 'ECHO' command");
            }
            else {
                sendBulkString(acceptSocket, argv[1]);
            }
        }
        else if (strcmp(argv[0], "SET") == 0) {
            if (argc != 3) {
                sendSimpleError(acceptSocket, "ERR wrong number of arguments for 'SET' command");
            }
            else {
                char *key = argv[1];
                char *value = argv[2];

                if (value != NULL) {
                    if (checkTheKey(head, key) == true) {
                        const char *errorMsg = "The key is already in use";
                        int sendErr = sendSimpleError(acceptSocket, errorMsg);
                        if (sendErr == SOCKET_ERROR) {
                            printf("Send error: %d\n", WSAGetLastError());
                            break;
                        }
                    }
                    else {
                        insertAtEnd(&head, key, value);
                        const char *okMsg = "OK";
                        int sendResult = sendSimpleString(acceptSocket, okMsg);
                        if (sendResult == SOCKET_ERROR) {
                            printf("Send error: %d\n", WSAGetLastError());
                            break;
                        }
                    }
                }
                else {
                    const char *errMsg = "Invalid SET command";
                    int sendErr = sendSimpleError(acceptSocket, errMsg);
                    if (sendErr == SOCKET_ERROR) {
                        printf("Send error: %d\n", WSAGetLastError());
                        break;
                    }
                }
            }
        }
        else if (strcmp(argv[0], "GET") == 0) {
            if (argc != 2) {
                const char *errorMsg = "ERR wrong number of arguments for 'GET' command";
                sendSimpleError(acceptSocket, errorMsg);
            }
            else {
                char *key = argv[1];
                char *result = printFromPosition(head, key);
                if (result != NULL) {
                    sendBulkString(acceptSocket, result);
                }
                else {
                    sendNull(acceptSocket); // Send a RESP Null for a non-existent key
                }
            }
        }
        else if (strcmp(argv[0], "QUIT") == 0) {
            printf("Client requested QUIT.\n");
            sendSimpleString(acceptSocket, "OK");
            break;
        }
        else if (strcmp(argv[0], "DEL") == 0) {
            char *key = argv[1];
            if (argc != 2) {
                const char *errorMsg = "ERR wrong number of arguments for 'DEL' command";
                sendSimpleError(acceptSocket, errorMsg);
            }
            else if (checkTheKey(head, key) != false) {
                deleteAtPosition(&head, key);
                const char *okMsg = "OK";
                sendSimpleString(acceptSocket, okMsg);
            }
            else {
                const char *errorMsg = "Key has not been found";
                sendSimpleError(acceptSocket, errorMsg);
            }
        }
        else if (strcmp(argv[0], "KEYS") == 0) {
            int keyCount;
            char **keys = listAllKeys(head, &keyCount);
            if (keys == NULL) {
                const char *errorMsg = "ERR no key exists";
                sendSimpleError(acceptSocket, errorMsg);
            }
            else if (argc != 1) {
                const char *errorMsg = "ERR wrong number of arguments for 'KEYS' command";
                sendSimpleError(acceptSocket, errorMsg);
            }
            else if (keys != NULL) {
                sendArray(acceptSocket, keyCount, keys);
            }
            free(keys);
        }
        else if (strcmp(argv[0], "KEY") == 0) {
            if (argc != 2) {
                const char *errorMsg = "ERR wrong number of arguments for 'KEY' command";
                sendSimpleError(acceptSocket, errorMsg);
            }
            else {
                if (checkTheKey(head, argv[1])) {
                    const char *returnMsg = "The key exists";
                    sendSimpleString(acceptSocket, returnMsg);
                }
                else {
                    const char *returnMsg = "The key does not exist";
                    sendSimpleString(acceptSocket, returnMsg);
                }
            }
        }
        else {
            // Unrecognized command
            char errorMsg[200];
            snprintf(errorMsg, sizeof(errorMsg), "ERR unknown command '%s'", argv[0]);
            sendSimpleError(acceptSocket, errorMsg);
        }
    }

    closesocket(acceptSocket);
    closesocket(serverSocket);
    WSACleanup();

    return 0;
}