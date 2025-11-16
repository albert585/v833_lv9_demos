#include "./events.h"
#include "./file_manager.h"
#include "./container.h"
#include "./settings.h"
#include "../main.h"
#include "./audio.h"

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
  lv_obj_remove_flag(parent,LV_OBJ_FLAG_HIDDEN);
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
    switchRobot();
}

// 测试FFmpeg播放 - 简化版本
void test_ffmpeg(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_CLICKED) {
        start_audio_playback("/mnt/app/neuro.mp4");
    }
}

// 停止播放按钮事件
void stop_audio_playback_ui(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_CLICKED) {
        stop_audio_playback();
    }
}