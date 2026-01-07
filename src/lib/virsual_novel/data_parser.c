/**
 * @file data_parser.c
 * @brief JSON数据解析器实现
 *
 * 负责解析JSON配置文件，提取页面信息
 */

#include "data_parser.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>

// 简单JSON解析器的实现
// 这个解析器仅支持我们需要的JSON格式，不支持完整的JSON规范

/**
 * @brief JSON值类型枚举
 */
typedef enum {
    JSON_TYPE_NULL,
    JSON_TYPE_BOOL,
    JSON_TYPE_NUMBER,
    JSON_TYPE_STRING,
    JSON_TYPE_ARRAY,
    JSON_TYPE_OBJECT
} json_type_t;

/**
 * @brief JSON值结构体
 */
typedef struct json_value {
    json_type_t type;
    union {
        bool boolean;
        double number;
        char *string;
        struct json_array *array;
        struct json_object *object;
    } value;
} json_value_t;

/**
 * @brief JSON数组结构体
 */
typedef struct json_array {
    json_value_t **items;
    int count;
} json_array_t;

/**
 * @brief JSON对象键值对结构体
 */
typedef struct json_pair {
    char *key;
    json_value_t *value;
} json_pair_t;

/**
 * @brief JSON对象结构体
 */
typedef struct json_object {
    json_pair_t **pairs;
    int count;
} json_object_t;

/**
 * @brief 跳过空白字符
 * @param json JSON字符串
 * @return 跳过空白字符后的位置
 */
static const char *skip_whitespace(const char *json) {
    while (*json && isspace(*json)) {
        json++;
    }
    return json;
}

/**
 * @brief 解析JSON字符串
 * @param json JSON字符串
 * @param result 解析结果
 * @return 解析后的位置
 */
static const char *parse_string(const char *json, char **result) {
    json++; // 跳过引号
    
    const char *start = json;
    while (*json && *json != '"') {
        if (*json == '\\') {
            json++; // 跳过转义字符
        }
        json++;
    }
    
    if (*json != '"') {
        return NULL; // 字符串未正确结束
    }
    
    int len = json - start;
    *result = (char *)malloc(len + 1);
    if (*result == NULL) {
        return NULL;
    }
    
    strncpy(*result, start, len);
    (*result)[len] = '\0';
    
    return json + 1; // 跳过结束引号
}

/**
 * @brief 解析JSON数字
 * @param json JSON字符串
 * @param result 解析结果
 * @return 解析后的位置
 */
static const char *parse_number(const char *json, double *result) {
    char *end;
    *result = strtod(json, &end);
    if (end == json) {
        return NULL; // 不是有效的数字
    }
    return end;
}

/**
 * @brief 解析JSON数组
 * @param json JSON字符串
 * @param result 解析结果
 * @return 解析后的位置
 */
static const char *parse_array(const char *json, json_array_t **result);

/**
 * @brief 解析JSON对象
 * @param json JSON字符串
 * @param result 解析结果
 * @return 解析后的位置
 */
static const char *parse_object(const char *json, json_object_t **result);

/**
 * @brief 释放JSON值
 * @param value JSON值
 */
static void free_json_value(json_value_t *value);

/**
 * @brief 解析JSON值
 * @param json JSON字符串
 * @param result 解析结果
 * @return 解析后的位置
 */
static const char *parse_value(const char *json, json_value_t **result) {
    json = skip_whitespace(json);
    if (!*json) {
        return NULL;
    }
    
    *result = (json_value_t *)malloc(sizeof(json_value_t));
    if (*result == NULL) {
        return NULL;
    }
    
    switch (*json) {
        case '{':
            (*result)->type = JSON_TYPE_OBJECT;
            return parse_object(json, &(*result)->value.object);
            
        case '[':
            (*result)->type = JSON_TYPE_ARRAY;
            return parse_array(json, &(*result)->value.array);
            
        case '"':
            (*result)->type = JSON_TYPE_STRING;
            return parse_string(json, &(*result)->value.string);
            
        case 't':
            if (strncmp(json, "true", 4) == 0) {
                (*result)->type = JSON_TYPE_BOOL;
                (*result)->value.boolean = true;
                return json + 4;
            }
            break;
            
        case 'f':
            if (strncmp(json, "false", 5) == 0) {
                (*result)->type = JSON_TYPE_BOOL;
                (*result)->value.boolean = false;
                return json + 5;
            }
            break;
            
        case 'n':
            if (strncmp(json, "null", 4) == 0) {
                (*result)->type = JSON_TYPE_NULL;
                return json + 4;
            }
            break;
            
        default:
            if (isdigit(*json) || *json == '-') {
                (*result)->type = JSON_TYPE_NUMBER;
                return parse_number(json, &(*result)->value.number);
            }
            break;
    }
    
    free(*result);
    return NULL;
}

/**
 * @brief 解析JSON数组
 * @param json JSON字符串
 * @param result 解析结果
 * @return 解析后的位置
 */
static const char *parse_array(const char *json, json_array_t **result) {
    json++; // 跳过 '['
    json = skip_whitespace(json);
    
    *result = (json_array_t *)malloc(sizeof(json_array_t));
    if (*result == NULL) {
        return NULL;
    }
    
    (*result)->items = NULL;
    (*result)->count = 0;
    
    if (*json == ']') {
        return json + 1; // 空数组
    }
    
    while (1) {
        json_value_t *value;
        json = parse_value(json, &value);
        if (!json) {
            free(*result);
            return NULL;
        }
        
        // 扩展数组
        (*result)->items = (json_value_t **)realloc((*result)->items, sizeof(json_value_t *) * ((*result)->count + 1));
        if ((*result)->items == NULL) {
            free(value);
            free(*result);
            return NULL;
        }
        
        (*result)->items[(*result)->count++] = value;
        
        json = skip_whitespace(json);
        if (*json == ']') {
            return json + 1;
        }
        
        if (*json != ',') {
            // 数组项之间必须用逗号分隔
            for (int i = 0; i < (*result)->count; i++) {
                free_json_value((*result)->items[i]);
            }
            free((*result)->items);
            free(*result);
            return NULL;
        }
        
        json++; // 跳过逗号
    }
}

/**
 * @brief 解析JSON对象
 * @param json JSON字符串
 * @param result 解析结果
 * @return 解析后的位置
 */
static const char *parse_object(const char *json, json_object_t **result) {
    json++; // 跳过 '{'
    json = skip_whitespace(json);
    
    *result = (json_object_t *)malloc(sizeof(json_object_t));
    if (*result == NULL) {
        return NULL;
    }
    
    (*result)->pairs = NULL;
    (*result)->count = 0;
    
    if (*json == '}') {
        return json + 1; // 空对象
    }
    
    while (1) {
        json = skip_whitespace(json);
        if (*json != '"') {
            // 对象键必须是字符串
            for (int i = 0; i < (*result)->count; i++) {
                free((*result)->pairs[i]->key);
                free_json_value((*result)->pairs[i]->value);
                free((*result)->pairs[i]);
            }
            free((*result)->pairs);
            free(*result);
            return NULL;
        }
        
        // 解析键
        char *key;
        json = parse_string(json, &key);
        if (!json) {
            for (int i = 0; i < (*result)->count; i++) {
                free((*result)->pairs[i]->key);
                free_json_value((*result)->pairs[i]->value);
                free((*result)->pairs[i]);
            }
            free((*result)->pairs);
            free(*result);
            return NULL;
        }
        
        // 解析冒号
        json = skip_whitespace(json);
        if (*json != ':') {
            free(key);
            for (int i = 0; i < (*result)->count; i++) {
                free((*result)->pairs[i]->key);
                free_json_value((*result)->pairs[i]->value);
                free((*result)->pairs[i]);
            }
            free((*result)->pairs);
            free(*result);
            return NULL;
        }
        json++; // 跳过冒号
        
        // 解析值
        json_value_t *value;
        json = parse_value(json, &value);
        if (!json) {
            free(key);
            for (int i = 0; i < (*result)->count; i++) {
                free((*result)->pairs[i]->key);
                free_json_value((*result)->pairs[i]->value);
                free((*result)->pairs[i]);
            }
            free((*result)->pairs);
            free(*result);
            return NULL;
        }
        
        // 扩展对象
        (*result)->pairs = (json_pair_t **)realloc((*result)->pairs, sizeof(json_pair_t *) * ((*result)->count + 1));
        if ((*result)->pairs == NULL) {
            free(key);
            free(value);
            for (int i = 0; i < (*result)->count; i++) {
                free((*result)->pairs[i]->key);
                free_json_value((*result)->pairs[i]->value);
                free((*result)->pairs[i]);
            }
            free((*result)->pairs);
            free(*result);
            return NULL;
        }
        
        (*result)->pairs[(*result)->count] = (json_pair_t *)malloc(sizeof(json_pair_t));
        if ((*result)->pairs[(*result)->count] == NULL) {
            free(key);
            free(value);
            for (int i = 0; i < (*result)->count; i++) {
                free((*result)->pairs[i]->key);
                free_json_value((*result)->pairs[i]->value);
                free((*result)->pairs[i]);
            }
            free((*result)->pairs);
            free(*result);
            return NULL;
        }
        
        (*result)->pairs[(*result)->count]->key = key;
        (*result)->pairs[(*result)->count]->value = value;
        (*result)->count++;
        
        json = skip_whitespace(json);
        if (*json == '}') {
            return json + 1;
        }
        
        if (*json != ',') {
            // 对象键值对之间必须用逗号分隔
            free(key);
            free(value);
            for (int i = 0; i < (*result)->count; i++) {
                free((*result)->pairs[i]->key);
                free_json_value((*result)->pairs[i]->value);
                free((*result)->pairs[i]);
            }
            free((*result)->pairs);
            free(*result);
            return NULL;
        }
        
        json++; // 跳过逗号
    }
}

/**
 * @brief 从JSON对象中获取字符串值
 * @param object JSON对象
 * @param key 键名
 * @param default_value 默认值
 * @return 字符串值，如果不存在或类型错误则返回默认值
 */
static const char *json_object_get_string(const json_object_t *object, const char *key, const char *default_value) {
    if (!object) {
        return default_value;
    }
    
    for (int i = 0; i < object->count; i++) {
        if (strcmp(object->pairs[i]->key, key) == 0) {
            if (object->pairs[i]->value->type == JSON_TYPE_STRING) {
                return object->pairs[i]->value->value.string;
            }
            break;
        }
    }
    
    return default_value;
}

/**
 * @brief 从JSON对象中获取数字值
 * @param object JSON对象
 * @param key 键名
 * @param default_value 默认值
 * @return 数字值，如果不存在或类型错误则返回默认值
 */
static double json_object_get_number(const json_object_t *object, const char *key, double default_value) {
    if (!object) {
        return default_value;
    }
    
    for (int i = 0; i < object->count; i++) {
        if (strcmp(object->pairs[i]->key, key) == 0) {
            if (object->pairs[i]->value->type == JSON_TYPE_NUMBER) {
                return object->pairs[i]->value->value.number;
            }
            break;
        }
    }
    
    return default_value;
}

/**
 * @brief 从JSON对象中获取布尔值
 * @param object JSON对象
 * @param key 键名
 * @param default_value 默认值
 * @return 布尔值，如果不存在或类型错误则返回默认值
 */
static bool json_object_get_bool(const json_object_t *object, const char *key, bool default_value) {
    if (!object) {
        return default_value;
    }
    
    for (int i = 0; i < object->count; i++) {
        if (strcmp(object->pairs[i]->key, key) == 0) {
            if (object->pairs[i]->value->type == JSON_TYPE_BOOL) {
                return object->pairs[i]->value->value.boolean;
            }
            break;
        }
    }
    
    return default_value;
}

/**
 * @brief 从JSON对象中获取数组
 * @param object JSON对象
 * @param key 键名
 * @return 数组，如果不存在或类型错误则返回NULL
 */
static const json_array_t *json_object_get_array(const json_object_t *object, const char *key) {
    if (!object) {
        return NULL;
    }
    
    for (int i = 0; i < object->count; i++) {
        if (strcmp(object->pairs[i]->key, key) == 0) {
            if (object->pairs[i]->value->type == JSON_TYPE_ARRAY) {
                return object->pairs[i]->value->value.array;
            }
            break;
        }
    }
    
    return NULL;
}

/**
 * @brief 从JSON对象中获取对象
 * @param object JSON对象
 * @param key 键名
 * @return 对象，如果不存在或类型错误则返回NULL
 */
static const json_object_t *json_object_get_object(const json_object_t *object, const char *key) {
    if (!object) {
        return NULL;
    }
    
    for (int i = 0; i < object->count; i++) {
        if (strcmp(object->pairs[i]->key, key) == 0) {
            if (object->pairs[i]->value->type == JSON_TYPE_OBJECT) {
                return object->pairs[i]->value->value.object;
            }
            break;
        }
    }
    
    return NULL;
}

/**
 * @brief 释放JSON值
 * @param value JSON值
 */
static void free_json_value(json_value_t *value) {
    if (!value) {
        return;
    }
    
    switch (value->type) {
        case JSON_TYPE_STRING:
            free(value->value.string);
            break;
            
        case JSON_TYPE_ARRAY:
            if (value->value.array) {
                for (int i = 0; i < value->value.array->count; i++) {
                    free_json_value(value->value.array->items[i]);
                }
                free(value->value.array->items);
                free(value->value.array);
            }
            break;
            
        case JSON_TYPE_OBJECT:
            if (value->value.object) {
                for (int i = 0; i < value->value.object->count; i++) {
                    free(value->value.object->pairs[i]->key);
                    free_json_value(value->value.object->pairs[i]->value);
                    free(value->value.object->pairs[i]);
                }
                free(value->value.object->pairs);
                free(value->value.object);
            }
            break;
            
        default:
            break;
    }
    
    free(value);
}

/**
 * @brief 读取文件内容
 * @param path 文件路径
 * @param size 文件大小（输出）
 * @return 文件内容，如果读取失败返回NULL
 */
static char *read_file(const char *path, size_t *size) {
    FILE *file = fopen(path, "rb");
    if (!file) {
        return NULL;
    }
    
    // 获取文件大小
    fseek(file, 0, SEEK_END);
    *size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    // 分配内存
    char *content = (char *)malloc(*size + 1);
    if (!content) {
        fclose(file);
        return NULL;
    }
    
    // 读取文件内容
    size_t read_size = fread(content, 1, *size, file);
    if (read_size != *size) {
        free(content);
        fclose(file);
        return NULL;
    }
    
    content[*size] = '\0'; // 确保字符串结束
    
    fclose(file);
    return content;
}

/**
 * @brief 解析JSON配置文件
 * @param json_path JSON文件路径
 * @return 故事配置结构体指针，如果解析失败返回NULL
 */
story_config_t *parse_story_json(const char *json_path) {
    // 读取JSON文件
    size_t json_size;
    char *json_content = read_file(json_path, &json_size);
    if (!json_content) {
        printf("无法读取JSON文件: %s\n", json_path);
        return NULL;
    }
    
    // 解析JSON
    json_value_t *json_value;
    const char *json_end = parse_value(json_content, &json_value);
    if (!json_end || json_value->type != JSON_TYPE_OBJECT) {
        printf("JSON解析失败\n");
        free(json_content);
        if (json_value) {
            free_json_value(json_value);
        }
        return NULL;
    }
    
    json_object_t *root = json_value->value.object;
    
    // 分配故事配置结构体
    story_config_t *story = (story_config_t *)malloc(sizeof(story_config_t));
    if (story == NULL) {
        free(json_content);
        free_json_value(json_value);
        return NULL;
    }
    
    // 设置故事基本信息
    story->title = strdup(json_object_get_string(root, "title", "未命名故事"));
    story->author = strdup(json_object_get_string(root, "author", "未知作者"));
    
    // 获取页面数组
    const json_array_t *pages_array = json_object_get_array(root, "pages");
    if (!pages_array) {
        printf("找不到pages数组\n");
        free((char *)story->title);
        free((char *)story->author);
        free(story);
        free(json_content);
        free_json_value(json_value);
        return NULL;
    }
    
    // 分配页面数组
    story->page_count = pages_array->count;
    story->pages = (page_config_t *)malloc(sizeof(page_config_t) * story->page_count);
    if (story->pages == NULL) {
        free((char *)story->title);
        free((char *)story->author);
        free(story);
        free(json_content);
        free_json_value(json_value);
        return NULL;
    }
    
    // 解析每个页面
    for (int i = 0; i < story->page_count; i++) {
        if (pages_array->items[i]->type != JSON_TYPE_OBJECT) {
            printf("页面 %d 不是有效的对象\n", i);
            // 释放已分配的资源
            for (int j = 0; j < i; j++) {
                if (story->pages[j].characters != NULL) {
                    free(story->pages[j].characters);
                }
                free((char *)story->pages[j].id);
                free((char *)story->pages[j].background);
                free((char *)story->pages[j].text);
                if (story->pages[j].next_page) {
                    free((char *)story->pages[j].next_page);
                }
            }
            free(story->pages);
            free((char *)story->title);
            free((char *)story->author);
            free(story);
            free(json_content);
            free_json_value(json_value);
            return NULL;
        }
        
        json_object_t *page_obj = pages_array->items[i]->value.object;
        
        // 设置页面基本信息
        story->pages[i].id = strdup(json_object_get_string(page_obj, "id", ""));
        story->pages[i].background = strdup(json_object_get_string(page_obj, "background", ""));
        story->pages[i].text = strdup(json_object_get_string(page_obj, "text", ""));
        story->pages[i].next_page = NULL;
        
        const char *next_page = json_object_get_string(page_obj, "next_page", NULL);
        if (next_page && strcmp(next_page, "null") != 0) {
            story->pages[i].next_page = strdup(next_page);
        }
        
        // 解析文本框配置
        const json_object_t *textbox_obj = json_object_get_object(page_obj, "textbox");
        if (textbox_obj) {
            story->pages[i].textbox.visible = json_object_get_bool(textbox_obj, "visible", true);
            story->pages[i].textbox.x = (int)json_object_get_number(textbox_obj, "x", 50);
            story->pages[i].textbox.y = (int)json_object_get_number(textbox_obj, "y", 400);
            story->pages[i].textbox.width = (int)json_object_get_number(textbox_obj, "width", 700);
            story->pages[i].textbox.height = (int)json_object_get_number(textbox_obj, "height", 150);
            story->pages[i].textbox.bg_color = strdup(json_object_get_string(textbox_obj, "bg_color", "#000000"));
            story->pages[i].textbox.text_color = strdup(json_object_get_string(textbox_obj, "text_color", "#FFFFFF"));
            story->pages[i].textbox.font = NULL;
            
            const char *font = json_object_get_string(textbox_obj, "font", NULL);
            if (font) {
                story->pages[i].textbox.font = strdup(font);
            }
            
            story->pages[i].textbox.font_size = (int)json_object_get_number(textbox_obj, "font_size", 16);
        } else {
            // 默认文本框配置
            story->pages[i].textbox.visible = true;
            story->pages[i].textbox.x = 50;
            story->pages[i].textbox.y = 400;
            story->pages[i].textbox.width = 700;
            story->pages[i].textbox.height = 150;
            story->pages[i].textbox.bg_color = strdup("#000000");
            story->pages[i].textbox.text_color = strdup("#FFFFFF");
            story->pages[i].textbox.font = NULL;
            story->pages[i].textbox.font_size = 16;
        }
        
        // 解析角色数组
        const json_array_t *characters_array = json_object_get_array(page_obj, "characters");
        if (characters_array) {
            story->pages[i].character_count = characters_array->count;
            story->pages[i].characters = (character_config_t *)malloc(sizeof(character_config_t) * story->pages[i].character_count);
            if (story->pages[i].characters == NULL) {
                // 释放已分配的资源
                for (int j = 0; j <= i; j++) {
                    if (story->pages[j].characters != NULL) {
                        free(story->pages[j].characters);
                    }
                    free((char *)story->pages[j].id);
                    free((char *)story->pages[j].background);
                    free((char *)story->pages[j].text);
                    if (story->pages[j].next_page) {
                        free((char *)story->pages[j].next_page);
                    }
                    free((char *)story->pages[j].textbox.bg_color);
                    free((char *)story->pages[j].textbox.text_color);
                    if (story->pages[j].textbox.font) {
                        free((char *)story->pages[j].textbox.font);
                    }
                }
                free(story->pages);
                free((char *)story->title);
                free((char *)story->author);
                free(story);
                free(json_content);
                free_json_value(json_value);
                return NULL;
            }
            
            // 解析每个角色
            for (int j = 0; j < story->pages[i].character_count; j++) {
                if (characters_array->items[j]->type != JSON_TYPE_OBJECT) {
                    printf("角色 %d 不是有效的对象\n", j);
                    // 释放已分配的资源
                    free(story->pages[i].characters);
                    for (int k = 0; k <= i; k++) {
                        if (k < i && story->pages[k].characters != NULL) {
                            free(story->pages[k].characters);
                        }
                        free((char *)story->pages[k].id);
                        free((char *)story->pages[k].background);
                        free((char *)story->pages[k].text);
                        if (story->pages[k].next_page) {
                            free((char *)story->pages[k].next_page);
                        }
                        free((char *)story->pages[k].textbox.bg_color);
                        free((char *)story->pages[k].textbox.text_color);
                        if (story->pages[k].textbox.font) {
                            free((char *)story->pages[k].textbox.font);
                        }
                    }
                    free(story->pages);
                    free((char *)story->title);
                    free((char *)story->author);
                    free(story);
                    free(json_content);
                    free_json_value(json_value);
                    return NULL;
                }
                
                json_object_t *char_obj = characters_array->items[j]->value.object;
                
                // 设置角色信息
                story->pages[i].characters[j].id = strdup(json_object_get_string(char_obj, "id", ""));
                story->pages[i].characters[j].image = strdup(json_object_get_string(char_obj, "image", ""));
                story->pages[i].characters[j].x = (int)json_object_get_number(char_obj, "x", 0);
                story->pages[i].characters[j].y = (int)json_object_get_number(char_obj, "y", 0);
                story->pages[i].characters[j].scale = (float)json_object_get_number(char_obj, "scale", 1.0);
                story->pages[i].characters[j].visible = json_object_get_bool(char_obj, "visible", true);
            }
        } else {
            story->pages[i].character_count = 0;
            story->pages[i].characters = NULL;
        }
    }
    
    // 释放JSON内容和解析结果
    free(json_content);
    free_json_value(json_value);
    
    return story;
}

/**
 * @brief 根据页面ID查找页面配置
 * @param story 故事配置
 * @param page_id 页面ID
 * @return 页面配置结构体指针，如果未找到返回NULL
 */
page_config_t *find_page_by_id(const story_config_t *story, const char *page_id) {
    if (story == NULL || page_id == NULL) {
        return NULL;
    }
    
    for (int i = 0; i < story->page_count; i++) {
        if (strcmp(story->pages[i].id, page_id) == 0) {
            return &story->pages[i];
        }
    }
    
    return NULL;
}

/**
 * @brief 释放故事配置
 * @param story 故事配置
 */
void free_story_config(story_config_t *story) {
    if (story == NULL) {
        return;
    }
    
    // 释放故事基本信息
    if (story->title) {
        free((char *)story->title);
    }
    if (story->author) {
        free((char *)story->author);
    }
    
    // 释放每个页面的配置
    for (int i = 0; i < story->page_count; i++) {
        // 释放页面基本信息
        if (story->pages[i].id) {
            free((char *)story->pages[i].id);
        }
        if (story->pages[i].background) {
            free((char *)story->pages[i].background);
        }
        if (story->pages[i].text) {
            free((char *)story->pages[i].text);
        }
        if (story->pages[i].next_page) {
            free((char *)story->pages[i].next_page);
        }
        
        // 释放文本框配置
        if (story->pages[i].textbox.bg_color) {
            free((char *)story->pages[i].textbox.bg_color);
        }
        if (story->pages[i].textbox.text_color) {
            free((char *)story->pages[i].textbox.text_color);
        }
        if (story->pages[i].textbox.font) {
            free((char *)story->pages[i].textbox.font);
        }
        
        // 释放角色配置
        if (story->pages[i].characters != NULL) {
            for (int j = 0; j < story->pages[i].character_count; j++) {
                if (story->pages[i].characters[j].id) {
                    free((char *)story->pages[i].characters[j].id);
                }
                if (story->pages[i].characters[j].image) {
                    free((char *)story->pages[i].characters[j].image);
                }
            }
            free(story->pages[i].characters);
        }
    }
    
    // 释放页面数组
    free(story->pages);
    
    // 释放故事配置
    free(story);
}