/**
 * @file resource_manager.h
 * @brief 资源管理器头文件
 *
 * 负责图片资源的加载和释放
 */

#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include "lvgl/lvgl.h"

/**
 * @brief 资源类型枚举
 */
typedef enum {
    RESOURCE_TYPE_IMAGE,  /**< 图片资源 */
    RESOURCE_TYPE_FONT,   /**< 字体资源 */
    RESOURCE_TYPE_AUDIO   /**< 音频资源（预留） */
} resource_type_t;

/**
 * @brief 资源结构体
 */
typedef struct {
    const char *id;           /**< 资源ID */
    resource_type_t type;     /**< 资源类型 */
    void *data;               /**< 资源数据 */
    int ref_count;            /**< 引用计数 */
} resource_t;

/**
 * @brief 初始化资源管理器
 */
void resource_manager_init(void);

/**
 * @brief 加载图片资源
 * @param id 资源ID
 * @param path 图片路径
 * @return 图片对象，如果加载失败返回NULL
 */
lv_obj_t *resource_manager_load_image(const char *id, const char *path);

/**
 * @brief 加载字体资源
 * @param id 资源ID
 * @param path 字体路径
 * @param size 字体大小
 * @return 字体对象，如果加载失败返回NULL
 */
lv_font_t *resource_manager_load_font(const char *id, const char *path, int size);

/**
 * @brief 增加资源引用计数
 * @param id 资源ID
 */
void resource_manager_ref(const char *id);

/**
 * @brief 减少资源引用计数，如果引用计数为0则释放资源
 * @param id 资源ID
 */
void resource_manager_unref(const char *id);

/**
 * @brief 释放所有未使用的资源（引用计数为0的资源）
 */
void resource_manager_free_unused(void);

/**
 * @brief 释放所有资源
 */
void resource_manager_free_all(void);

/**
 * @brief 反初始化资源管理器
 */
void resource_manager_deinit(void);

#endif /* RESOURCE_MANAGER_H */