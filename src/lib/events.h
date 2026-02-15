#include "lvgl/lvgl.h"
#include "lvgl/src/misc/lv_types.h"
#include "player.h"

extern void event_close_manager(lv_event_t * e);
extern void event_open_manager(lv_event_t * e);
extern void slider_event_cb(lv_event_t * e);
extern void event_open_settings(lv_event_t *e);
extern void btn_robot_click(lv_event_t * e);
extern void file_select_event(lv_event_t * e);
extern void event_open_visual_novel(lv_event_t * e);
extern void event_close_visual_novel(lv_event_t * e);
extern void event_open_2048(lv_event_t * e);
// 音频播放器相关
extern player_t *current_player;
extern void event_close_player(lv_event_t * e);
extern void player_destroy_callback(player_t *player);
// 音频测试