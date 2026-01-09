#include "audio.h"
#include <pthread.h>
#include <libavdevice/avdevice.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static pthread_t audio_thread;
static volatile int is_playing = 0;
static volatile int is_paused = 0;
static AVFormatContext *out_fmt_ctx = NULL;

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

            // 使用 avdevice 输出音频
            if (out_fmt_ctx) {
                AVPacket *out_pkt = av_packet_alloc();
                if (out_pkt) {
                    out_pkt->data = audio_buf;
                    out_pkt->size = out_samples * 2 * 2; // 16位双通道
                    out_pkt->stream_index = 0;
                    out_pkt->pts = player->frame->pts;
                    out_pkt->dts = player->frame->pkt_dts;
                    out_pkt->duration = player->frame->duration;
                    
                    av_interleaved_write_frame(out_fmt_ctx, out_pkt);
                    av_packet_free(&out_pkt);
                }
            }
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

    printf("[audio] Initializing audio player with avdevice...\n");

    // 初始化 avdevice
    avdevice_register_all();

    // 打开 ALSA 输出设备
    int ret = avformat_alloc_output_context2(&out_fmt_ctx, NULL, "alsa", "default");
    if (ret < 0 || !out_fmt_ctx) {
        printf("[audio] Error creating output context: %s\n", av_err2str(ret));
        free(player);
        return NULL;
    }

    // 设置音频参数
    AVStream *out_stream = avformat_new_stream(out_fmt_ctx, NULL);
    if (!out_stream) {
        printf("[audio] Error creating output stream\n");
        avformat_free_context(out_fmt_ctx);
        free(player);
        return NULL;
    }

    AVCodecParameters *codecpar = out_stream->codecpar;
    codecpar->codec_type = AVMEDIA_TYPE_AUDIO;
    codecpar->codec_id = AV_CODEC_ID_PCM_S16LE;
    codecpar->sample_rate = 44100;
    codecpar->ch_layout = (AVChannelLayout)AV_CHANNEL_LAYOUT_STEREO;
    codecpar->format = AV_SAMPLE_FMT_S16;

    // 打开输出设备
    if (!(out_fmt_ctx->oformat->flags & AVFMT_NOFILE)) {
        ret = avio_open(&out_fmt_ctx->pb, out_fmt_ctx->url, AVIO_FLAG_WRITE);
        if (ret < 0) {
            printf("[audio] Error opening output device: %s\n", av_err2str(ret));
            avformat_free_context(out_fmt_ctx);
            free(player);
            return NULL;
        }
    }

    // 写入头信息
    ret = avformat_write_header(out_fmt_ctx, NULL);
    if (ret < 0) {
        printf("[audio] Error writing header: %s\n", av_err2str(ret));
        avio_closep(&out_fmt_ctx->pb);
        avformat_free_context(out_fmt_ctx);
        free(player);
        return NULL;
    }

    printf("[audio] Audio player initialized successfully with avdevice\n");
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

    // 重新初始化输出设备
    if (out_fmt_ctx) {
        av_write_trailer(out_fmt_ctx);
        if (!(out_fmt_ctx->oformat->flags & AVFMT_NOFILE)) {
            avio_closep(&out_fmt_ctx->pb);
        }
        avformat_free_context(out_fmt_ctx);
        out_fmt_ctx = NULL;
    }

    int ret = avformat_alloc_output_context2(&out_fmt_ctx, NULL, "alsa", "default");
    if (ret < 0 || !out_fmt_ctx) {
        printf("[audio] Error creating output context: %s\n", av_err2str(ret));
        return -1;
    }

    AVStream *out_stream = avformat_new_stream(out_fmt_ctx, NULL);
    if (!out_stream) {
        printf("[audio] Error creating output stream\n");
        avformat_free_context(out_fmt_ctx);
        out_fmt_ctx = NULL;
        return -1;
    }

    AVCodecParameters *codecpar = out_stream->codecpar;
    codecpar->codec_type = AVMEDIA_TYPE_AUDIO;
    codecpar->codec_id = AV_CODEC_ID_PCM_S16LE;
    codecpar->sample_rate = 44100;
    codecpar->ch_layout = (AVChannelLayout)AV_CHANNEL_LAYOUT_STEREO;
    codecpar->format = AV_SAMPLE_FMT_S16;

    // 打开输出设备
    if (!(out_fmt_ctx->oformat->flags & AVFMT_NOFILE)) {
        ret = avio_open(&out_fmt_ctx->pb, out_fmt_ctx->url, AVIO_FLAG_WRITE);
        if (ret < 0) {
            printf("[audio] Error opening output device: %s\n", av_err2str(ret));
            avformat_free_context(out_fmt_ctx);
            out_fmt_ctx = NULL;
            return -1;
        }
    }

    // 写入头信息
    ret = avformat_write_header(out_fmt_ctx, NULL);
    if (ret < 0) {
        printf("[audio] Error writing header: %s\n", av_err2str(ret));
        avio_closep(&out_fmt_ctx->pb);
        avformat_free_context(out_fmt_ctx);
        out_fmt_ctx = NULL;
        return -1;
    }

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
    
    // 刷新输出缓冲区
    if (out_fmt_ctx) {
        av_write_trailer(out_fmt_ctx);
    }
}

void audio_player_set_volume(audio_player_t *player, int volume) {
    player->volume = LV_CLAMP(player->volume_min, volume, player->volume_max);
    
    // avdevice 可能支持音量控制，但需要具体设备支持
    // 这里暂时保持音量值，实际音量控制可能需要通过硬件特性实现
    // 如果设备支持，可以使用 AVDictionary 设置音量参数
    
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
    
    // 清理 avdevice 输出
    if (out_fmt_ctx) {
        av_write_trailer(out_fmt_ctx);
        if (!(out_fmt_ctx->oformat->flags & AVFMT_NOFILE)) {
            avio_closep(&out_fmt_ctx->pb);
        }
        avformat_free_context(out_fmt_ctx);
        out_fmt_ctx = NULL;
    }
    
    free(player);
}