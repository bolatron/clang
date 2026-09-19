#ifndef UTILS_LIST_H
#define UTILS_LIST_H

#include <stdint.h>
#include <stdlib.h>

#include "includes/datatypes.h"

typedef struct _ListData {
    void *value;
    enum DataType type;
} ListData;

typedef struct _Node {
    struct _Node *next;
    ListData* data;
} Node;

typedef struct _List { 
    Node* head;
    int32_t size;
} List;

/* METHODS */
int list__append__(List*, ListData);

List* list__new__();
char* list__str__(List*);
Node* list__iter__(List*, Node*);
void  list__free__(List*);
List* list__copy__(List*);

#endif