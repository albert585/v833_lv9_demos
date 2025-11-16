#include "audio.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static AudioState *g_audio_state = NULL;

// 初始化FFmpeg并打开音频文件
int init_ffmpeg(AudioState *state, const char *filename) {
    int ret;

    // 打开输入文件
    ret = avformat_open_input(&state->format_ctx, filename, NULL, NULL);
    if (ret < 0) {
        fprintf(stderr, "无法打开文件: %s\n", filename);
        return -1;
    }

    // 查找流信息
    ret = avformat_find_stream_info(state->format_ctx, NULL);
    if (ret < 0) {
        fprintf(stderr, "无法获取流信息\n");
        return -1;
    }

    // 查找音频流
    state->audio_stream_idx = -1;
    for (int i = 0; i < state->format_ctx->nb_streams; i++) {
        if (state->format_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            state->audio_stream_idx = i;
            break;
        }
    }

    if (state->audio_stream_idx == -1) {
        fprintf(stderr, "找不到音频流\n");
        return -1;
    }

    // 获取解码器
    AVCodecParameters *codecpar = state->format_ctx->streams[state->audio_stream_idx]->codecpar;
    const AVCodec *codec = avcodec_find_decoder(codecpar->codec_id);
    if (!codec) {
        fprintf(stderr, "找不到解码器\n");
        return -1;
    }

    // 创建解码器上下文
    state->codec_ctx = avcodec_alloc_context3(codec);
    if (!state->codec_ctx) {
        fprintf(stderr, "无法分配解码器上下文\n");
        return -1;
    }

    // 复制参数到解码器上下文
    ret = avcodec_parameters_to_context(state->codec_ctx, codecpar);
    if (ret < 0) {
        fprintf(stderr, "无法复制参数到解码器上下文\n");
        return -1;
    }

    // 打开解码器
    ret = avcodec_open2(state->codec_ctx, codec, NULL);
    if (ret < 0) {
        fprintf(stderr, "无法打开解码器\n");
        return -1;
    }

    // 初始化包和帧
    state->pkt = av_packet_alloc();
    state->frame = av_frame_alloc();
    if (!state->pkt || !state->frame) {
        fprintf(stderr, "无法分配包或帧\n");
        return -1;
    }

    // 初始化重采样器
    state->swr_ctx = swr_alloc();
    if (!state->swr_ctx) {
        fprintf(stderr, "无法分配重采样器\n");
        return -1;
    }

    // 设置重采样参数 - 使用新API处理通道布局
    AVChannelLayout in_ch_layout = {0};
    AVChannelLayout out_ch_layout = AV_CHANNEL_LAYOUT_STEREO;

    // 获取输入通道布局
    if (state->codec_ctx->ch_layout.order != AV_CHANNEL_ORDER_UNSPEC) {
        in_ch_layout = state->codec_ctx->ch_layout;
    } else {
        // 如果通道布局未指定，使用默认布局
        av_channel_layout_default(&in_ch_layout, state->codec_ctx->ch_layout.nb_channels);
    }

    // 使用新API设置重采样器参数
    ret = swr_alloc_set_opts2(&state->swr_ctx,
                         &out_ch_layout, AV_SAMPLE_FMT_S16, 44100,
                         &in_ch_layout, state->codec_ctx->sample_fmt, state->codec_ctx->sample_rate,
                         0, NULL);
    if (ret < 0) {
        fprintf(stderr, "无法设置重采样选项\n");
        return -1;
    }

    ret = swr_init(state->swr_ctx);
    if (ret < 0) {
        fprintf(stderr, "无法初始化重采样器\n");
        return -1;
    }

    // 设置ALSA参数
    state->alsa_format = SND_PCM_FORMAT_S16_LE;
    state->alsa_channels = 2;
    state->alsa_sample_rate = 44100;
    state->alsa_period_size = 1024;

    // 获取歌曲时长
    if (state->format_ctx->duration != AV_NOPTS_VALUE) {
        state->duration_sec = (double)state->format_ctx->duration / AV_TIME_BASE;
    } else {
        state->duration_sec = -1.0;
    }

    // 释放临时通道布局
    av_channel_layout_uninit(&in_ch_layout);

    return 0;
}

// 初始化ALSA
int init_alsa(AudioState *state) {
    int ret;

    // 打开ALSA设备
    ret = snd_pcm_open(&state->alsa_handle, "default", SND_PCM_STREAM_PLAYBACK, 0);
    if (ret < 0) {
        fprintf(stderr, "无法打开PCM设备: %s\n", snd_strerror(ret));
        return -1;
    }

    // 设置硬件参数
    snd_pcm_hw_params_t *hw_params;
    snd_pcm_hw_params_alloca(&hw_params);

    ret = snd_pcm_hw_params_any(state->alsa_handle, hw_params);
    if (ret < 0) {
        fprintf(stderr, "无法初始化硬件参数: %s\n", snd_strerror(ret));
        return -1;
    }

    ret = snd_pcm_hw_params_set_access(state->alsa_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
    if (ret < 0) {
        fprintf(stderr, "无法设置访问类型: %s\n", snd_strerror(ret));
        return -1;
    }

    ret = snd_pcm_hw_params_set_format(state->alsa_handle, hw_params, state->alsa_format);
    if (ret < 0) {
        fprintf(stderr, "无法设置样本格式: %s\n", snd_strerror(ret));
        return -1;
    }

    ret = snd_pcm_hw_params_set_channels(state->alsa_handle, hw_params, state->alsa_channels);
    if (ret < 0) {
        fprintf(stderr, "无法设置声道数: %s\n", snd_strerror(ret));
        return -1;
    }

    unsigned int sample_rate = state->alsa_sample_rate;
    ret = snd_pcm_hw_params_set_rate_near(state->alsa_handle, hw_params, &sample_rate, 0);
    if (ret < 0) {
        fprintf(stderr, "无法设置采样率: %s\n", snd_strerror(ret));
        return -1;
    }

    ret = snd_pcm_hw_params_set_period_size_near(state->alsa_handle, hw_params, &state->alsa_period_size, 0);
    if (ret < 0) {
        fprintf(stderr, "无法设置周期大小: %s\n", snd_strerror(ret));
        return -1;
    }

    ret = snd_pcm_hw_params(state->alsa_handle, hw_params);
    if (ret < 0) {
        fprintf(stderr, "无法设置硬件参数: %s\n", snd_strerror(ret));
        return -1;
    }

    return 0;
}

// 解码并播放音频（修改版本，添加停止检查）
void decode_and_play(AudioState *state) {
    int ret;

    while (av_read_frame(state->format_ctx, state->pkt) >= 0 && !state->should_stop) {
        if (state->pkt->stream_index == state->audio_stream_idx) {
            // 发送包到解码器
            ret = avcodec_send_packet(state->codec_ctx, state->pkt);
            if (ret < 0) {
                fprintf(stderr, "发送包到解码器失败\n");
                break;
            }

            while (ret >= 0 && !state->should_stop) {
                // 接收解码后的帧
                ret = avcodec_receive_frame(state->codec_ctx, state->frame);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                    break;
                } else if (ret < 0) {
                    fprintf(stderr, "解码错误\n");
                    break;
                }

                // 重采样
                uint8_t **src_data = state->frame->extended_data;
                int src_nb_samples = state->frame->nb_samples;

                // 计算输出样本数
                int dst_nb_samples = av_rescale_rnd(
                    swr_get_delay(state->swr_ctx, state->frame->sample_rate) + src_nb_samples,
                    44100, state->frame->sample_rate, AV_ROUND_UP);

                // 分配输出缓冲区
                uint8_t *dst_data;
                int dst_linesize;
                ret = av_samples_alloc(&dst_data, &dst_linesize, 2, dst_nb_samples, AV_SAMPLE_FMT_S16, 0);
                if (ret < 0) {
                    fprintf(stderr, "无法分配输出样本\n");
                    break;
                }

                // 执行重采样
                int converted = swr_convert(state->swr_ctx, &dst_data, dst_nb_samples,
                                         (const uint8_t **)src_data, src_nb_samples);
                if (converted < 0) {
                    fprintf(stderr, "重采样失败\n");
                    av_freep(&dst_data);
                    break;
                }

                // 计算实际转换的数据大小
                int data_size = converted * 2 * 2; // 2声道 * 2字节(16位)

                // 写入ALSA
                snd_pcm_sframes_t frames = snd_pcm_writei(state->alsa_handle, dst_data, converted);
                if (frames < 0) {
                    frames = snd_pcm_recover(state->alsa_handle, frames, 0);
                }
                if (frames < 0) {
                    fprintf(stderr, "写入ALSA失败: %s\n", snd_strerror(frames));
                }

                // 释放输出缓冲区
                av_freep(&dst_data);
                
                // 添加小的延迟，避免过度占用CPU
                if (!state->should_stop) {
                    usleep(1000); // 1ms延迟
                }
            }
        }
        av_packet_unref(state->pkt);
        
        if (state->should_stop) break;
    }
}

// 清理资源
void cleanup(AudioState *state) {
    if (!state) return;
    
    if (state->swr_ctx) swr_free(&state->swr_ctx);
    if (state->frame) av_frame_free(&state->frame);
    if (state->pkt) av_packet_free(&state->pkt);
    if (state->codec_ctx) avcodec_free_context(&state->codec_ctx);
    if (state->format_ctx) avformat_close_input(&state->format_ctx);
    if (state->alsa_handle) snd_pcm_close(state->alsa_handle);
}

// 音频播放线程函数
static void* audio_play_thread(void* arg) {
    const char* filename = (const char*)arg;
    
    // 分配音频状态
    g_audio_state = (AudioState*)calloc(1, sizeof(AudioState));
    if (!g_audio_state) {
        free(arg);
        return NULL;
    }
    
    // 初始化FFmpeg
    if (init_ffmpeg(g_audio_state, filename) < 0) {
        cleanup(g_audio_state);
        free(g_audio_state);
        g_audio_state = NULL;
        free(arg);
        return NULL;
    }
    
    // 初始化ALSA
    if (init_alsa(g_audio_state) < 0) {
        cleanup(g_audio_state);
        free(g_audio_state);
        g_audio_state = NULL;
        free(arg);
        return NULL;
    }
    
    g_audio_state->is_playing = 1;
    g_audio_state->should_stop = 0;
    
    // 开始播放
    decode_and_play(g_audio_state);
    
    // 播放完成或停止后的清理
    g_audio_state->is_playing = 0;
    cleanup(g_audio_state);
    free(g_audio_state);
    g_audio_state = NULL;
    
    free(arg);
    return NULL;
}

// 开始播放音频（线程安全）
int start_audio_playback(const char* filename) {
    // 如果正在播放，先停止
    if (g_audio_state && g_audio_state->is_playing) {
        stop_audio_playback();
        // 等待线程结束
        pthread_join(g_audio_state->play_thread, NULL);
    }
    
    char* filename_copy = strdup(filename);
    if (!filename_copy) {
        return -1;
    }
    
    pthread_t thread_id;
    if (pthread_create(&thread_id, NULL, audio_play_thread, filename_copy) == 0) {
        if (g_audio_state) {
            g_audio_state->play_thread = thread_id;
        }
        return 0;
    } else {
        free(filename_copy);
        return -1;
    }
}

// 停止播放音频
void stop_audio_playback(void) {
    if (g_audio_state) {
        g_audio_state->should_stop = 1;
    }
}

// 检查是否正在播放
int is_audio_playing(void) {
    return (g_audio_state && g_audio_state->is_playing);
}

// 获取歌曲总时长（秒）
double get_audio_duration(void) {
    if (!g_audio_state) {
        return -1.0;
    }
    return g_audio_state->duration_sec;
}