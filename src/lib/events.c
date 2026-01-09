#include "./events.h"
#include "./file_manager.h"
#include "./settings.h"
#include "../main.h"
#include "./audio.h"
#include "./player.h"
#include "./virsual_novel/visual_novel_engine.h"

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



void event_open_visual_novel(lv_event_t * e)
{
    (void)e; // 避免未使用参数警告
    vn_engine_start();
}

void event_close_visual_novel(lv_event_t * e)
{
    (void)e; // 避免未使用参数警告
    vn_engine_deinit();


}

void event_close_player(lv_event_t * e)
{
    (void)e; // 避免未使用参数警告
}

void player_destroy_callback(player_t *player)
{
    (void)player; // 避免未使用参数警告
}

