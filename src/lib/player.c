#include "player.h"
#include <stdio.h>
#include <pthread.h>
#include <alsa/asoundlib.h>

static pthread_t alsa_init_thread;
static volatile int alsa_initialized = 0;

// ALSA 预初始化线程
static void *alsa_preinit_thread_func(void *arg) {
    (void)arg;
    printf("[player] ALSA pre-init thread started\n");

    // 禁用 ALSA 配置文件
    setenv("ALSA_CONFIG_PATH", "/dev/null", 1);

    // 预初始化 ALSA，使用非阻塞模式打开设备
    snd_pcm_t *test_pcm;
    printf("[player] Opening ALSA device (non-blocking)...\n");
    int ret = snd_pcm_open(&test_pcm, "hw:0,0", SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK);
    if (ret >= 0) {
        printf("[player] ALSA pre-init successful\n");
        snd_pcm_close(test_pcm);
    } else {
        printf("[player] ALSA pre-init failed: %s\n", snd_strerror(ret));
    }

    alsa_initialized = 1;
    printf("[player] ALSA pre-init thread finished\n");
    return NULL;
}

// 启动 ALSA 预初始化（在 main() 中调用）
void player_preinit_alsa(void) {
    if (alsa_initialized) return;
    printf("[player] Starting ALSA pre-init...\n");
    pthread_create(&alsa_init_thread, NULL, alsa_preinit_thread_func, NULL);
    pthread_detach(alsa_init_thread);
}

static void update_time_label(player_t *player) {
    if (!player || !player->audio) return;

    int pos = audio_player_get_position(player->audio);
    int dur = audio_player_get_duration(player->audio);

    char buf[32];
    snprintf(buf, sizeof(buf), "%02d:%02d / %02d:%02d",
             pos / 60, pos % 60, dur / 60, dur % 60);
    lv_label_set_text(player->time_label, buf);
}

static void timer_callback(lv_timer_t *timer) {
    player_t *player = timer->user_data;
    if (!player || !player->audio) return;

    if (player->state == PLAYER_STATE_PLAYING) {
        int pos = audio_player_get_position(player->audio);
        int dur = audio_player_get_duration(player->audio);
        int pct = dur > 0 ? (pos * 100) / dur : 0;
        lv_slider_set_value(player->progress_slider, pct, LV_ANIM_OFF);
        update_time_label(player);
    }
}

static void control_btn_callback(lv_event_t *e) {
    player_t *player = lv_event_get_user_data(e);
    player_toggle_play_pause(player);
}

static void progress_slider_callback(lv_event_t *e) {
    player_t *player = lv_event_get_user_data(e);
    if (lv_event_get_code(e) == LV_EVENT_RELEASED) {
        int pct = lv_slider_get_value(player->progress_slider);
        int dur = audio_player_get_duration(player->audio);
        audio_player_set_position(player->audio, (dur * pct) / 100);
    }
}

static void volume_slider_callback(lv_event_t *e) {
    player_t *player = lv_event_get_user_data(e);
    int vol = lv_slider_get_value(player->volume_slider);
    audio_player_set_volume(player->audio, vol);
}

static void close_btn_callback(lv_event_t *e) {
    player_t *player = lv_event_get_user_data(e);
    player_destroy(player);
}

player_t *player_create(lv_obj_t *parent) {
    player_t *player = malloc(sizeof(player_t));
    if (!player) return NULL;
    memset(player, 0, sizeof(player_t));

    // 创建容器
    player->cont = lv_obj_create(parent);
    lv_obj_set_size(player->cont, lv_pct(100), 120);
    lv_obj_set_flex_flow(player->cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(player->cont, 8, 0);

    // 创建顶部行容器（标题和关闭按钮）
    lv_obj_t *top_row = lv_obj_create(player->cont);
    lv_obj_set_size(top_row, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(top_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(top_row, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(top_row, 0, 0);
    lv_obj_set_style_border_width(top_row, 0, 0);
    lv_obj_set_style_bg_opa(top_row, LV_OPA_TRANSP, 0);

    // 标题标签
    player->title_label = lv_label_create(top_row);
    lv_label_set_text(player->title_label, "未选择文件");
    lv_obj_set_flex_grow(player->title_label, 1);

    // 关闭按钮
    lv_obj_t *close_btn = lv_btn_create(top_row);
    lv_obj_set_size(close_btn, 40, 40);
    lv_obj_add_event_cb(close_btn, close_btn_callback, LV_EVENT_CLICKED, player);
    lv_obj_t *close_label = lv_label_create(close_btn);
    lv_label_set_text(close_label, "×");

    // 进度条
    player->progress_slider = lv_slider_create(player->cont);
    lv_obj_set_width(player->progress_slider, lv_pct(100));
    lv_obj_add_event_cb(player->progress_slider, progress_slider_callback,
                       LV_EVENT_VALUE_CHANGED | LV_EVENT_RELEASED, player);

    // 时间标签
    player->time_label = lv_label_create(player->cont);
    lv_label_set_text(player->time_label, "00:00 / 00:00");

    // 控制按钮
    player->control_btn = lv_btn_create(player->cont);
    lv_obj_set_size(player->control_btn, 50, 50);
    lv_obj_add_event_cb(player->control_btn, control_btn_callback, LV_EVENT_CLICKED, player);
    lv_obj_t *btn_label = lv_label_create(player->control_btn);
    lv_label_set_text(btn_label, "播放");

    // 音量滑块
    player->volume_slider = lv_slider_create(player->cont);
    lv_obj_set_width(player->volume_slider, 150);
    lv_slider_set_range(player->volume_slider, 0, 100);
    lv_obj_add_event_cb(player->volume_slider, volume_slider_callback, LV_EVENT_VALUE_CHANGED, player);

    // 延迟初始化音频播放器，避免在 LVGL 事件回调中阻塞
    player->audio = NULL;
    player->state = PLAYER_STATE_STOPPED;

    // 创建定时器更新进度
    player->timer = lv_timer_create(timer_callback, 1000, player);
    if (!player->timer) {
        lv_obj_del(player->cont);
        free(player);
        return NULL;
    }
    lv_timer_pause(player->timer);

    return player;
}

void player_set_file(player_t *player, const char *file_path) {
    if (!player || !file_path) return;

    player_stop(player);

    // 等待 ALSA 预初始化完成
    printf("[player] Waiting for ALSA pre-init...\n");
    int timeout = 50; // 最多等待 5 秒
    while (!alsa_initialized && timeout > 0) {
        usleep(100000); // 等待 100ms
        timeout--;
    }

    if (!alsa_initialized) {
        printf("[player] Warning: ALSA pre-init timeout\n");
    } else {
        printf("[player] ALSA pre-init completed\n");
    }

    // 初始化音频播放器（由于 ALSA 已经预初始化，这里应该不会阻塞）
    printf("[player] Initializing audio player...\n");
    player->audio = audio_player_init(player->volume_slider);
    if (!player->audio) {
        printf("[player] Failed to initialize audio player\n");
        return;
    }
    printf("[player] Audio player initialized\n");

    if (audio_player_open(player->audio, file_path) == 0) {
        lv_label_set_text(player->title_label, file_path);
        lv_slider_set_value(player->progress_slider, 0, LV_ANIM_OFF);
        update_time_label(player);
    }
}

void player_toggle_play_pause(player_t *player) {
    if (!player) return;
    if (!player->audio) {
        printf("[player] Audio player not initialized\n");
        return;
    }

    if (player->state == PLAYER_STATE_PLAYING) {
        audio_player_pause(player->audio);
        player->state = PLAYER_STATE_PAUSED;
        lv_label_set_text(lv_obj_get_child(player->control_btn, 0), "播放");
        lv_timer_pause(player->timer);
    } else {
        audio_player_play(player->audio);
        player->state = PLAYER_STATE_PLAYING;
        lv_label_set_text(lv_obj_get_child(player->control_btn, 0), "暂停");
        lv_timer_resume(player->timer);
    }
}

void player_stop(player_t *player) {
    if (!player) return;

    if (player->audio) {
        audio_player_stop(player->audio);
    }
    player->state = PLAYER_STATE_STOPPED;
    lv_label_set_text(lv_obj_get_child(player->control_btn, 0), "播放");
    lv_slider_set_value(player->progress_slider, 0, LV_ANIM_OFF);
    update_time_label(player);
    lv_timer_pause(player->timer);
}

player_state_t player_get_state(player_t *player) {
    return player ? player->state : PLAYER_STATE_STOPPED;
}

int player_get_position_pct(player_t *player) {
    if (!player || !player->audio) return 0;
    int dur = audio_player_get_duration(player->audio);
    return dur > 0 ? (audio_player_get_position(player->audio) * 100) / dur : 0;
}

void player_destroy(player_t *player) {
    if (!player) return;

    // 调用销毁回调
    player_destroy_callback(player);

    lv_timer_del(player->timer);
    if (player->audio) {
        audio_player_deinit(player->audio);
    }
    lv_obj_del(player->cont);
    free(player);
}