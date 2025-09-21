#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "linlist.h"

//Function to create a new entry
struct Entry* createEntry(char *key, char *value) {
    struct Entry* newEntry = (struct Entry*)malloc(sizeof(struct Entry));
    if (newEntry == NULL) {
        return NULL;
    }
    newEntry->key = (char*)malloc(strlen(key) + 1);
    if (newEntry->key == NULL) {
        free(newEntry);
        return NULL;
    }
    strcpy(newEntry->key, key);
    newEntry->value = (char*)malloc(strlen(value) + 1);
    if (newEntry->value == NULL) {
        free(newEntry->key);
        free(newEntry);
        return NULL;
    }
    strcpy(newEntry->value, value);
    newEntry->next = NULL;
    return newEntry;
}

void freeEntry(struct Entry* entry) {
    free(entry->key);
    free(entry->value);
    free(entry);
}

void insertAtEnd(struct Entry** head, char *key, char *value) {
    struct Entry* newEntry = createEntry(key, value);
    if (*head == NULL) {
        *head = newEntry;
        return;
    }
    struct Entry* temp = *head;
    while(temp->next != NULL) {
        temp = temp->next;
    }
    temp->next = newEntry;
}

void deleteFromFirst(struct Entry** head) {
    if (*head == NULL) {
        printf("List is empty\n");
        return;
    }
    struct Entry* temp = *head;
    *head = temp->next;
    freeEntry(temp);
}

void deleteFromEnd(struct Entry** head) {
    if (*head == NULL) {
        printf("List is empty\n");
        return;
    }
    struct Entry* temp = *head;
    if (temp->next == NULL) {
        freeEntry(temp);
        *head = NULL;
        return;
    }
    while(temp->next->next != NULL) {
        temp = temp->next;
    }
    freeEntry(temp->next);
    temp->next = NULL;
}

// Counts number of nodes in linked list
int countNodes(struct Entry* head) {
    // Initialize count with 0
    int count = 0;

    // Initialize curr with head of Linked List
    struct Entry* curr = head;

    // Traverse till we reach NULL
    while (curr != NULL) {
      
        // Increment count by 1
        count++;
        
      	// Move pointer to next node
        curr = curr->next;
    }
    // Return the count of nodes
    return count;
}

void deleteAtPosition(struct Entry** head, char *key) {
    if (*head == NULL) {
        printf("List is empty\n");
        return;
    }
    struct Entry* temp = *head;
    struct Entry* prev = NULL;
    while (temp != NULL) {
        if (strcmp(temp->key, key) == 0) {
            if (prev == NULL) {
                *head = temp->next;
            }
            else{
                prev->next = temp->next;
            }
            freeEntry(temp);
            return;
        }
        prev = temp;
        temp = temp->next;
    }
    printf("Key not Found\n");
}

char* printFromPosition(struct Entry* head, char *key) {
    struct Entry* temp = head;
    while (temp != NULL) {
        if (strcmp(temp->key, key) == 0) {
            return temp->value;
        }
        temp = temp->next;
    }
    return NULL;
}

bool checkTheKey(struct Entry* head, char *key) {
    struct Entry* temp = head;
    while (temp != NULL) {
        if (strcmp(temp->key, key) == 0) {
            return true;
        }
        temp = temp->next;
    }
    return false;
}