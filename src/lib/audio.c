#include "audio.h"
#include <pthread.h>
#include <alsa/asoundlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static pthread_t audio_thread;
static volatile int is_playing = 0;
static volatile int is_paused = 0;
static snd_pcm_t *pcm_handle;

static void *audio_playback_thread(void *arg) {
    audio_player_t *player = (audio_player_t *)arg;
    uint8_t *audio_buf = NULL;
    int audio_buf_size = 0;

    while (is_playing) {
        if (is_paused) {
            usleep(50000);
            continue;
        }

        int ret = av_read_frame(player->fmt_ctx, player->pkt);
        if (ret < 0) {
            if (ret == AVERROR_EOF) {
                // 播放结束
                is_playing = 0;
                break;
            }
            usleep(10000);
            continue;
        }

        if (player->pkt->stream_index != player->stream_idx) {
            av_packet_unref(player->pkt);
            continue;
        }

        ret = avcodec_send_packet(player->codec_ctx, player->pkt);
        if (ret < 0) {
            av_packet_unref(player->pkt);
            continue;
        }

        while (ret >= 0) {
            ret = avcodec_receive_frame(player->codec_ctx, player->frame);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) break;
            
            // 音频重采样
            int dst_nb_samples = av_rescale_rnd(
                swr_get_delay(player->swr_ctx, player->codec_ctx->sample_rate) + 
                player->frame->nb_samples,
                44100, player->codec_ctx->sample_rate, AV_ROUND_UP
            );

            if (dst_nb_samples > audio_buf_size / 2) {
                audio_buf_size = dst_nb_samples * 2 * 2; // 16位双通道
                uint8_t *new_buf = realloc(audio_buf, audio_buf_size);
                if (!new_buf) {
                    continue;
                }
                audio_buf = new_buf;
            }

            int out_samples = swr_convert(
                player->swr_ctx, &audio_buf, dst_nb_samples,
                (const uint8_t **)player->frame->data, player->frame->nb_samples
            );

            // 写入音频设备
            snd_pcm_writei(pcm_handle, audio_buf, out_samples);
            av_frame_unref(player->frame);
        }
        av_packet_unref(player->pkt);
    }

    free(audio_buf);
    return NULL;
}

audio_player_t *audio_player_init(lv_obj_t *volume_slider) {
    audio_player_t *player = malloc(sizeof(audio_player_t));
    memset(player, 0, sizeof(audio_player_t));
    player->volume_slider = volume_slider;
    player->volume = 75;
    player->volume_min = 0;
    player->volume_max = 100;
    player->playback_speed = 1.0f;

    printf("[audio] Initializing audio player...\n");

    int ret;
    unsigned int rate = 44100;
    int dir = 0;
    snd_pcm_hw_params_t *hw_params;
    snd_pcm_sw_params_t *sw_params;

    // 打开 PCM 设备（使用非阻塞模式，避免在单核环境下阻塞）
    printf("[audio] Opening PCM device default (non-blocking)...\n");
    ret = snd_pcm_open(&pcm_handle, "default", SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK);
    if (ret < 0) {
        printf("[audio] Error opening PCM device: %s\n", snd_strerror(ret));
        if (ret == -EBUSY) {
            printf("[audio] Device is busy, may be occupied by another process\n");
        } else if (ret == -EINVAL) {
            printf("[audio] Invalid parameters or device name\n");
        }
        free(player);
        return NULL;
    }
    printf("[audio] PCM device opened successfully\n");

    // 切换回阻塞模式（用于后续的 snd_pcm_writei 调用）
    ret = snd_pcm_nonblock(pcm_handle, 0);
    if (ret < 0) {
        printf("[audio] Warning: Failed to set blocking mode: %s\n", snd_strerror(ret));
    }
    if (ret < 0) {
        printf("[audio] Error opening PCM device: %s\n", snd_strerror(ret));
        if (ret == -EBUSY) {
            printf("[audio] Device is busy, may be occupied by another process\n");
        } else if (ret == -EINVAL) {
            printf("[audio] Invalid parameters or device name\n");
        }
        free(player);
        return NULL;
    }
    printf("[audio] PCM device opened successfully\n");

    // 分配硬件参数结构
    snd_pcm_hw_params_alloca(&hw_params);

    // 初始化硬件参数
    printf("[audio] Initializing hardware parameters...\n");
    ret = snd_pcm_hw_params_any(pcm_handle, hw_params);
    if (ret < 0) {
        printf("[audio] Error initializing hardware parameters: %s\n", snd_strerror(ret));
        snd_pcm_close(pcm_handle);
        free(player);
        return NULL;
    }

    // 设置访问模式
    ret = snd_pcm_hw_params_set_access(pcm_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
    if (ret < 0) {
        printf("[audio] Error setting access type: %s\n", snd_strerror(ret));
        snd_pcm_close(pcm_handle);
        free(player);
        return NULL;
    }

    // 设置采样格式
    ret = snd_pcm_hw_params_set_format(pcm_handle, hw_params, SND_PCM_FORMAT_S16_LE);
    if (ret < 0) {
        printf("[audio] Error setting sample format: %s\n", snd_strerror(ret));
        snd_pcm_close(pcm_handle);
        free(player);
        return NULL;
    }

    // 设置通道数
    ret = snd_pcm_hw_params_set_channels(pcm_handle, hw_params, 2);
    if (ret < 0) {
        printf("[audio] Error setting channel count: %s\n", snd_strerror(ret));
        snd_pcm_close(pcm_handle);
        free(player);
        return NULL;
    }

    // 设置采样率
    ret = snd_pcm_hw_params_set_rate_near(pcm_handle, hw_params, &rate, &dir);
    if (ret < 0) {
        printf("[audio] Error setting sample rate: %s\n", snd_strerror(ret));
        snd_pcm_close(pcm_handle);
        free(player);
        return NULL;
    }
    printf("[audio] Sample rate set to %u Hz\n", rate);

    // 应用硬件参数
    printf("[audio] Applying hardware parameters...\n");
    ret = snd_pcm_hw_params(pcm_handle, hw_params);
    if (ret < 0) {
        printf("[audio] Error setting hardware parameters: %s\n", snd_strerror(ret));
        snd_pcm_close(pcm_handle);
        free(player);
        return NULL;
    }

    printf("[audio] Audio player initialized successfully\n");
    return player;
}

int audio_player_open(audio_player_t *player, const char *file_path) {
    audio_player_stop(player);

    if (avformat_open_input(&player->fmt_ctx, file_path, NULL, NULL) != 0)
        return -1;

    if (avformat_find_stream_info(player->fmt_ctx, NULL) < 0)
        return -1;

    // 查找音频流
    player->stream_idx = av_find_best_stream(player->fmt_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
    if (player->stream_idx < 0) return -1;

    player->stream = player->fmt_ctx->streams[player->stream_idx];
    const AVCodec *codec = avcodec_find_decoder(player->stream->codecpar->codec_id);
    player->codec_ctx = avcodec_alloc_context3(codec);
    avcodec_parameters_to_context(player->codec_ctx, player->stream->codecpar);
    avcodec_open2(player->codec_ctx, codec, NULL);

    // 初始化重采样上下文
    AVChannelLayout src_ch_layout = player->codec_ctx->ch_layout;
    AVChannelLayout dst_ch_layout;
    av_channel_layout_default(&dst_ch_layout, 2);

    player->swr_ctx = swr_alloc();
    av_opt_set_chlayout(player->swr_ctx, "in_chlayout", &src_ch_layout, 0);
    av_opt_set_int(player->swr_ctx, "in_sample_rate", player->codec_ctx->sample_rate, 0);
    av_opt_set_sample_fmt(player->swr_ctx, "in_sample_fmt", player->codec_ctx->sample_fmt, 0);
    av_opt_set_chlayout(player->swr_ctx, "out_chlayout", &dst_ch_layout, 0);
    av_opt_set_int(player->swr_ctx, "out_sample_rate", 44100, 0);
    av_opt_set_sample_fmt(player->swr_ctx, "out_sample_fmt", AV_SAMPLE_FMT_S16, 0);
    swr_init(player->swr_ctx);

    player->frame = av_frame_alloc();
    player->pkt = av_packet_alloc();

    return 0;
}

void audio_player_play(audio_player_t *player) {
    if (!player->fmt_ctx) return;
    is_paused = 0;
    if (!is_playing) {
        is_playing = 1;
        pthread_create(&audio_thread, NULL, audio_playback_thread, player);
    }
}

void audio_player_pause(audio_player_t *player) {
    (void)player;
    is_paused = 1;
}

void audio_player_stop(audio_player_t *player) {
    if (!player) return;

    is_playing = 0;
    is_paused = 0;
    if (audio_thread) {
        pthread_join(audio_thread, NULL);
        audio_thread = 0;
    }
    if (player->pkt) av_packet_unref(player->pkt);
    if (player->frame) av_frame_unref(player->frame);
}

void audio_player_set_volume(audio_player_t *player, int volume) {
    player->volume = LV_CLAMP(player->volume_min, volume, player->volume_max);
    snd_mixer_t *mixer;
    snd_mixer_selem_id_t *sid;
    snd_mixer_open(&mixer, 0);
    snd_mixer_attach(mixer, "default");
    snd_mixer_selem_register(mixer, NULL, NULL);
    snd_mixer_load(mixer);
    
    snd_mixer_selem_id_alloca(&sid);
    snd_mixer_selem_id_set_name(sid, "Master");
    snd_mixer_elem_t *elem = snd_mixer_find_selem(mixer, sid);
    
    long min, max;
    snd_mixer_selem_get_playback_volume_range(elem, &min, &max);
    snd_mixer_selem_set_playback_volume_all(elem, min + (max - min) * player->volume / 100);
    snd_mixer_close(mixer);
    
    if (player->volume_slider) {
        lv_slider_set_value(player->volume_slider, player->volume, LV_ANIM_ON);
    }
}

int audio_player_get_position(audio_player_t *player) {
    if (!player->fmt_ctx) return 0;
    // FFmpeg 5.0+ 移除了 cur_dts，使用 pkt->dts 作为近似值
    if (player->pkt && player->pkt->dts != AV_NOPTS_VALUE) {
        return av_rescale_q(player->pkt->dts,
                           player->stream->time_base, AV_TIME_BASE_Q) / 1000;
    }
    return 0;
}

int audio_player_get_duration(audio_player_t *player) {
    if (!player->fmt_ctx) return 0;
    return player->fmt_ctx->duration / 1000000;
}

void audio_player_set_position(audio_player_t *player, int pos_ms) {
    if (!player->fmt_ctx) return;
    int64_t pos = pos_ms * 1000;
    av_seek_frame(player->fmt_ctx, player->stream_idx, pos, AVSEEK_FLAG_ANY);
}

void audio_player_set_speed(audio_player_t *player, float speed) {
    player->playback_speed = LV_CLAMP(0.5f, speed, 2.0f);
    // FFmpeg 5.0+ 移除了 avcodec_set_pkt_timebase，直接设置 time_base
    player->codec_ctx->time_base = (AVRational){1, (int)(player->stream->codecpar->sample_rate * speed)};
}

void audio_player_deinit(audio_player_t *player) {
    audio_player_stop(player);
    if (player->swr_ctx) swr_free(&player->swr_ctx);
    if (player->codec_ctx) avcodec_free_context(&player->codec_ctx);
    if (player->fmt_ctx) avformat_close_input(&player->fmt_ctx);
    if (player->frame) av_frame_free(&player->frame);
    if (player->pkt) av_packet_free(&player->pkt);
    snd_pcm_close(pcm_handle);
    free(player);
}