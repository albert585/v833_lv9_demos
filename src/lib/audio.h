// audio.h
#ifndef AUDIO_H
#define AUDIO_H

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
#include <alsa/asoundlib.h>
#include <pthread.h>

typedef struct {
    // FFmpeg相关
    AVFormatContext *format_ctx;
    AVCodecContext *codec_ctx;
    int audio_stream_idx;
    SwrContext *swr_ctx;
    AVFrame *frame;
    AVPacket *pkt;
    
    // ALSA相关
    snd_pcm_t *alsa_handle;
    snd_pcm_uframes_t alsa_period_size;
    snd_pcm_format_t alsa_format;
    unsigned int alsa_channels;
    unsigned int alsa_sample_rate;
    
    // 控制相关
    int is_playing;
    int should_stop;
    pthread_t play_thread;
    
    // 新增：歌曲时长信息
    double duration_sec;  // 歌曲总时长（秒）
} AudioState;

// 函数声明
int init_ffmpeg(AudioState *state, const char *filename);
int init_alsa(AudioState *state);
void decode_and_play(AudioState *state);
void cleanup(AudioState *state);

// 新增控制函数
int start_audio_playback(const char* filename);
void stop_audio_playback(void);
int is_audio_playing(void);
double get_audio_duration(void);

#endif