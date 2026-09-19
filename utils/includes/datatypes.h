#ifndef UTILS_DATATYPES_H
#define UTILS_DATATYPES_H

enum DataType {
    DTYPE_INTEGER,
    DTYPE_FLOAT,
    DTYPE_STRING,
    DTYPE_OBJECT,
    DTYPE_ARRAY,
    DTYPE_BOOL,
    DTYPE_NULL
};

char* datatype__str__(void*, enum DataType);
void* datatype__copy__(void*, enum DataType);
void  datatype__free__(void*, enum DataType);

#endif