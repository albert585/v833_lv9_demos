#ifndef PLAYER_H
#define PLAYER_H

#include "lvgl/lvgl.h"
#include "audio.h"

typedef enum {
    PLAYER_STATE_STOPPED,
    PLAYER_STATE_PLAYING,
    PLAYER_STATE_PAUSED
} player_state_t;

typedef struct {
    lv_obj_t *cont;
    lv_obj_t *title_label;
    lv_obj_t *progress_slider;
    lv_obj_t *time_label;
    lv_obj_t *control_btn;
    lv_obj_t *volume_slider;
    audio_player_t *audio;
    player_state_t state;
    lv_timer_t *timer;
} player_t;

player_t *player_create(lv_obj_t *parent);
void player_set_file(player_t *player, const char *file_path);
void player_toggle_play_pause(player_t *player);
void player_stop(player_t *player);
player_state_t player_get_state(player_t *player);
int player_get_position_pct(player_t *player);
void player_destroy(player_t *player);
void player_preinit_alsa(void);

// 播放器销毁回调（需要在 events.c 中实现）
extern void player_destroy_callback(player_t *player);

#endif // PLAYER_H