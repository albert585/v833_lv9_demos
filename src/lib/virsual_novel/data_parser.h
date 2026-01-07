/**
 * @file data_parser.h
 * @brief JSON数据解析器头文件
 *
 * 负责解析JSON配置文件，提取页面信息
 */

#ifndef DATA_PARSER_H
#define DATA_PARSER_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief 文本框配置结构体
 */
typedef struct {
    bool visible;       /**< 是否显示文本框 */
    int x;              /**< X坐标 */
    int y;              /**< Y坐标 */
    int width;          /**< 宽度 */
    int height;         /**< 高度 */
    const char *bg_color;   /**< 背景颜色 */
    const char *text_color; /**< 文字颜色 */
    const char *font;       /**< 字体文件路径 */
    int font_size;      /**< 字体大小 */
} textbox_config_t;

/**
 * @brief 角色配置结构体
 */
typedef struct {
    const char *id;     /**< 角色ID */
    const char *image;  /**< 角色图片路径 */
    int x;              /**< X坐标 */
    int y;              /**< Y坐标 */
    float scale;        /**< 缩放比例 */
    bool visible;       /**< 是否可见 */
} character_config_t;

/**
 * @brief 页面配置结构体
 */
typedef struct {
    const char *id;                 /**< 页面ID */
    const char *background;         /**< 背景图片路径 */
    character_config_t *characters; /**< 角色数组 */
    int character_count;            /**< 角色数量 */
    const char *text;               /**< 页面文字内容 */
    textbox_config_t textbox;       /**< 文本框配置 */
    const char *next_page;          /**< 下一页ID，NULL表示故事结束 */
} page_config_t;

/**
 * @brief 故事配置结构体
 */
typedef struct {
    const char *title;      /**< 故事标题 */
    const char *author;     /**< 作者信息 */
    page_config_t *pages;   /**< 页面数组 */
    int page_count;         /**< 页面数量 */
} story_config_t;

/**
 * @brief 解析JSON配置文件
 * @param json_path JSON文件路径
 * @return 故事配置结构体指针，如果解析失败返回NULL
 */
story_config_t *parse_story_json(const char *json_path);

/**
 * @brief 根据页面ID查找页面配置
 * @param story 故事配置
 * @param page_id 页面ID
 * @return 页面配置结构体指针，如果未找到返回NULL
 */
page_config_t *find_page_by_id(const story_config_t *story, const char *page_id);

/**
 * @brief 释放故事配置
 * @param story 故事配置
 */
void free_story_config(story_config_t *story);

#endif /* DATA_PARSER_H */