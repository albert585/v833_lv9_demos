/**
 * @file visual_novel_engine.h
 * @brief 视觉小说引擎头文件
 *
 * 负责管理页面切换、渲染和交互逻辑
 */

#ifndef VISUAL_NOVEL_ENGINE_H
#define VISUAL_NOVEL_ENGINE_H

#include "lvgl/lvgl.h"
#include "data_parser.h"

/**
 * @brief 视觉小说引擎状态枚举
 */
typedef enum {
    VN_ENGINE_STATE_IDLE,      /**< 空闲状态 */
    VN_ENGINE_STATE_INIT,      /**< 初始化状态 */
    VN_ENGINE_STATE_RUNNING,   /**< 运行状态 */
    VN_ENGINE_STATE_PAUSED,    /**< 暂停状态 */
    VN_ENGINE_STATE_FINISHED   /**< 结束状态 */
} vn_engine_state_t;

/**
 * @brief 视觉小说引擎结构体
 */
typedef struct {
    vn_engine_state_t state;        /**< 引擎状态 */
    story_config_t *story;          /**< 故事配置 */
    page_config_t *current_page;    /**< 当前页面 */
    lv_obj_t *screen;               /**< 屏幕对象 */
    lv_obj_t *background_img;       /**< 背景图片对象 */
    lv_obj_t **character_imgs;      /**< 角色图片对象数组 */
    lv_obj_t *textbox_bg;           /**< 文本框背景对象 */
    lv_obj_t *text_label;           /**< 文本标签对象 */
    char *current_page_id;          /**< 当前页面ID */
} vn_engine_t;

/**
 * @brief 初始化视觉小说引擎
 * @param json_path JSON配置文件路径
 * @return 初始化是否成功
 */
bool vn_engine_init(const char *json_path);

/**
 * @brief 启动视觉小说引擎
 */
void vn_engine_start(void);

/**
 * @brief 暂停视觉小说引擎
 */
void vn_engine_pause(void);

/**
 * @brief 恢复视觉小说引擎
 */
void vn_engine_resume(void);

/**
 * @brief 停止视觉小说引擎
 */
void vn_engine_stop(void);

/**
 * @brief 加载指定页面
 * @param page_id 页面ID
 * @return 加载是否成功
 */
bool vn_engine_load_page(const char *page_id);

/**
 * @brief 加载下一页
 * @return 加载是否成功，如果没有下一页返回false
 */
bool vn_engine_load_next_page(void);

/**
 * @brief 获取当前引擎状态
 * @return 引擎状态
 */
vn_engine_state_t vn_engine_get_state(void);

/**
 * @brief 释放当前页面资源
 */
void vn_engine_free_current_resources(void);

/**
 * @brief 反初始化视觉小说引擎
 */
void vn_engine_deinit(void);

#endif /* VISUAL_NOVEL_ENGINE_H */