#ifndef UTILS_DICT_H
#define UTILS_DICT_H

#include "includes/datatypes.h"

#define DICT_MAX_STACK 100

typedef struct _DictData {
    char* key;
    void* value;
    enum DataType type;
} DictData;

typedef struct _Item {
    DictData* data;
    struct _Item* next;
} Item;

typedef struct _Dict {
    Item* map[DICT_MAX_STACK];
} Dict;

unsigned int hash(const char *);


Dict* dict__new__();
int   dict__set__(Dict*, DictData);
DictData dict__get__(Dict*, char*);
char* dict__str__(Dict*);
Item* dict__iter__(Dict*, Item*);
void  dict__free__(Dict*);
Dict* dict__copy__(Dict*);

#endif