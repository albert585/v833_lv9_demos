#include "events.h"
#include "file_manager.h"
#include "lvgl/src/core/lv_obj_pos.h"
#include "lvgl/src/display/lv_display.h"
#include "settings.h"
#include "container.h"
#include "../main.h"
#include "audio.h"
#include "player.h"
#include "virsual_novel/visual_novel_engine.h"
#include "lv_lib_100ask/lv_lib_100ask.h"
#include "button.h"
extern lv_obj_t *parent;
lv_obj_t *obj_2048 = NULL;
player_t *current_player = NULL;

void event_open_manager(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
      lv_obj_add_flag(parent, LV_OBJ_FLAG_HIDDEN);
      file_manager();
    }
}

void event_close_manager(lv_event_t * e){
  // 如果是通过代码调用（非事件触发），e 可能为 NULL
  lv_obj_t * manager = NULL;
  if (e) {
    manager = lv_event_get_user_data(e);
  } else {
    // 如果没有事件，直接使用全局 manager 变量
    extern lv_obj_t *manager;
    manager = manager;
  }

  if (manager) {
    lv_obj_del(manager);
    manager = NULL;
  }

  lv_obj_clear_flag(parent, LV_OBJ_FLAG_HIDDEN);
  settings_close();
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

// 文件选择事件处理
void file_select_event(lv_event_t * e)
{
    lv_obj_t * file_explorer = lv_event_get_target(e);
    lv_event_code_t code = lv_event_get_code(e);
    
    if(code == LV_EVENT_VALUE_CHANGED) {
        // 获取当前路径和选中的文件名
        const char * cur_path = lv_file_explorer_get_current_path(file_explorer);
        const char * sel_fn = lv_file_explorer_get_selected_file_name(file_explorer);
        
        if (cur_path && sel_fn) {
            // 构建完整路径（LVGL 格式）
            char full_path[PATH_MAX];
            snprintf(full_path, sizeof(full_path), "%s%s", cur_path, sel_fn);
            
            printf("[events] File selected: %s\n", full_path);
            
            // 调用 music_player 函数处理音频文件
            music_player(full_path);
        }
    }
}



void event_open_visual_novel(lv_event_t * e)
{
    (void)e; // 避免未使用参数警告
    vn_engine_start();
}

void event_close_visual_novel(lv_event_t * e)
{
    (void)e;
    vn_engine_deinit();
}

void event_close_player(lv_event_t * e)
{
    (void)e; 
    player_destroy(current_player);
}

void player_destroy_callback(player_t *player)
{
    if (!player) return;
    
    // 如果销毁的是当前播放器，清空全局指针
    if (current_player == player) {
        printf("[events] Current player destroyed\n");
        current_player = NULL;
    }
}

void event_open_2048(lv_event_t *e){
    (void)e;
    lv_obj_add_flag(parent, LV_OBJ_FLAG_HIDDEN);
    obj_2048 = lv_100ask_2048_create(lv_screen_active());
    
    if (obj_2048 == NULL) {
        printf("[2048] Failed to create 2048 game object\n");
        lv_obj_clear_flag(parent, LV_OBJ_FLAG_HIDDEN);
        return;
    }
    
    lv_obj_set_size(obj_2048, 200, 200);
    
    if (btn_exit != NULL) {
        lv_obj_set_size(btn_exit, 40, 40);
    }

    lv_obj_center(obj_2048);
}
void event_close_2048(lv_event_t *e){
  (void)e;
  
  if (obj_2048 != NULL) {
    lv_obj_del(obj_2048);
    obj_2048 = NULL;
  }
  
  lv_obj_clear_flag(parent, LV_OBJ_FLAG_HIDDEN);
}

void event_play_video(lv_event_t *e)
{
    (void)e;
    printf("[video] Play video button clicked\n");
    
    // 视频文件路径
    const char *video_file = "/mnt/app/neuro.mp4";
    printf("[video] Playing video file: %s\n", video_file);
    
#if LV_USE_FFMPEG != 0
    // 隐藏主界面
    lv_obj_add_flag(parent, LV_OBJ_FLAG_HIDDEN);
    
    // 创建视频播放器
    lv_obj_t *player = lv_ffmpeg_player_create(lv_screen_active());
    if (!player) {
        printf("[video] Failed to create video player\n");
        lv_obj_clear_flag(parent, LV_OBJ_FLAG_HIDDEN);
        return;
    }
    
    // 设置视频文件
    lv_result_t ret = lv_ffmpeg_player_set_src(player, video_file);
    if (ret != LV_RESULT_OK) {
        printf("[video] Failed to set video source\n");
        lv_obj_del(player);
        lv_obj_clear_flag(parent, LV_OBJ_FLAG_HIDDEN);
        return;
    }
    
    // 设置播放器大小
    lv_obj_set_size(player, LV_HOR_RES, LV_VER_RES);
    lv_obj_center(player);
    
    // 启用音频
#if LV_FFMPEG_AUDIO_SUPPORT != 0
    lv_ffmpeg_player_set_audio_enabled(player, true);
    lv_ffmpeg_player_set_volume(player, 75);
#endif
    
    // 设置自动重播
    lv_ffmpeg_player_set_auto_restart(player, true);
    
    // 开始播放
    lv_ffmpeg_player_set_cmd(player, LV_FFMPEG_PLAYER_CMD_START);
    
    printf("[video] Video playback started\n");
#else
    printf("[video] FFmpeg support is not enabled\n");
#endif
}
// void event_audio_test(lv_event_t * e)     /* 暂时不需要了 */
// {
//     (void)e;
//     printf("[audio_test] Audio test button clicked\n");

//     // 简单的音频测试：播放一个测试文件
//     const char *test_file = "/mnt/app/factory/1khz.wav";
//     printf("[audio_test] Playing test file: %s\n", test_file);    

//     // 如果当前没有播放器，创建一个新的
//     if (current_player == NULL) {
//         printf("[audio_test] Creating new player instance\n");
//         current_player = player_create(parent);  // 使用 parent 容器
//         if (!current_player) {
//             printf("[audio_test] Failed to create player\n");
//             return;
//         }
//     }

//     // 设置音频文件并自动播放
//     player_set_file(current_player, test_file);
//     player_toggle_play_pause(current_player);

//     printf("[audio_test] Audio test started\n");
// }



