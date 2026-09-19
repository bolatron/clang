#include "includes/list.h"
#include "includes/datatypes.h"

#include <stdlib.h>
#include <string.h>
#include <stdint.h>


List* list__new__() {
    List* object = (List*) malloc(sizeof(List));
    
    object->head = NULL;
    object->size = 0;
    
    return object;
}


int list__append__(List* list, ListData data) {

    Node* node_object = (Node*) malloc(sizeof(Node));
    ListData* data_object = (ListData*) malloc(sizeof(ListData));
    void* value_object = datatype__copy__(&data, DTYPE_ARRAY);

    data_object->value = value_object;
    data_object->type = data.type;

    node_object->next = NULL;
    node_object->data = data_object;

    if (list->size == 0) {
        list->head = node_object;
        list->size++;
        return 0;
    }

    Node* tmp;
    for (tmp = list->head; tmp->next; tmp = tmp->next) {}
    tmp->next = node_object;
    list->size++;

    return 0;
}


char* list__str__(List* list) {
    size_t capacity = 3;
    char* output = (char*) malloc(capacity);
    output[0] = '\0';
    strcat(output, "[");

    for (Node* curr = NULL; (curr = list__iter__(list, curr));) {
        char* entry = datatype__str__(curr->data, DTYPE_ARRAY);

        capacity += strlen(entry) + 2;
        output = (char*) realloc(output, capacity);

        if (!(strcmp("[", output) == 0)) strcat(output, ", ");
        strcat(output, entry);
        free(entry);
    }
    strcat(output, "]");

    return output;
}


Node* list__iter__(List* list, Node* curr) {
    if (curr) return curr->next;
    return list->head;
}


void list__free__(List* list) {
    Node *curr = list->head;

    if (!curr) {
        free(list);
        return;
    }

    for (Node* _next = curr->next; _next; _next = curr->next) {
        datatype__free__(curr->data, DTYPE_ARRAY);
        free(curr->data);
        free(curr);
        curr = _next;
    }
    
    datatype__free__(curr->data, DTYPE_ARRAY);
    free(curr->data);
    free(curr);

    free(list);
}


List* list__copy__(List* source) {
    List* dest = list__new__();

    for (Node* curr = NULL; (curr = list__iter__(source, curr));)
        list__append__(dest, (ListData) {curr->data->value, curr->data->type});

    return dest;
}