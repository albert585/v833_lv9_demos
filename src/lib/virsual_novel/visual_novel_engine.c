/**
 * @file visual_novel_engine.c
 * @brief 视觉小说引擎实现
 *
 * 负责管理页面切换、渲染和交互逻辑
 */

#include "visual_novel_engine.h"
#include "resource_manager.h"
#include "data_parser.h"
#include <stdlib.h>
#include <string.h>

static vn_engine_t engine;  /**< 视觉小说引擎实例 */

/**
 * @brief 更新文本框
 * @param text 文本内容
 * @param textbox 文本框配置
 */
static void update_textbox(const char *text, const textbox_config_t *textbox) {
    if (textbox == NULL) {
        return;
    }

    // 如果文本框不可见，隐藏文本框
    if (!textbox->visible) {
        if (engine.textbox_bg != NULL) {
            lv_obj_add_flag(engine.textbox_bg, LV_OBJ_FLAG_HIDDEN);
        }
        if (engine.text_label != NULL) {
            lv_obj_add_flag(engine.text_label, LV_OBJ_FLAG_HIDDEN);
        }
        return;
    }

    // 如果文本框背景不存在，创建文本框背景
    if (engine.textbox_bg == NULL) {
        engine.textbox_bg = lv_obj_create(engine.screen);
        if (engine.textbox_bg == NULL) {
            return;
        }
    }

    // 设置文本框背景属性
    lv_obj_set_size(engine.textbox_bg, textbox->width, textbox->height);
    lv_obj_set_pos(engine.textbox_bg, textbox->x, textbox->y);
    lv_obj_set_style_bg_color(engine.textbox_bg, lv_color_hex(0x000000), 0);
    lv_obj_set_style_radius(engine.textbox_bg, 10, 0);
    lv_obj_set_style_bg_opa(engine.textbox_bg, 180, 0);  // 设置半透明
    lv_obj_clear_flag(engine.textbox_bg, LV_OBJ_FLAG_HIDDEN);

    // 如果文本标签不存在，创建文本标签
    if (engine.text_label == NULL) {
        engine.text_label = lv_label_create(engine.textbox_bg);
        if (engine.text_label == NULL) {
            return;
        }
    }

    // 设置文本标签属性
    lv_obj_set_size(engine.text_label, textbox->width - 20, textbox->height - 20);
    lv_obj_set_pos(engine.text_label, 10, 10);
    lv_label_set_text(engine.text_label, text);
    lv_obj_set_style_text_color(engine.text_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(engine.text_label, NULL, 0);
    lv_label_set_long_mode(engine.text_label, LV_LABEL_LONG_WRAP);
    lv_obj_clear_flag(engine.text_label, LV_OBJ_FLAG_HIDDEN);
}

/**
 * @brief 屏幕点击事件回调函数
 * @param e 事件对象
 */
static void screen_click_event_handler(lv_event_t *e) {
    // 如果引擎不在运行状态，不处理点击事件
    if (engine.state != VN_ENGINE_STATE_RUNNING) {
        return;
    }
    
    // 加载下一页
    vn_engine_load_next_page();
}

/**
 * @brief 初始化视觉小说引擎
 * @param json_path JSON配置文件路径
 * @return 初始化是否成功
 */
bool vn_engine_init(const char *json_path) {
    // 初始化引擎状态
    memset(&engine, 0, sizeof(vn_engine_t));
    engine.state = VN_ENGINE_STATE_INIT;
    
    // 初始化资源管理器
    resource_manager_init();
    
    // 解析JSON配置文件
    engine.story = parse_story_json(json_path);
    if (engine.story == NULL) {
        engine.state = VN_ENGINE_STATE_IDLE;
        return false;
    }
    
    // 获取屏幕对象
    engine.screen = lv_scr_act();
    if (engine.screen == NULL) {
        free_story_config(engine.story);
        engine.state = VN_ENGINE_STATE_IDLE;
        return false;
    }
    
    // 注册屏幕点击事件
    lv_obj_add_event_cb(engine.screen, screen_click_event_handler, LV_EVENT_CLICKED, NULL);
    
    // 创建背景图片对象
    engine.background_img = lv_img_create(engine.screen);
    if (engine.background_img == NULL) {
        free_story_config(engine.story);
        engine.state = VN_ENGINE_STATE_IDLE;
        return false;
    }
    
    // 设置背景图片大小为屏幕大小
    lv_obj_set_size(engine.background_img, lv_obj_get_width(engine.screen), lv_obj_get_height(engine.screen));
    lv_obj_set_pos(engine.background_img, 0, 0);
    lv_obj_set_style_bg_color(engine.screen, lv_color_hex(0x000000), 0);
    
    // 初始化完成，设置状态为空闲
    engine.state = VN_ENGINE_STATE_IDLE;
    
    return true;
}

/**
 * @brief 启动视觉小说引擎
 */
void vn_engine_start(void) {
    // 如果引擎未初始化，直接返回
    if (engine.state == VN_ENGINE_STATE_IDLE) {
        // 加载第一页
        if (engine.story != NULL && engine.story->page_count > 0) {
            vn_engine_load_page(engine.story->pages[0].id);
            engine.state = VN_ENGINE_STATE_RUNNING;
        }
    } else if (engine.state == VN_ENGINE_STATE_PAUSED) {
        // 恢复运行
        engine.state = VN_ENGINE_STATE_RUNNING;
    }
}

/**
 * @brief 暂停视觉小说引擎
 */
void vn_engine_pause(void) {
    if (engine.state == VN_ENGINE_STATE_RUNNING) {
        engine.state = VN_ENGINE_STATE_PAUSED;
    }
}

/**
 * @brief 恢复视觉小说引擎
 */
void vn_engine_resume(void) {
    if (engine.state == VN_ENGINE_STATE_PAUSED) {
        engine.state = VN_ENGINE_STATE_RUNNING;
    }
}

/**
 * @brief 停止视觉小说引擎
 */
void vn_engine_stop(void) {
    if (engine.state == VN_ENGINE_STATE_RUNNING || engine.state == VN_ENGINE_STATE_PAUSED) {
        // 释放当前页面资源
        vn_engine_free_current_resources();
        
        // 设置状态为结束
        engine.state = VN_ENGINE_STATE_FINISHED;
    }
}

/**
 * @brief 加载指定页面
 * @param page_id 页面ID
 * @return 加载是否成功
 */
bool vn_engine_load_page(const char *page_id) {
    // 如果引擎未初始化或已结束，直接返回
    if (engine.state == VN_ENGINE_STATE_IDLE || engine.state == VN_ENGINE_STATE_FINISHED) {
        return false;
    }
    
    // 查找页面配置
    page_config_t *page = find_page_by_id(engine.story, page_id);
    if (page == NULL) {
        return false;
    }
    
    // 释放当前页面资源
    vn_engine_free_current_resources();
    
    // 保存当前页面ID
    engine.current_page_id = strdup(page_id);
    engine.current_page = page;
    
    // 加载背景图片
    if (page->background != NULL) {
        lv_img_set_src(engine.background_img, page->background);
    }
    
    // 加载角色图片
    if (page->character_count > 0) {
        engine.character_imgs = (lv_obj_t **)malloc(sizeof(lv_obj_t *) * page->character_count);
        if (engine.character_imgs == NULL) {
            return false;
        }
        
        for (int i = 0; i < page->character_count; i++) {
            character_config_t *char_config = &page->characters[i];
            
            // 创建角色图片对象
            engine.character_imgs[i] = lv_img_create(engine.screen);
            if (engine.character_imgs[i] == NULL) {
                // 释放已创建的角色图片
                for (int j = 0; j < i; j++) {
                    lv_obj_del(engine.character_imgs[j]);
                }
                free(engine.character_imgs);
                engine.character_imgs = NULL;
                return false;
            }

            // 设置角色图片属性
            lv_img_set_src(engine.character_imgs[i], char_config->image);
            lv_obj_set_pos(engine.character_imgs[i], char_config->x, char_config->y);
            // lv_obj_set_style_transform_scale(engine.character_imgs[i], char_config->scale * 256, 0); // Not available in LVGL 8.x
            if (char_config->visible) {
                lv_obj_clear_flag(engine.character_imgs[i], LV_OBJ_FLAG_HIDDEN);
            } else {
                lv_obj_add_flag(engine.character_imgs[i], LV_OBJ_FLAG_HIDDEN);
            }
        }
    }
    
    // 更新文本框
    update_textbox(page->text, &page->textbox);
    
    return true;
}

/**
 * @brief 加载下一页
 * @return 加载是否成功，如果没有下一页返回false
 */
bool vn_engine_load_next_page(void) {
    // 如果当前页面为空或没有下一页，直接返回
    if (engine.current_page == NULL || engine.current_page->next_page == NULL) {
        // 故事结束，停止引擎
        vn_engine_stop();
        return false;
    }
    
    // 加载下一页
    return vn_engine_load_page(engine.current_page->next_page);
}

/**
 * @brief 获取当前引擎状态
 * @return 引擎状态
 */
vn_engine_state_t vn_engine_get_state(void) {
    return engine.state;
}

/**
 * @brief 释放当前页面资源
 */
void vn_engine_free_current_resources(void) {
    // 释放角色图片
    if (engine.character_imgs != NULL && engine.current_page != NULL) {
        for (int i = 0; i < engine.current_page->character_count; i++) {
            if (engine.character_imgs[i] != NULL) {
                lv_obj_del(engine.character_imgs[i]);
            }
        }
        free(engine.character_imgs);
        engine.character_imgs = NULL;
    }
    
    // 释放当前页面ID
    if (engine.current_page_id != NULL) {
        free(engine.current_page_id);
        engine.current_page_id = NULL;
    }
    
    // 清空当前页面
    engine.current_page = NULL;
}

/**
 * @brief 反初始化视觉小说引擎
 */
void vn_engine_deinit(void) {
    // 停止引擎
    vn_engine_stop();
    
    // 释放故事配置
    if (engine.story != NULL) {
        free_story_config(engine.story);
        engine.story = NULL;
    }
    
    // 释放文本框
    if (engine.text_label != NULL) {
        lv_obj_del(engine.text_label);
        engine.text_label = NULL;
    }
    
    if (engine.textbox_bg != NULL) {
        lv_obj_del(engine.textbox_bg);
        engine.textbox_bg = NULL;
    }
    
    // 释放背景图片
    if (engine.background_img != NULL) {
        lv_obj_del(engine.background_img);
        engine.background_img = NULL;
    }
    
    // 反初始化资源管理器
    resource_manager_deinit();
    
    // 重置引擎状态
    engine.state = VN_ENGINE_STATE_IDLE;
}