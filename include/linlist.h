#ifndef LINLIST_H
#define LINLIST_H

struct Entry {
    char *key;
    char *value;
    struct Entry *next;
};

//Function to create a new entry
struct Entry* createEntry(char *key, char *value);

void freeEntry(struct Entry* entry);

void insertAtEnd(struct Entry** head, char *key, char *value);

void deleteFromFirst(struct Entry** head);

void deleteFromEnd(struct Entry** head);

// Counts number of nodes in linked list
int countNodes(struct Entry* head);

void deleteAtPosition(struct Entry** head, char *key);

char* printFromPosition(struct Entry* head, char *key);

bool checkTheKey(struct Entry* head, char *key);

char** listAllKeys(struct Entry* head, int *countOut);
#endif