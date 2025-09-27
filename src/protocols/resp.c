#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "resp.h"

int sendSimpleString(SOCKET sock, const char *msg) {
    size_t msgLength = 1 + strlen(msg) + 2; //1 for +, 2 for \r\n, we dont send null terminator because of strlen()
    char responseBuffer[msgLength + 1]; //+1 for null terminator

    int len = snprintf(responseBuffer, sizeof(responseBuffer), "+%s\r\n", msg);

    int sendResult = send(sock, responseBuffer, len, 0);
    if (sendResult == SOCKET_ERROR) {
        printf("Send error: %d\n", WSAGetLastError());
        return -1;
    }

    return 0;
}

int sendSimpleError(SOCKET sock, const char *msg) {
    char responseBuffer[512];
    int len = snprintf(responseBuffer, sizeof(responseBuffer), "-%s\r\n", msg);
    
    int sendResult = send(sock, responseBuffer, len, 0);
    if (sendResult == SOCKET_ERROR) {
        printf("Send error: %d\n", WSAGetLastError());
        return -1;
    }
    

    return 0;
}

int sendInteger(SOCKET sock, int val) {
    //Convert int to string 
    char intToString[50];
    snprintf(intToString, sizeof(intToString), "%d", val);

    size_t msgLength = 1 + strlen(intToString) + 2; //1 for :, 2 for \r\n, we dont send null terminator because of strlen()
    char responseBuffer[msgLength];
    
    snprintf(responseBuffer, sizeof(responseBuffer), ":%s\r\n", intToString);

    int sendResult = send(sock, responseBuffer, strlen(responseBuffer), 0);
    if (sendResult == SOCKET_ERROR) {
        printf("Send error: %d\n", WSAGetLastError());
        return -1;
    }

    return 0;
}

int sendBulkString(SOCKET sock, const char *msg) {
    size_t msg_len = strlen(msg);
    char header[32];
    int header_len = snprintf(header, sizeof(header), "$%zu\r\n", msg_len);

    // Send header: "$<len>\r\n"
    if (send(sock, header, header_len, 0) == SOCKET_ERROR) return -1;

    // Send payload: "<msg>"
    if (send(sock, msg, msg_len, 0) == SOCKET_ERROR) return -1;

    // Send trailer: "\r\n"
    if (send(sock, "\r\n", 2, 0) == SOCKET_ERROR) return -1;

    return 0;
}

int sendNull(SOCKET sock) {
    const char *nullResp = "$-1\r\n";
    int sendResult = send(sock, nullResp, strlen(nullResp), 0);
    if (sendResult == SOCKET_ERROR) {
        printf("Send error: %d\n", WSAGetLastError());
        return -1;
    }

    return 0;
}

int sendArray(SOCKET sock, int count, char** elements) {
    //Send the Array Header: *<count>\r\n
    char arrayHeader[64];
    // Use an error specific to Nil Array if count is -1, 
    // but for non-nil arrays, *0 or *N is standard.
    if (count < 0) {
        return send(sock, "*-1\r\n", 5, 0); // Nil Array
    }
    
    int headerLen = snprintf(arrayHeader, sizeof(arrayHeader), "*%d\r\n", count);
    if (send(sock, arrayHeader, headerLen, 0) == SOCKET_ERROR) {
        printf("Send error: %d\n", WSAGetLastError());
        return -1;
    }

    //Loop and send each element as a Bulk String
    for (int i = 0; i < count; i++) {
        // sendBulkString handles the element being NULL
        if (sendBulkString(sock, elements[i]) == -1) {
            printf("Send error during bulk string array element.\n");
            return -1;
        }
    }
    return 0;
}

//Client Functions
int sendCommand(SOCKET sock, int argc, char args[][100]) {

    char numBuffer[100];
    snprintf(numBuffer, sizeof(numBuffer), "*%d\r\n", argc);
    int sendLength = send(sock, numBuffer, strlen(numBuffer), 0);
    if (sendLength == SOCKET_ERROR) {
        printf("Send error: %d\n", WSAGetLastError());
        return -1;
    }

    for (int i = 0; i < argc; i++) {
        const char *arg = args[i];
        int argLen = (int)strlen(arg);

        //Send $<len>\r\n
        char bulkHeader[64];
        int bulkHeaderlen = snprintf(bulkHeader, sizeof(bulkHeader), "$%d\r\n", argLen);
        int sendLen = send(sock, bulkHeader, bulkHeaderlen, 0);
        if (sendLen == SOCKET_ERROR) {
            printf("Send error: %d\n", WSAGetLastError());
            return -1;
        }

        //Send <arg>\r\n
        int sendArg = send(sock, arg, argLen, 0);
        if (sendArg == SOCKET_ERROR) {
            printf("Send error: %d\n", WSAGetLastError());
            return -1;
        }
        if (send(sock, "\r\n", 2, 0) == SOCKET_ERROR) {
            printf("Send error: %d\n", WSAGetLastError());
            return -1;
        }
    }
    
    return 0;
}

int read_n_bytes(SOCKET sock, char *buf, int n) {
    int bytes_read = 0;
    while (bytes_read < n) {
        int result = recv(sock, buf + bytes_read, n - bytes_read, 0);
        if (result <= 0) {
            return -1;
        }
        bytes_read += result;
    }
    return 0;
}

int parseResponse(SOCKET sock) {
    char line[1024];

    if (recvLine(sock, line, sizeof(line)) <= 0) {
        printf("Failed to read response from server.\n");
        return -1;
    }

    // Null-terminate the line by removing \r\n
    line[strcspn(line, "\r\n")] = '\0';
    
    char type = line[0];
    char *payload = line + 1;

    switch (type) {
        case '+': // Simple String
            printf("%s\n", payload);
            break;

        case '-': // Error
            printf("(error) %s\n", payload);
            break;

        case ':': // Integer
            printf("(integer) %s\n", payload);
            break;

        case '$': // Bulk String
            {
                int len = atoi(payload);
                if (len == -1) {
                    printf("(nil)\n");
                } else {
                    char *bulk_str = malloc(len + 1);
                    if (!bulk_str) {
                        return -1; // Out of memory
                    }
                    
                    // Read the string data itself
                    if (read_n_bytes(sock, bulk_str, len) != 0) {
                        free(bulk_str);
                        return -1;
                    }
                    bulk_str[len] = '\0';

                    // Read the trailing \r\n
                    char crlf[2];
                    if (read_n_bytes(sock, crlf, 2) != 0) {
                        free(bulk_str);
                        return -1;
                    }

                    printf("\"%s\"\n", bulk_str);
                    free(bulk_str);
                }
            }
            break;

        case '*': // Array
            {
                int count = atoi(payload);
                printf("(array) %d elements:\n", count);
                for (int i = 0; i < count; i++) {
                    printf("%d) ", i + 1);
                    // Recursively parse each element of the array
                    if (parseResponse(sock) == -1) {
                        return -1;
                    }
                }
            }
            break;

        default:
            printf("Unknown response type: '%c'\n", type);
            return -1;
    }

    return 0;
}

int recvLine(SOCKET sock, char *buf, int max) { //too slow, fix it
    int i = 0;
    while (i < max - 1) {
        int r = recv(sock, &buf[i], 1, 0);
        if (r <= 0) {
            return r;
        }
        if (i > 0 && buf[i-1] == '\r' && buf[i] == '\n') {
            buf[i + 1] = '\0';
            return i + 1;
        }
        i++;
    } 
    buf[max - 1] = '\0';
    return i;
}



int parseArrayFromSocket(SOCKET sock, int *argc, char argv[][100]) { //add dynamically allocated mem
    char line[200];
    // First line *<n>\r\n
    if (recvLine(sock, line, sizeof(line)) <= 0) {
        return -1;
    }
    if (line[0] != '*') {
        return -1;
    }
    *argc = atoi(line + 1);

    for (int i = 0; i < *argc; i++) {
        // Length line $<len>\r\n
        if (recvLine(sock, line, sizeof(line)) <= 0) {
            return -1;
        }
        if (line[0] != '$') {
            return -1;
        }
        int len = atoi(line + 1);
        if (len < 0 || len >= 100) { // Add bounds check for safety
             return -1;
        }

        //Use helper to read the payload
        if (read_n_bytes(sock, argv[i], len) != 0) {
            return -1;
        }
        argv[i][len] = '\0'; // Null-terminate the string

        // read the trailing \r\n
        char crlfBuffer[2];
        if (read_n_bytes(sock, crlfBuffer, 2) != 0) {
            return -1;
        }
        /* Optional: check if crlf_buff is indeed "\r\n"
         if (crlf_buffer[0] != '\r' || crlf_buffer[1] != '\n') {
            return -1;
         }*/
    }
    return 0; 
}