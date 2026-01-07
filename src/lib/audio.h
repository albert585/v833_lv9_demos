#ifndef AUDIO_H
#define AUDIO_H

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
#include "lvgl/lvgl.h"

typedef struct {
    AVFormatContext *fmt_ctx;
    AVCodecContext *codec_ctx;
    AVStream *stream;
    int stream_idx;
    SwrContext *swr_ctx;
    AVFrame *frame;
    AVPacket *pkt;
    lv_obj_t *volume_slider;
    int volume;
    int volume_min;
    int volume_max;
    float playback_speed;
} audio_player_t;

audio_player_t *audio_player_init(lv_obj_t *volume_slider);
int audio_player_open(audio_player_t *player, const char *file_path);
void audio_player_play(audio_player_t *player);
void audio_player_pause(audio_player_t *player);
void audio_player_stop(audio_player_t *player);
void audio_player_set_volume(audio_player_t *player, int volume);
int audio_player_get_position(audio_player_t *player);
int audio_player_get_duration(audio_player_t *player);
void audio_player_set_position(audio_player_t *player, int pos_ms);
void audio_player_set_speed(audio_player_t *player, float speed);
void audio_player_deinit(audio_player_t *player);

#endif // AUDIO_H