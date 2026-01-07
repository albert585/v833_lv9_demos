#include "simple_json.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

static const char *skip_whitespace(const char *json) {
    while (*json && isspace((unsigned char)*json)) {
        json++;
    }
    return json;
}

static const char *parse_string(const char *json, char **result) {
    if (*json != '"') {
        return NULL;
    }
    
    json++;
    const char *start = json;
    size_t length = 0;
    
    while (*json && *json != '"') {
        if (*json == '\\') {
            json++;
            if (!*json) return NULL;
        }
        json++;
        length++;
    }
    
    if (*json != '"') {
        return NULL;
    }
    
    *result = (char *)malloc(length + 1);
    if (!*result) {
        return NULL;
    }
    
    memcpy(*result, start, length);
    (*result)[length] = '\0';
    
    return json + 1;
}

static const char *parse_number(const char *json, double *result) {
    char *end;
    *result = strtod(json, &end);
    if (end == json) {
        return NULL;
    }
    return end;
}

static json_value *parse_value(const char *json, const char **end);

static json_value *parse_array(const char *json, const char **end) {
    json = skip_whitespace(json);
    if (*json != '[') {
        return NULL;
    }
    
    json_value *value = (json_value *)malloc(sizeof(json_value));
    if (!value) {
        return NULL;
    }
    
    value->type = JSON_TYPE_ARRAY;
    value->data.array.items = NULL;
    value->data.array.count = 0;
    
    json = skip_whitespace(json + 1);
    if (*json == ']') {
        *end = json + 1;
        return value;
    }
    
    while (1) {
        const char *item_end;
        json_value *item = parse_value(json, &item_end);
        if (!item) {
            json_free(value);
            return NULL;
        }
        
        value->data.array.items = (json_value **)realloc(
            value->data.array.items,
            sizeof(json_value *) * (value->data.array.count + 1)
        );
        if (!value->data.array.items) {
            json_free(item);
            json_free(value);
            return NULL;
        }
        
        value->data.array.items[value->data.array.count++] = item;
        json = skip_whitespace(item_end);
        
        if (*json == ']') {
            *end = json + 1;
            return value;
        }
        
        if (*json != ',') {
            json_free(value);
            return NULL;
        }
        
        json = skip_whitespace(json + 1);
    }
}

static json_value *parse_object(const char *json, const char **end) {
    json = skip_whitespace(json);
    if (*json != '{') {
        return NULL;
    }
    
    json_value *value = (json_value *)malloc(sizeof(json_value));
    if (!value) {
        return NULL;
    }
    
    value->type = JSON_TYPE_OBJECT;
    value->data.object.keys = NULL;
    value->data.object.values = NULL;
    value->data.object.count = 0;
    
    json = skip_whitespace(json + 1);
    if (*json == '}') {
        *end = json + 1;
        return value;
    }
    
    while (1) {
        char *key;
        json = parse_string(json, &key);
        if (!json) {
            json_free(value);
            return NULL;
        }
        
        json = skip_whitespace(json);
        if (*json != ':') {
            free(key);
            json_free(value);
            return NULL;
        }
        
        const char *value_end;
        json_value *item_value = parse_value(skip_whitespace(json + 1), &value_end);
        if (!item_value) {
            free(key);
            json_free(value);
            return NULL;
        }
        
        value->data.object.keys = (char **)realloc(
            value->data.object.keys,
            sizeof(char *) * (value->data.object.count + 1)
        );
        value->data.object.values = (json_value **)realloc(
            value->data.object.values,
            sizeof(json_value *) * (value->data.object.count + 1)
        );
        
        if (!value->data.object.keys || !value->data.object.values) {
            free(key);
            json_free(item_value);
            json_free(value);
            return NULL;
        }
        
        value->data.object.keys[value->data.object.count] = key;
        value->data.object.values[value->data.object.count] = item_value;
        value->data.object.count++;
        
        json = skip_whitespace(value_end);
        
        if (*json == '}') {
            *end = json + 1;
            return value;
        }
        
        if (*json != ',') {
            json_free(value);
            return NULL;
        }
        
        json = skip_whitespace(json + 1);
    }
}

static json_value *parse_value(const char *json, const char **end) {
    json = skip_whitespace(json);
    if (!*json) {
        return NULL;
    }
    
    json_value *value = (json_value *)malloc(sizeof(json_value));
    if (!value) {
        return NULL;
    }
    
    switch (*json) {
        case '"': {
            char *string;
            const char *next = parse_string(json, &string);
            if (!next) {
                free(value);
                return NULL;
            }
            value->type = JSON_TYPE_STRING;
            value->data.string = string;
            *end = next;
            break;
        }
        
        case '[':
            *end = NULL;
            free(value);
            return parse_array(json, end);
        
        case '{':
            *end = NULL;
            free(value);
            return parse_object(json, end);
        
        case 't':
            if (strncmp(json, "true", 4) == 0) {
                value->type = JSON_TYPE_TRUE;
                value->data.boolean = true;
                *end = json + 4;
            } else {
                free(value);
                return NULL;
            }
            break;
        
        case 'f':
            if (strncmp(json, "false", 5) == 0) {
                value->type = JSON_TYPE_FALSE;
                value->data.boolean = false;
                *end = json + 5;
            } else {
                free(value);
                return NULL;
            }
            break;
        
        case 'n':
            if (strncmp(json, "null", 4) == 0) {
                value->type = JSON_TYPE_NULL;
                *end = json + 4;
            } else {
                free(value);
                return NULL;
            }
            break;
        
        default:
            if (isdigit((unsigned char)*json) || *json == '-') {
                double number;
                const char *next = parse_number(json, &number);
                if (!next) {
                    free(value);
                    return NULL;
                }
                value->type = JSON_TYPE_NUMBER;
                value->data.number = number;
                *end = next;
            } else {
                free(value);
                return NULL;
            }
            break;
    }
    
    return value;
}

json_value *json_parse(const char *json) {
    const char *end;
    json_value *value = parse_value(json, &end);
    if (value) {
        end = skip_whitespace(end);
        if (*end) {
            json_free(value);
            return NULL;
        }
    }
    return value;
}

void json_free(json_value *value) {
    if (!value) {
        return;
    }
    
    switch (value->type) {
        case JSON_TYPE_STRING:
            free(value->data.string);
            break;
        
        case JSON_TYPE_ARRAY:
            for (size_t i = 0; i < value->data.array.count; i++) {
                json_free(value->data.array.items[i]);
            }
            free(value->data.array.items);
            break;
        
        case JSON_TYPE_OBJECT:
            for (size_t i = 0; i < value->data.object.count; i++) {
                free(value->data.object.keys[i]);
                json_free(value->data.object.values[i]);
            }
            free(value->data.object.keys);
            free(value->data.object.values);
            break;
        
        default:
            break;
    }
    
    free(value);
}

json_value *json_object_get(const json_value *object, const char *key) {
    if (!object || object->type != JSON_TYPE_OBJECT) {
        return NULL;
    }
    
    for (size_t i = 0; i < object->data.object.count; i++) {
        if (strcmp(object->data.object.keys[i], key) == 0) {
            return object->data.object.values[i];
        }
    }
    
    return NULL;
}

json_value *json_array_get(const json_value *array, size_t index) {
    if (!array || array->type != JSON_TYPE_ARRAY || index >= array->data.array.count) {
        return NULL;
    }
    
    return array->data.array.items[index];
}

size_t json_array_length(const json_value *array) {
    if (!array || array->type != JSON_TYPE_ARRAY) {
        return 0;
    }
    
    return array->data.array.count;
}

bool json_is_null(const json_value *value) {
    return value && value->type == JSON_TYPE_NULL;
}

bool json_is_boolean(const json_value *value) {
    return value && (value->type == JSON_TYPE_TRUE || value->type == JSON_TYPE_FALSE);
}

bool json_is_number(const json_value *value) {
    return value && value->type == JSON_TYPE_NUMBER;
}

bool json_is_string(const json_value *value) {
    return value && value->type == JSON_TYPE_STRING;
}

bool json_is_array(const json_value *value) {
    return value && value->type == JSON_TYPE_ARRAY;
}

bool json_is_object(const json_value *value) {
    return value && value->type == JSON_TYPE_OBJECT;
}

bool json_get_boolean(const json_value *value, bool default_value) {
    if (!json_is_boolean(value)) {
        return default_value;
    }
    return value->data.boolean;
}

double json_get_number(const json_value *value, double default_value) {
    if (!json_is_number(value)) {
        return default_value;
    }
    return value->data.number;
}

const char *json_get_string(const json_value *value, const char *default_value) {
    if (!json_is_string(value)) {
        return default_value;
    }
    return value->data.string;
}