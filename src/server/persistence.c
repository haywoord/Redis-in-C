#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <persistence.h>
#include <linlist.h>

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

int writeToAOF(FILE *fptr, int argc, char argv[][100]) {

    FILE* Firstfptr;
    Firstfptr = fopen("appendonly.aof","ab");
    if (Firstfptr == NULL) {
        printf("File append error.\n");
        return 1;
    }

    fprintf(Firstfptr, "*%d\r\n", argc);
    for (int i = 0; i < argc; i++) {
        size_t len = strlen(argv[i]);
        fprintf(Firstfptr, "$%zu\r\n", len);
        fwrite(argv[i], 1, len, Firstfptr);
        fwrite("\r\n", 1, 2, Firstfptr);
    }
    return 0;
}

int rewriteAOF(const char *fileName, struct Entry *head) {

    FILE *fptr = fopen(fileName, "w");
    if (!fptr) {
        printf("File open error:\n");
        return -1;
    }

    struct Entry *curr = head;
    while (curr) {
        fprintf(fptr, "SET %s %s\n", curr->key, curr->value);
        curr = curr->next;
    }

    fclose(fptr);
    return 0;
}

long getFileSize(const char *filename) {
    FILE *fptr = fopen(filename, "rb");
    if (!fptr) {
        return -1;
    }
    fseek(fptr, 0, SEEK_END);
    long size = ftell(fptr);
    fclose(fptr);
    return size;
}

//RDB functions

int saveSnapShot(const char *fileName, struct Entry *head) {

    FILE *fptr = fopen(fileName, "w");
    if (!fptr) {
        printf("File open error:\n");
        return -1;
    }

    struct Entry *curr = head;
    while (curr) {
        fprintf(fptr, "%s %s\n", curr->key, curr->value);
        curr = curr->next;
    }

    fclose(fptr);
    return 0;
}

int loadSnapShot(const char *filename, struct Entry* head) {
    
    FILE *fptr = fopen(filename,"r");
    if (!fptr) {
        printf("File open/read error\n");
        return -1;
    }

    char key[256], value[256];
    while(fscanf(fptr, "%255s %255s", key, value) == 2) {
        insertAtEnd(&head, key, value);
    }
    
    fclose(fptr);
    return 0;
}
