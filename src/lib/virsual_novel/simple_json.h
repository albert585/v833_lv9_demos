#ifndef SIMPLE_JSON_H
#define SIMPLE_JSON_H

#include <stddef.h>
#include <stdbool.h>

/* JSON Types */
#define JSON_TYPE_NULL     0
#define JSON_TYPE_FALSE    1
#define JSON_TYPE_TRUE     2
#define JSON_TYPE_NUMBER   3
#define JSON_TYPE_STRING   4
#define JSON_TYPE_ARRAY    5
#define JSON_TYPE_OBJECT   6

/* JSON Structure */
typedef struct json_value json_value;

struct json_value {
    int type;
    union {
        bool boolean;
        double number;
        char *string;
        struct {
            json_value **items;
            size_t count;
        } array;
        struct {
            char **keys;
            json_value **values;
            size_t count;
        } object;
    } data;
};

/* Parser functions */
json_value *json_parse(const char *json);
void json_free(json_value *value);

/* Accessor functions */
json_value *json_object_get(const json_value *object, const char *key);
json_value *json_array_get(const json_value *array, size_t index);
size_t json_array_length(const json_value *array);

/* Type checking functions */
bool json_is_null(const json_value *value);
bool json_is_boolean(const json_value *value);
bool json_is_number(const json_value *value);
bool json_is_string(const json_value *value);
bool json_is_array(const json_value *value);
bool json_is_object(const json_value *value);

/* Value extraction functions */
bool json_get_boolean(const json_value *value, bool default_value);
double json_get_number(const json_value *value, double default_value);
const char *json_get_string(const json_value *value, const char *default_value);

#endif /* SIMPLE_JSON_H */