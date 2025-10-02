#include <stdio.h>
#include <resp.h>
#include <string.h>
#include <linlist.h>
#include <stdlib.h>
#ifndef PERS_H
#define PERS_H

int recvLineFile(FILE *f, char *buf, int max);

int parseArrayFromFile(FILE *f, int *argc, char argv[][100]);

void loadAOF(struct Entry **head);

#endif