#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <persistence.h>

int recvLineFile(FILE *f, char *buf, int max) {
    int i = 0, c;
    while (i < max - 1 && (c = fgetc(f)) != EOF) {
        buf[i++] = (char)c;
        if (i > 1 && buf[i-2] == '\r' && buf[i-1] == '\n') {
            buf[i] = '\0';
            return i;
        }
    }
    return (i == 0) ? -1 : i;
}

int parseArrayFromFile(FILE *f, int *argc, char argv[][100]) {
    char line[200];

    // Read "*<n>\r\n"
    if (recvLineFile(f, line, sizeof(line)) <= 0) return -1;
    if (line[0] != '*') return -1;
    *argc = atoi(line + 1);

    for (int i = 0; i < *argc; i++) {
        // Read "$<len>\r\n"
        if (recvLineFile(f, line, sizeof(line)) <= 0) return -1;
        if (line[0] != '$') return -1;
        int len = atoi(line + 1);

        if (len < 0 || len >= 100) return -1;

        // Read <arg>
        if (fread(argv[i], 1, len, f) != (size_t)len) return -1;
        argv[i][len] = '\0';

        // Read trailing \r\n
        char crlf[2];
        if (fread(crlf, 1, 2, f) != 2) return -1;
    }
    return 0;
}

void loadAOF(struct Entry **head) {
    FILE *aof = fopen("appendonly.aof", "rb");
    if (!aof) {
        printf("No AOF found. Starting empty\n");
        return;
    }

    char argv[10][100];
    int argc;

    while(parseArrayFromFile(aof, &argc, argv) == 0) {
        if (argc == 0) {
            continue;
        }
        if (strcmp(argv[0], "SET") == 0 && argc == 3) {
            insertAtEnd(head, argv[1], argv[2]);
        }
        else if (strcmp(argv[0], "DEL") == 0 && argc == 2) {
            deleteAtPosition(head, argv[1]);
        }
     }

    fclose(aof);
}