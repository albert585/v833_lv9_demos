#include "audio.h"
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include "./file_manager.h"
#include "./container.h"
#include "./events.h"
#include "./player.h"

lv_obj_t *manager = NULL;
static lv_obj_t *file_list = NULL;
static char current_path[PATH_MAX] = "/mnt/app";

// 将 LVGL 文件浏览器路径转换为实际系统路径
// A:/xxx -> /xxx (根据 lv_conf.h 中 LV_FS_POSIX_LETTER = 'A')
static const char *convert_lvgl_path(const char *lvgl_path)
{
    static char real_path[PATH_MAX];
    
    if (!lvgl_path) {
        return NULL;
    }
    
    // 检查是否是 A:/ 开头的路径
    if (strncmp(lvgl_path, "A:/", 3) == 0) {
        // 去掉 A:/ 前缀，直接使用后面的路径
        snprintf(real_path, sizeof(real_path), "%s", lvgl_path + 2);
    } else {
        // 不是 A:/ 路径，直接使用
        snprintf(real_path, sizeof(real_path), "%s", lvgl_path);
    }
    
    return real_path;
}



void file_manager(void) {

    manager = lv_obj_create(lv_screen_active());
    lv_obj_set_size(manager, 960, 540);
    lv_obj_set_pos(manager, 0, 0);
    lv_obj_set_style_border_width(manager, 0, 0);
    lv_obj_add_event_cb(manager,event_close_manager,LV_EVENT_CLICKED,manager);


    /* Create back button */
    lv_obj_t *back_btn = lv_button_create(manager);
    lv_obj_set_size(back_btn, 60, 30);
    lv_obj_add_event_cb(back_btn, event_close_manager, LV_EVENT_CLICKED, manager);

    lv_obj_t *back_label = lv_label_create(back_btn);
    lv_label_set_text(back_label, LV_SYMBOL_LEFT);
    lv_obj_center(back_label);

    lv_obj_t *fmg =lv_file_explorer_create(manager);
    lv_file_explorer_set_sort(fmg, LV_EXPLORER_SORT_KIND);
    lv_obj_set_size(fmg, 960, 540);

    /* Get the file table object and set row height */

    lv_file_explorer_open_dir(fmg,"A:/");

    /* 添加文件选择事件处理 */
    lv_obj_add_event_cb(fmg, file_select_event, LV_EVENT_VALUE_CHANGED, fmg);
  }

static bool is_end_with(const char * str1, const char * str2)
{
    if(str1 == NULL || str2 == NULL)
        return false;
    
    uint16_t len1 = strlen(str1);
    uint16_t len2 = strlen(str2);
    if((len1 < len2) || (len1 == 0 || len2 == 0))
        return false;
    
    while(len2 >= 1)
    {
        if(tolower(str2[len2 - 1])  != tolower(str1[len1 - 1]))
            return false;

        len2--;
        len1--;
    }

    return true;
}

// 音乐播放器函数：根据文件路径创建或更新播放器
void music_player(const char * path)
{
    if (!path) return;

    // 转换 LVGL 路径为实际系统路径
    const char *real_path = convert_lvgl_path(path);
    if (!real_path) return;

    printf("[file_manager] Opening file: %s\n", real_path);

    // 检查是否是音频文件
    if (is_end_with(real_path, ".mp3") ||
        is_end_with(real_path, ".wav") ||
        is_end_with(real_path, ".flac") ||
        is_end_with(real_path, ".aac") ||
        is_end_with(real_path, ".ogg") ||
        is_end_with(real_path, ".m4a")) {

        printf("[file_manager] Audio file detected, creating player...\n");

        // 关闭文件管理器
        event_close_manager(NULL);

        // 如果当前没有播放器，创建一个新的
        if (current_player == NULL) {
            printf("[file_manager] Creating new player instance\n");
            current_player = player_create(parent);  // 使用 parent 容器
            if (!current_player) {
                printf("[file_manager] Failed to create player\n");
                return;
            }
        }

        // 设置音频文件并自动播放
        player_set_file(current_player, real_path);
        player_toggle_play_pause(current_player);

        printf("[file_manager] Player started successfully\n");
    } else {
        printf("[file_manager] Not an audio file: %s\n", real_path);
    }
}
