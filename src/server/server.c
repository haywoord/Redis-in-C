#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <stdlib.h>
#include <sys/types.h>
#define PORT 8080
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
    }

    //Server details
    struct sockaddr_in server;
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons(8080); //Default Redis port - 6379, change it later
    
    //Conect to server
    if(connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        printf("Connection failed with error code: %d\n", WSAGetLastError());
        return 1;
    }

    printf("Connected to a server! \n");
    // The Winsock DLL is acceptable. Proceed to use it. 
    // then call WSACleanup when done using the Winsock dll 
    closesocket(sock);
    WSACleanup();
    return 0;
}