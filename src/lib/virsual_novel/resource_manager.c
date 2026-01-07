/**
 * @file resource_manager.c
 * @brief 资源管理器实现
 *
 * 负责图片资源的加载和释放
 */

#include "resource_manager.h"
#include <string.h>
#include <stdlib.h>

/**
 * @brief 资源节点结构体
 */
typedef struct resource_node {
    resource_t resource;
    struct resource_node *next;
} resource_node_t;

static resource_node_t *resource_list = NULL;  /**< 资源链表 */

/**
 * @brief 查找资源节点
 * @param id 资源ID
 * @return 资源节点，如果未找到返回NULL
 */
static resource_node_t *find_resource_node(const char *id) {
    resource_node_t *node = resource_list;
    while (node != NULL) {
        if (strcmp(node->resource.id, id) == 0) {
            return node;
        }
        node = node->next;
    }
    return NULL;
}

/**
 * @brief 创建新的资源节点
 * @param id 资源ID
 * @param type 资源类型
 * @param data 资源数据
 * @return 资源节点
 */
static resource_node_t *create_resource_node(const char *id, resource_type_t type, void *data) {
    resource_node_t *node = (resource_node_t *)malloc(sizeof(resource_node_t));
    if (node == NULL) {
        return NULL;
    }
    
    node->resource.id = strdup(id);
    node->resource.type = type;
    node->resource.data = data;
    node->resource.ref_count = 1;
    node->next = NULL;
    
    return node;
}

/**
 * @brief 初始化资源管理器
 */
void resource_manager_init(void) {
    resource_list = NULL;
}

/**
 * @brief 加载图片资源
 * @param id 资源ID
 * @param path 图片路径
 * @return 图片对象，如果加载失败返回NULL
 */
lv_obj_t *resource_manager_load_image(const char *id, const char *path) {
    // 查找是否已加载该资源
    resource_node_t *node = find_resource_node(id);
    if (node != NULL) {
        // 增加引用计数
        node->resource.ref_count++;
        return (lv_obj_t *)node->resource.data;
    }
    
    // 创建图片对象
    lv_obj_t *img = lv_img_create(lv_scr_act());
    if (img == NULL) {
        return NULL;
    }
    
    // 设置图片源
    lv_img_set_src(img, path);
    
    // 创建资源节点并添加到链表
    node = create_resource_node(id, RESOURCE_TYPE_IMAGE, img);
    if (node == NULL) {
        lv_obj_del(img);
        return NULL;
    }
    
    // 添加到链表头部
    node->next = resource_list;
    resource_list = node;
    
    return img;
}

/**
 * @brief 加载字体资源
 * @param id 资源ID
 * @param path 字体路径
 * @param size 字体大小
 * @return 字体对象，如果加载失败返回NULL
 */
lv_font_t *resource_manager_load_font(const char *id, const char *path, int size) {
    // 查找是否已加载该资源
    resource_node_t *node = find_resource_node(id);
    if (node != NULL) {
        // 增加引用计数
        node->resource.ref_count++;
        return (lv_font_t *)node->resource.data;
    }
    
    // TODO: 实现字体加载
    // 这里简化处理，实际应用中需要根据LVGL的字体加载机制实现
    lv_font_t *font = NULL;
    
    if (font == NULL) {
        return NULL;
    }
    
    // 创建资源节点并添加到链表
    node = create_resource_node(id, RESOURCE_TYPE_FONT, font);
    if (node == NULL) {
        // 释放字体资源
        return NULL;
    }
    
    // 添加到链表头部
    node->next = resource_list;
    resource_list = node;
    
    return font;
}

/**
 * @brief 增加资源引用计数
 * @param id 资源ID
 */
void resource_manager_ref(const char *id) {
    resource_node_t *node = find_resource_node(id);
    if (node != NULL) {
        node->resource.ref_count++;
    }
}

/**
 * @brief 减少资源引用计数，如果引用计数为0则释放资源
 * @param id 资源ID
 */
void resource_manager_unref(const char *id) {
    resource_node_t *node = find_resource_node(id);
    if (node == NULL) {
        return;
    }
    
    // 减少引用计数
    node->resource.ref_count--;
    
    // 如果引用计数为0，释放资源
    if (node->resource.ref_count <= 0) {
        // 从链表中删除节点
        if (node == resource_list) {
            resource_list = node->next;
        } else {
            resource_node_t *prev = resource_list;
            while (prev != NULL && prev->next != node) {
                prev = prev->next;
            }
            if (prev != NULL) {
                prev->next = node->next;
            }
        }
        
        // 释放资源数据
        if (node->resource.type == RESOURCE_TYPE_IMAGE) {
            lv_obj_del((lv_obj_t *)node->resource.data);
        }
        // TODO: 处理其他类型资源的释放
        
        // 释放资源ID
        free((char *)node->resource.id);
        
        // 释放节点
        free(node);
    }
}

/**
 * @brief 释放所有未使用的资源（引用计数为0的资源）
 */
void resource_manager_free_unused(void) {
    resource_node_t *node = resource_list;
    while (node != NULL) {
        resource_node_t *next = node->next;
        
        if (node->resource.ref_count <= 0) {
            // 从链表中删除节点
            if (node == resource_list) {
                resource_list = node->next;
            } else {
                resource_node_t *prev = resource_list;
                while (prev != NULL && prev->next != node) {
                    prev = prev->next;
                }
                if (prev != NULL) {
                    prev->next = node->next;
                }
            }
            
            // 释放资源数据
            if (node->resource.type == RESOURCE_TYPE_IMAGE) {
                lv_obj_del((lv_obj_t *)node->resource.data);
            }
            // TODO: 处理其他类型资源的释放
            
            // 释放资源ID
            free((char *)node->resource.id);
            
            // 释放节点
            free(node);
        }
        
        node = next;
    }
}

/**
 * @brief 释放所有资源
 */
void resource_manager_free_all(void) {
    resource_node_t *node = resource_list;
    while (node != NULL) {
        resource_node_t *next = node->next;
        
        // 释放资源数据
        if (node->resource.type == RESOURCE_TYPE_IMAGE) {
            lv_obj_del((lv_obj_t *)node->resource.data);
        }
        // TODO: 处理其他类型资源的释放
        
        // 释放资源ID
        free((char *)node->resource.id);
        
        // 释放节点
        free(node);
        
        node = next;
    }
    
    resource_list = NULL;
}

/**
 * @brief 反初始化资源管理器
 */
void resource_manager_deinit(void) {
    resource_manager_free_all();
}