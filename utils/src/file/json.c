#include "includes/json.h"
#include "includes/dict.h"
#include "includes/list.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


static int json__read_file__(char*, char**);
static void json__free_temp_value__(void*, enum DataType);
static const char* json__build_value__(const char*, void**, enum DataType*);


JsonFile* json__new__(char* path) {
    JsonFile* object = (JsonFile*) malloc(sizeof(JsonFile));
    
    Dict* content = json__read__(path);
    object->content = content;
    
    return object;
}


Dict* json__read__(char* path) {
    if (!json__validate__(path)) return NULL;

    char* buffer;
    if (!json__read_file__(path, &buffer)) return NULL;

    void* value;
    enum DataType type;
    const char* end = json__build_value__(buffer, &value, &type);

    free(buffer);

    if (type != DTYPE_OBJECT) {
        if (end) json__free_temp_value__(value, type);
        return NULL;
    }

    return (Dict*) value;
}


int json__write__(JsonFile* object, char* path) {
    FILE* file = fopen(path, "w");
    
    if (file == NULL) {
        printf("Erro ao abrir o arquivo!\n");
        return 0;
    }

    char* dest = dict__str__(object->content);
    fprintf(file, dest);
    
    free(dest);
    fclose(file);

    return 1;
}


void json__free__(JsonFile* object) {
    if(object->content) dict__free__(object->content);
    free(object);
}


char* json__str__(JsonFile* object) {
    return dict__str__(object->content);
}


static const char* json__skip_ws__(const char* c) {
    while (*c == ' ' || *c == '\t' || *c == '\n' || *c == '\r') c++;
    return c;
}


static const char* json__parse_value__(const char* c);


static const char* json__parse_string__(const char* c) {
    if (*c != '"') return NULL;
    c++;

    while (*c != '"') {
        if (*c == '\0') return NULL;

        if (*c == '\\') {
            c++;
            switch (*c) {
                case '"': case '\\': case '/': case 'b':
                case 'f': case 'n': case 'r': case 't':
                    c++;
                    break;
                case 'u':
                    c++;
                    for (int i = 0; i < 4; i++, c++)
                        if (!isxdigit((unsigned char) *c)) return NULL;
                    break;
                default:
                    return NULL;
            }
        } else {
            c++;
        }
    }

    return c + 1;
}


static const char* json__parse_number__(const char* c) {
    const char* start = c;

    if (*c == '-') c++;

    if (*c == '0') {
        c++;
    } else if (isdigit((unsigned char) *c)) {
        while (isdigit((unsigned char) *c)) c++;
    } else {
        return NULL;
    }

    if (*c == '.') {
        c++;
        if (!isdigit((unsigned char) *c)) return NULL;
        while (isdigit((unsigned char) *c)) c++;
    }

    if (*c == 'e' || *c == 'E') {
        c++;
        if (*c == '+' || *c == '-') c++;
        if (!isdigit((unsigned char) *c)) return NULL;
        while (isdigit((unsigned char) *c)) c++;
    }

    return (c == start) ? NULL : c;
}


static const char* json__parse_literal__(const char* c, const char* literal) {
    size_t len = strlen(literal);
    if (strncmp(c, literal, len) != 0) return NULL;
    return c + len;
}


static const char* json__parse_array__(const char* c) {
    c = json__skip_ws__(c + 1);

    if (*c == ']') return c + 1;

    while (1) {
        c = json__parse_value__(c);
        if (!c) return NULL;

        c = json__skip_ws__(c);

        if (*c == ',') {
            c = json__skip_ws__(c + 1);
            continue;
        }

        if (*c == ']') return c + 1;

        return NULL;
    }
}


static const char* json__parse_object__(const char* c) {
    c = json__skip_ws__(c + 1);

    if (*c == '}') return c + 1;

    while (1) {
        c = json__skip_ws__(c);
        c = json__parse_string__(c);
        if (!c) return NULL;

        c = json__skip_ws__(c);
        if (*c != ':') return NULL;

        c = json__parse_value__(c + 1);
        if (!c) return NULL;

        c = json__skip_ws__(c);

        if (*c == ',') {
            c = json__skip_ws__(c + 1);
            continue;
        }

        if (*c == '}') return c + 1;

        return NULL;
    }
}


static const char* json__parse_value__(const char* c) {
    c = json__skip_ws__(c);

    switch (*c) {
        case '{': return json__parse_object__(c);
        case '[': return json__parse_array__(c);
        case '"': return json__parse_string__(c);
        case 't': return json__parse_literal__(c, "true");
        case 'f': return json__parse_literal__(c, "false");
        case 'n': return json__parse_literal__(c, "null");
        default:
            if (*c == '-' || isdigit((unsigned char) *c)) return json__parse_number__(c);
            return NULL;
    }
}


static int json__read_file__(char* path, char** out_buffer) {
    FILE* file = fopen(path, "r");

    if (file == NULL) {
        perror("File does not exists.");
        return 0;
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* buffer = (char*) malloc(size + 1);
    fread(buffer, 1, size, file);
    buffer[size] = '\0';
    fclose(file);

    *out_buffer = buffer;
    return 1;
}


int json__validate__(char* path) {
    char* buffer;
    if (!json__read_file__(path, &buffer)) return 0;

    const char* c = json__parse_value__(buffer);
    int valid = c && (*json__skip_ws__(c) == '\0');

    free(buffer);

    return valid;
}


/* --- value extraction (used by json__read__) --- */


static void json__free_temp_value__(void* value, enum DataType type) {
    switch (type) {
        case DTYPE_STRING:
        case DTYPE_INTEGER:
        case DTYPE_FLOAT:
        case DTYPE_BOOL:
            free(value);
            break;

        case DTYPE_OBJECT:
            dict__free__(value);
            break;

        case DTYPE_ARRAY:
            list__free__(value);
            break;

        case DTYPE_NULL:
        default:
            break;
    }
}


static const char* json__extract_string__(const char* c, char** out) {
    if (*c != '"') return NULL;
    c++;

    size_t capacity = 16;
    size_t len = 0;
    char* buffer = (char*) malloc(capacity);

    while (*c != '"') {
        if (*c == '\0') { free(buffer); return NULL; }

        if (len + 4 >= capacity) {
            capacity *= 2;
            buffer = (char*) realloc(buffer, capacity);
        }

        if (*c != '\\') {
            buffer[len++] = *c++;
            continue;
        }

        c++;
        switch (*c) {
            case '"':  buffer[len++] = '"';  c++; break;
            case '\\': buffer[len++] = '\\'; c++; break;
            case '/':  buffer[len++] = '/';  c++; break;
            case 'b':  buffer[len++] = '\b'; c++; break;
            case 'f':  buffer[len++] = '\f'; c++; break;
            case 'n':  buffer[len++] = '\n'; c++; break;
            case 'r':  buffer[len++] = '\r'; c++; break;
            case 't':  buffer[len++] = '\t'; c++; break;

            case 'u': {
                c++;
                int code = 0;

                for (int i = 0; i < 4; i++, c++) {
                    if (!isxdigit((unsigned char) *c)) { free(buffer); return NULL; }
                    code = code * 16 + (isdigit((unsigned char) *c) ?
                        *c - '0' : tolower((unsigned char) *c) - 'a' + 10);
                }

                /* basic UTF-8 encoding of the BMP codepoint; surrogate
                   pairs are not reassembled */
                if (code < 0x80) {
                    buffer[len++] = (char) code;
                } else if (code < 0x800) {
                    buffer[len++] = (char) (0xC0 | (code >> 6));
                    buffer[len++] = (char) (0x80 | (code & 0x3F));
                } else {
                    buffer[len++] = (char) (0xE0 | (code >> 12));
                    buffer[len++] = (char) (0x80 | ((code >> 6) & 0x3F));
                    buffer[len++] = (char) (0x80 | (code & 0x3F));
                }
                break;
            }

            default:
                free(buffer);
                return NULL;
        }
    }

    buffer[len] = '\0';
    *out = buffer;
    return c + 1;
}


static const char* json__extract_number__(const char* c, void** out, enum DataType* out_type) {
    const char* start = c;
    int is_float = 0;

    if (*c == '-') c++;

    if (*c == '0') {
        c++;
    } else if (isdigit((unsigned char) *c)) {
        while (isdigit((unsigned char) *c)) c++;
    } else {
        return NULL;
    }

    if (*c == '.') {
        is_float = 1;
        c++;
        if (!isdigit((unsigned char) *c)) return NULL;
        while (isdigit((unsigned char) *c)) c++;
    }

    if (*c == 'e' || *c == 'E') {
        is_float = 1;
        c++;
        if (*c == '+' || *c == '-') c++;
        if (!isdigit((unsigned char) *c)) return NULL;
        while (isdigit((unsigned char) *c)) c++;
    }

    if (c == start) return NULL;

    size_t len = (size_t) (c - start);
    char* token = (char*) malloc(len + 1);
    memcpy(token, start, len);
    token[len] = '\0';

    if (is_float) {
        float* value = (float*) malloc(sizeof(float));
        *value = strtof(token, NULL);
        *out = value;
        *out_type = DTYPE_FLOAT;
    } else {
        int* value = (int*) malloc(sizeof(int));
        *value = atoi(token);
        *out = value;
        *out_type = DTYPE_INTEGER;
    }

    free(token);
    return c;
}


static const char* json__build_value__(const char* c, void** out, enum DataType* out_type);


static const char* json__build_object__(const char* c, void** out, enum DataType* out_type) {
    Dict* dict = dict__new__();

    c = json__skip_ws__(c + 1);

    if (*c == '}') {
        *out = dict;
        *out_type = DTYPE_OBJECT;
        return c + 1;
    }

    while (1) {
        c = json__skip_ws__(c);

        char* key;
        c = json__extract_string__(c, &key);
        if (!c) { dict__free__(dict); return NULL; }

        c = json__skip_ws__(c);
        if (*c != ':') { free(key); dict__free__(dict); return NULL; }

        void* value;
        enum DataType value_type;
        c = json__build_value__(json__skip_ws__(c + 1), &value, &value_type);
        if (!c) { free(key); dict__free__(dict); return NULL; }

        dict__set__(dict, (DictData) {key, value, value_type});
        free(key);
        json__free_temp_value__(value, value_type);

        c = json__skip_ws__(c);

        if (*c == ',') {
            c = json__skip_ws__(c + 1);
            continue;
        }

        if (*c == '}') {
            *out = dict;
            *out_type = DTYPE_OBJECT;
            return c + 1;
        }

        dict__free__(dict);
        return NULL;
    }
}


static const char* json__build_array__(const char* c, void** out, enum DataType* out_type) {
    List* list = list__new__();

    c = json__skip_ws__(c + 1);

    if (*c == ']') {
        *out = list;
        *out_type = DTYPE_ARRAY;
        return c + 1;
    }

    while (1) {
        void* value;
        enum DataType value_type;
        c = json__build_value__(c, &value, &value_type);
        if (!c) { json__free_temp_value__(list, DTYPE_ARRAY); return NULL; }

        list__append__(list, (ListData) {value, value_type});
        json__free_temp_value__(value, value_type);

        c = json__skip_ws__(c);

        if (*c == ',') {
            c = json__skip_ws__(c + 1);
            continue;
        }

        if (*c == ']') {
            *out = list;
            *out_type = DTYPE_ARRAY;
            return c + 1;
        }

        json__free_temp_value__(list, DTYPE_ARRAY);
        return NULL;
    }
}


static const char* json__build_value__(const char* c, void** out, enum DataType* out_type) {
    c = json__skip_ws__(c);

    if (*c == '{') return json__build_object__(c, out, out_type);
    if (*c == '[') return json__build_array__(c, out, out_type);

    if (*c == '"') {
        char* value;
        c = json__extract_string__(c, &value);
        if (!c) return NULL;
        *out = value;
        *out_type = DTYPE_STRING;
        return c;
    }

    if (strncmp(c, "true", 4) == 0 || strncmp(c, "false", 5) == 0) {
        int is_true = (*c == 't');
        *out = strdup(is_true ? "true" : "false");
        *out_type = DTYPE_BOOL;
        return c + (is_true ? 4 : 5);
    }

    if (strncmp(c, "null", 4) == 0) {
        *out = NULL;
        *out_type = DTYPE_NULL;
        return c + 4;
    }

    if (*c == '-' || isdigit((unsigned char) *c))
        return json__extract_number__(c, out, out_type);

    return NULL;
}