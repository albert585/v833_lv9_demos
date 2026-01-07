#include "./events.h"
#include "./file_manager.h"
#include "./container.h"
#include "./settings.h"
#include "../main.h"
#include "./audio.h"
#include "./player.h"
#include "./virsual_novel/visual_novel_engine.h"
#include "lv_lib_100ask.h"

player_t *current_player = NULL;

void event_open_manager(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
      file_manager();
    }
}

void event_close_manager(lv_event_t * e){
  lv_obj_t * obj = lv_event_get_target(e);
  lv_obj_t * manager = lv_event_get_user_data(e);
  lv_obj_del(manager);
  settings_close();
  lv_obj_clear_flag(parent,LV_OBJ_FLAG_HIDDEN);
  lv_obj_set_style_opa(parent, LV_OPA_COVER, 255);
}

void event_open_settings(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_CLICKED) {
      settings();
    }
}

void btn_robot_click(lv_event_t * e)
{
    (void)e; // 避免未使用参数警告
    switchRobot();
}

void file_select_event(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_VALUE_CHANGED) {
        // 使用 lv_100ask_file_explorer 获取选中的文件
        lv_obj_t * fe = lv_event_get_target(e);
        const char * sel_fn = lv_100ask_file_explorer_get_sel_fn(fe);
        const char * cur_path = lv_100ask_file_explorer_get_cur_path(fe);
        
        if (sel_fn && strlen(sel_fn) > 0) {
            // 构建完整文件路径
            char full_path[512];
            snprintf(full_path, sizeof(full_path), "%s/%s", cur_path, sel_fn);
            
            printf("[player] Selected file: %s\n", full_path);

            if (current_player) {
                player_destroy(current_player);
            }

            current_player = player_create(parent);
            if (current_player) {
                player_set_file(current_player, full_path);
                player_toggle_play_pause(current_player);
            }
        }
    }
}


void event_open_visual_novel(lv_event_t * e)
{
    (void)e; // 避免未使用参数警告
    // 暂时注释掉
    // event_close_visual_novel(e);
}

void event_close_visual_novel(lv_event_t * e)
{
    (void)e; // 避免未使用参数警告
}

void event_close_player(lv_event_t * e)
{
    (void)e; // 避免未使用参数警告
}

void player_destroy_callback(player_t *player)
{
    (void)player; // 避免未使用参数警告
}

void event_audio_test(lv_event_t * e)
{
    (void)e; // 避免未使用参数警告
    
    printf("[Audio Test] Starting audio test...\n");
    
    if (current_player) {
        printf("[Audio Test] Destroying existing player\n");
        player_destroy(current_player);
        current_player = NULL;
    }
    
    printf("[Audio Test] Creating player\n");
    current_player = player_create(parent);
    
    if (current_player) {
        const char *test_file = "/mnt/app/factory/play_test.wav";
        printf("[Audio Test] Playing test file: %s\n", test_file);
        
        player_set_file(current_player, test_file);
        player_toggle_play_pause(current_player);
        
        printf("[Audio Test] Audio test started\n");
    } else {
        printf("[Audio Test] Error: Failed to create player\n");
    }
}