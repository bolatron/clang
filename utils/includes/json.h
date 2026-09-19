#ifndef UTILS_JSON_H
#define UTILS_JSON_H

#include "dict.h"

typedef struct _JsonFile {
    Dict* content;
} JsonFile;

JsonFile* json__new__(char*);
Dict* json__read__(char*);
int   json__write__(JsonFile*, char*);
char* json__str__(JsonFile*);
int   json__validate__(char*);
void  json__free__(JsonFile*);

#endif