#include <stdlib.h>
#include <string.h>

#include "includes/dict.h"
#include "includes/datatypes.h"


unsigned int hash(const char *key) {
    unsigned long hash = 5381;
    int c;
    
    while ((c = *key++)) { hash = ((hash << 5) + hash) + c; }

    return hash % DICT_MAX_STACK;
}


Dict* dict__new__() {
    Dict *object = (Dict*) malloc(sizeof(Dict));
    for (int i=0; i < DICT_MAX_STACK; object->map[i++] = NULL) {}
    return object;
}


DictData dict__get__(Dict* dict, char* key) {
    unsigned int index = hash(key);
  
    for (Item* curr = dict->map[index]; curr; curr = curr->next)
        if (strcmp(curr->data->key, key) == 0)
            return *(curr->data);
    
    return (DictData) {NULL, NULL, DTYPE_NULL};
}


int dict__set__(Dict* dict, DictData data) {
    unsigned int index = hash(data.key);
  
    for (Item* curr = dict->map[index]; curr; curr = curr->next) {
        if (strcmp(curr->data->key, data.key) == 0) {
            void* value_object = datatype__copy__(&data, DTYPE_OBJECT);
            datatype__free__(curr->data, DTYPE_OBJECT);
            curr->data->value = value_object;
            curr->data->type = data.type;
            return 0;
        }
    }
    
    Item* item_object = (Item*) malloc(sizeof(Item));
    DictData* data_object = (DictData*) malloc(sizeof(DictData));
    void* value_object = datatype__copy__(&data, DTYPE_OBJECT);

    data_object->key = strdup(data.key);
    data_object->value = value_object;
    data_object->type = data.type;
    
    item_object->data = data_object;
    item_object->next = dict->map[index];

    dict->map[index] = item_object;
    
    return 0;
}


Item* dict__iter__(Dict* dict, Item* curr) {
    int index = 0;

    if (curr) {
        if (curr->next) return curr->next;
        index = hash(curr->data->key) + 1;
    }

    for (; index < DICT_MAX_STACK; index++)
        if (dict->map[index]) return dict->map[index];
        
    return NULL;
}


char* dict__str__(Dict* dict) {
    size_t capacity = 3;
    char* output = (char*) malloc(capacity);
    output[0] = '\0';
    strcat(output, "{");

    for (Item* curr = NULL; (curr = dict__iter__(dict, curr));) {
        char* entry = datatype__str__(curr->data, DTYPE_OBJECT);

        capacity += strlen(entry) + 2;
        output = (char*) realloc(output, capacity);

        if (!(strcmp("{", output) == 0)) strcat(output, ", ");
        strcat(output, entry);
        free(entry);
    }
    strcat(output, "}");

    return output;
}


void dict__free__(Dict* dict) {

    for (int i=0; i < DICT_MAX_STACK; i++) {
        Item *curr = dict->map[i];

        if (!curr) continue;

        for (Item* _next = curr->next; _next; _next = curr->next) {
            datatype__free__(curr->data, DTYPE_OBJECT);
            free(curr->data->key);
            free(curr->data);
            free(curr);
            curr = _next;
        }

        datatype__free__(curr->data, DTYPE_OBJECT);
        free(curr->data->key);
        free(curr->data);
        free(curr);
    }
    
    free(dict);
}


Dict* dict__copy__(Dict* source) {
    Dict* dest = dict__new__();

    for (Item* curr = NULL; (curr = dict__iter__(source, curr));)
        dict__set__(dest, (DictData) {curr->data->key, curr->data->value, curr->data->type});

    return dest;
}