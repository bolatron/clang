#include <stdio.h>
#include <string.h>

#include "includes/dict.h"
#include "includes/list.h"


char* datatype__str__(void* _data, enum DataType dtype) {
    
    void* value;
    char* key = NULL;
    enum DataType type;

    if (dtype == DTYPE_ARRAY) {
        ListData* data = (ListData*) _data;
        value = data->value;
        type  = data->type;
    }

    if (dtype == DTYPE_OBJECT) {
        DictData* data = (DictData*) _data;
        key = data->key;
        value = data->value;
        type  = data->type;
    }

    /* size of the "'key': " prefix printed ahead of OBJECT values; 0 for
       ARRAY values, where `key` is unset */
    size_t key_len = key ? strlen(key) : 0;

    char* output;

    switch (type) {

        case DTYPE_INTEGER: {
            output = (char*) malloc(key_len + 32);
            if (dtype == DTYPE_ARRAY) sprintf(output, "%d", *((int*) value));
            if (dtype == DTYPE_OBJECT) sprintf(output, "\'%s\': %d", key, *((int*) value));
            break;
        }

        case DTYPE_FLOAT: {
            output = (char*) malloc(key_len + 64);
            if (dtype == DTYPE_ARRAY) sprintf(output, "%f", *((float*) value));
            if (dtype == DTYPE_OBJECT) sprintf(output, "\'%s\': %f", key, *((float*) value));
            break;
        }

        case DTYPE_STRING: {
            output = (char*) malloc(key_len + strlen((char*) value) + 8);
            if (dtype == DTYPE_ARRAY) sprintf(output, "\'%s\'", (char*) value);
            if (dtype == DTYPE_OBJECT) sprintf(output, "\'%s\': \'%s\'", key, (char*) value);
            break;
        }

        case DTYPE_OBJECT: {
            char* nested = dict__str__(value);
            output = (char*) malloc(key_len + strlen(nested) + 8);
            if (dtype == DTYPE_ARRAY) sprintf(output, "%s", nested);
            if (dtype == DTYPE_OBJECT) sprintf(output, "\'%s\': %s", key, nested);
            free(nested);
            break;
        }

        case DTYPE_ARRAY: {
            char* nested = list__str__(value);
            output = (char*) malloc(key_len + strlen(nested) + 8);
            if (dtype == DTYPE_ARRAY) sprintf(output, "%s", nested);
            if (dtype == DTYPE_OBJECT) sprintf(output, "\'%s\': %s", key, nested);
            free(nested);
            break;
        }

        case DTYPE_BOOL: {
            output = (char*) malloc(key_len + strlen((char*) value) + 8);
            if (dtype == DTYPE_ARRAY) sprintf(output, "%s", (char*) value);
            if (dtype == DTYPE_OBJECT) sprintf(output, "\'%s\': %s", key, (char*) value);
            break;
        }
        
        case DTYPE_NULL: {
            output = (char*) malloc(key_len + 12);
            if (dtype == DTYPE_ARRAY) sprintf(output, "null");
            if (dtype == DTYPE_OBJECT) sprintf(output, "\'%s\': null", key);
            break;
        }
        
        default:
            output = strdup("");
            break;
    }

    return output;
}


void* datatype__copy__(void* _data, enum DataType dtype) {

    void* value;
    enum DataType type;

    if (dtype == DTYPE_ARRAY) {
        ListData* data = (ListData*) _data;
        value = data->value;
        type  = data->type;
    }
    
    if (dtype == DTYPE_OBJECT) {
        DictData* data = (DictData*) _data;
        value = data->value;
        type  = data->type;
    }

    void *dest = NULL;

    if (type == DTYPE_INTEGER) {
        dest = (int*) malloc(sizeof(int));
        memcpy(dest, value, sizeof(int));
    }

    if (type == DTYPE_STRING) {
        dest = (char*) malloc(strlen((char*) value) + 1);
        memcpy(dest, value, strlen((char*) value) + 1);
    }

    if (type == DTYPE_FLOAT) {
        dest = (float*) malloc(sizeof(float));
        memcpy(dest, value, sizeof(float));
    }
    
    if (type == DTYPE_BOOL) {
        dest = (char*) malloc(strlen((char*) value) + 1);
        memcpy(dest, value, strlen((char*) value) + 1);
    }

    if (type == DTYPE_OBJECT)
        dest = dict__copy__(value);

    if (type == DTYPE_ARRAY)
        dest = list__copy__(value);

    return dest;
}


void datatype__free__(void* _data, enum DataType dtype) {

    void* value;
    enum DataType type;

    if (dtype == DTYPE_ARRAY) {
        ListData* data = (ListData*) _data;
        value = data->value;
        type  = data->type;
    }
    
    if (dtype == DTYPE_OBJECT) {
        DictData* data = (DictData*) _data;
        value = data->value;
        type  = data->type;
    }

    if (type == DTYPE_OBJECT) {
        dict__free__(value);
        return;
    }

    if (type == DTYPE_ARRAY) {
        list__free__(value);
        return;
    }
    
    if (type == DTYPE_NULL)
        return;
    
    free(value);
}