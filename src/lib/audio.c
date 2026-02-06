#include "audio.h"
#include <pthread.h>
#include <libavdevice/avdevice.h>
#include <alsa/asoundlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static pthread_t audio_thread = 0;
static volatile int is_playing = 0;
static volatile int is_paused = 0;
static AVFormatContext *out_fmt_ctx = NULL;
static snd_mixer_t *mixer_handle = NULL;
static snd_mixer_elem_t *mixer_elem = NULL;

// ALSA PCM 句柄（直接输出）
static snd_pcm_t *pcm_handle = NULL;
static pthread_mutex_t pcm_mutex = PTHREAD_MUTEX_INITIALIZER;

// ALSA PCM 初始化（备用方案）
static int audio_pcm_init(void) {
    int err;
    snd_pcm_hw_params_t *hw_params;
    unsigned int rate = 44100;
    int channels = 2;
    int dir;
    snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE;
    
    if (pcm_handle) {
        return 0; // 已经初始化
    }
    
    // 打开 PCM 设备
    err = snd_pcm_open(&pcm_handle, "default", SND_PCM_STREAM_PLAYBACK, 0);
    if (err < 0) {
        printf("[audio] Error opening PCM device: %s\n", snd_strerror(err));
        return -1;
    }
    
    // 分配硬件参数结构
    snd_pcm_hw_params_alloca(&hw_params);
    
    // 初始化硬件参数
    err = snd_pcm_hw_params_any(pcm_handle, hw_params);
    if (err < 0) {
        printf("[audio] Error initializing hardware parameters: %s\n", snd_strerror(err));
        snd_pcm_close(pcm_handle);
        pcm_handle = NULL;
        return -1;
    }
    
    // 设置访问类型（交错模式）
    err = snd_pcm_hw_params_set_access(pcm_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
    if (err < 0) {
        printf("[audio] Error setting access type: %s\n", snd_strerror(err));
        snd_pcm_close(pcm_handle);
        pcm_handle = NULL;
        return -1;
    }
    
    // 设置采样格式（16位小端）
    err = snd_pcm_hw_params_set_format(pcm_handle, hw_params, format);
    if (err < 0) {
        printf("[audio] Error setting sample format: %s\n", snd_strerror(err));
        // 尝试其他格式
        if (snd_pcm_hw_params_set_format(pcm_handle, hw_params, SND_PCM_FORMAT_S16_BE) >= 0) {
            format = SND_PCM_FORMAT_S16_BE;
            printf("[audio] Using alternative format: S16_BE\n");
        } else if (snd_pcm_hw_params_set_format(pcm_handle, hw_params, SND_PCM_FORMAT_U16_LE) >= 0) {
            format = SND_PCM_FORMAT_U16_LE;
            printf("[audio] Using alternative format: U16_LE\n");
        } else {
            printf("[audio] No suitable format found\n");
            snd_pcm_close(pcm_handle);
            pcm_handle = NULL;
            return -1;
        }
    }
    
    // 设置通道数
    err = snd_pcm_hw_params_set_channels(pcm_handle, hw_params, channels);
    if (err < 0) {
        printf("[audio] Error setting channels: %s\n", snd_strerror(err));
        snd_pcm_close(pcm_handle);
        pcm_handle = NULL;
        return -1;
    }
    
    // 设置采样率
    err = snd_pcm_hw_params_set_rate_near(pcm_handle, hw_params, &rate, &dir);
    if (err < 0) {
        printf("[audio] Error setting sample rate: %s\n", snd_strerror(err));
        snd_pcm_close(pcm_handle);
        pcm_handle = NULL;
        return -1;
    }
    
    // 应用硬件参数
    err = snd_pcm_hw_params(pcm_handle, hw_params);
    if (err < 0) {
        printf("[audio] Error setting hardware parameters: %s\n", snd_strerror(err));
        snd_pcm_close(pcm_handle);
        pcm_handle = NULL;
        return -1;
    }
    
    printf("[audio] PCM initialized successfully (rate=%u, channels=%d, format=%s)\n", 
           rate, channels, snd_pcm_format_name(format));
    return 0;
}

// ALSA PCM 写入
static int audio_pcm_write(const uint8_t *data, int size) {
    int err;
    int frames = size / 4; // 16位双通道，每帧4字节
    
    pthread_mutex_lock(&pcm_mutex);
    if (!pcm_handle) {
        pthread_mutex_unlock(&pcm_mutex);
        return -1;
    }
    
    err = snd_pcm_writei(pcm_handle, data, frames);
    pthread_mutex_unlock(&pcm_mutex);
    
    if (err == -EPIPE) {
        // 缓冲区下溢，需要恢复
        printf("[audio] Buffer underrun, recovering...\n");
        pthread_mutex_lock(&pcm_mutex);
        snd_pcm_prepare(pcm_handle);
        pthread_mutex_unlock(&pcm_mutex);
        err = snd_pcm_writei(pcm_handle, data, frames);
    }
    
    if (err < 0) {
        printf("[audio] Error writing to PCM: %s\n", snd_strerror(err));
        return -1;
    }
    
    return 0;
}

// ALSA PCM 清理
static void audio_pcm_deinit(void) {
    pthread_mutex_lock(&pcm_mutex);
    if (pcm_handle) {
        snd_pcm_drain(pcm_handle);
        snd_pcm_close(pcm_handle);
        pcm_handle = NULL;
    }
    pthread_mutex_unlock(&pcm_mutex);
}

// ALSA Mixer 初始化
int audio_mixer_init(void) {
    int err;
    
    if (mixer_handle) {
        return 0; // 已经初始化
    }
    
    // 打开 Mixer
    err = snd_mixer_open(&mixer_handle, 0);
    if (err < 0) {
        printf("[audio] Error opening mixer: %s\n", snd_strerror(err));
        return -1;
    }
    
    // 附加到默认声卡
    err = snd_mixer_attach(mixer_handle, "default");
    if (err < 0) {
        printf("[audio] Error attaching mixer: %s\n", snd_strerror(err));
        snd_mixer_close(mixer_handle);
        mixer_handle = NULL;
        return -1;
    }
    
    // 注册 mixer 元素
    err = snd_mixer_selem_register(mixer_handle, NULL, NULL);
    if (err < 0) {
        printf("[audio] Error registering mixer: %s\n", snd_strerror(err));
        snd_mixer_close(mixer_handle);
        mixer_handle = NULL;
        return -1;
    }
    
    // 加载 mixer
    err = snd_mixer_load(mixer_handle);
    if (err < 0) {
        printf("[audio] Error loading mixer: %s\n", snd_strerror(err));
        snd_mixer_close(mixer_handle);
        mixer_handle = NULL;
        return -1;
    }
    
    // 查找 PCM 播放音量控制元素
    for (mixer_elem = snd_mixer_first_elem(mixer_handle); mixer_elem; 
         mixer_elem = snd_mixer_elem_next(mixer_elem)) {
        if (snd_mixer_selem_has_playback_volume(mixer_elem)) {
            printf("[audio] Found playback volume control: %s\n", 
                   snd_mixer_selem_get_name(mixer_elem));
            break;
        }
    }
    
    if (!mixer_elem) {
        printf("[audio] No playback volume control found\n");
        snd_mixer_close(mixer_handle);
        mixer_handle = NULL;
        return -1;
    }
    
    printf("[audio] Mixer initialized successfully\n");
    return 0;
}

// 设置 Mixer 音量
static int audio_mixer_set_volume(int volume) {
    long min, max, value;
    
    if (!mixer_handle || !mixer_elem) {
        printf("[audio] Mixer not initialized\n");
        return -1;
    }
    
    // 获取音量范围
    snd_mixer_selem_get_playback_volume_range(mixer_elem, &min, &max);
    
    // 计算音量值 (0-100 映射到 min-max)
    value = min + (max - min) * volume / 100;
    
    // 设置左右声道音量
    snd_mixer_selem_set_playback_volume_all(mixer_elem, value);
    
    return 0;
}

// 获取 Mixer 音量
int audio_mixer_get_volume(void) {
    long min, max, value;
    
    if (!mixer_handle || !mixer_elem) {
        return 75; // 默认值
    }
    
    snd_mixer_selem_get_playback_volume_range(mixer_elem, &min, &max);
    snd_mixer_selem_get_playback_volume(mixer_elem, SND_MIXER_SCHN_FRONT_LEFT, &value);
    
    // 映射回 0-100 范围
    // 防止除以零：如果 max == min，返回默认值 75
    if (max == min) {
        printf("[audio] Warning: Volume range is zero (min=%ld, max=%ld), using default volume\n", min, max);
        return 75;
    }
    
    int volume = (value - min) * 100 / (max - min);
    
    // 确保返回值在 0-100 范围内
    if (volume < 0) volume = 0;
    if (volume > 100) volume = 100;
    
    return volume;
}

// 清理 Mixer
static void audio_mixer_deinit(void) {
    if (mixer_handle) {
        snd_mixer_close(mixer_handle);
        mixer_handle = NULL;
        mixer_elem = NULL;
    }
}

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
            
            // 检查帧是否有效
            if (!player->frame || !player->frame->data[0] || player->frame->nb_samples <= 0) {
                printf("[audio] Warning: Invalid frame received, skipping\n");
                continue;
            }
            
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
                    printf("[audio] Error: Failed to allocate audio buffer\n");
                    av_frame_unref(player->frame);
                    continue; // 跳过这一帧
                }
                audio_buf = new_buf;
            }

            int out_samples = swr_convert(
                player->swr_ctx, &audio_buf, dst_nb_samples,
                (const uint8_t **)player->frame->data, player->frame->nb_samples
            );

            // 检查重采样是否成功
            if (out_samples < 0) {
                printf("[audio] Error: swr_convert failed\n");
                av_frame_unref(player->frame);
                continue;
            }

            // 调试：打印帧信息
            static int frame_count = 0;
            if (frame_count < 5) {
                printf("[audio] Frame %d: pts=%lld, dts=%lld, nb_samples=%d, out_samples=%d\n",
                       frame_count, (long long)player->frame->pts, 
                       (long long)player->frame->pkt_dts, 
                       player->frame->nb_samples, out_samples);
                frame_count++;
            }

            // 使用 ALSA PCM 直接输出音频
#if USE_AVDEVICE
            // 使用 avdevice 输出音频
            pthread_mutex_lock(&out_fmt_ctx_mutex);
            if (out_fmt_ctx) {
                // 对于 AVFMT_NOFILE 设备（如 ALSA），pb 可以为 NULL
                if ((out_fmt_ctx->oformat->flags & AVFMT_NOFILE) || out_fmt_ctx->pb) {
                    AVPacket *out_pkt = av_packet_alloc();
                    if (out_pkt) {
                        // 创建新的缓冲区，避免缓存指针问题
                        int buf_size = out_samples * 2 * 2; // 16位双通道
                        out_pkt->data = av_malloc(buf_size);
                        if (out_pkt->data) {
                            memcpy(out_pkt->data, audio_buf, buf_size);
                            out_pkt->size = buf_size;
                            out_pkt->stream_index = 0;
                            out_pkt->pts = player->frame->pts;
                            out_pkt->dts = player->frame->pkt_dts;
                            out_pkt->duration = player->frame->duration;

                            // 使用 av_write_frame 而不是 av_interleaved_write_frame
                            // av_write_frame 不会释放 packet 的数据
                            int ret = av_write_frame(out_fmt_ctx, out_pkt);
                            if (ret < 0) {
                                printf("[audio] Error writing frame: %s (pts=%lld, size=%d)\n",
                                       av_err2str(ret), (long long)out_pkt->pts, out_pkt->size);
                            }
                        } else {
                            printf("[audio] Error: Failed to allocate packet data\n");
                        }
                        av_packet_free(&out_pkt);
                    } else {
                        printf("[audio] Error: Failed to allocate packet\n");
                    }
                } else {
                    printf("[audio] Error: Output context not initialized (pb is NULL for non-NOFILE device)\n");
                }
            } else {
                printf("[audio] Error: Output context not initialized (out_fmt_ctx is NULL)\n");
            }
            pthread_mutex_unlock(&out_fmt_ctx_mutex);
#else
            // 使用 ALSA PCM 直接输出音频
            int ret = audio_pcm_write(audio_buf, out_samples * 2 * 2);
            if (ret < 0) {
                printf("[audio] Error writing to PCM device\n");
            }
#endif
            
            // 添加小延迟，避免过快写入
            usleep(1000);
            av_frame_unref(player->frame);
        }
        av_packet_unref(player->pkt);
    }

    free(audio_buf);
    return NULL;
}

// 定义是否使用 avdevice（0 = 使用 ALSA PCM，1 = 使用 avdevice）
#define USE_AVDEVICE 0

audio_player_t *audio_player_init(lv_obj_t *volume_slider) {
    audio_player_t *player = malloc(sizeof(audio_player_t));
    memset(player, 0, sizeof(audio_player_t));
    player->volume_slider = volume_slider;
    player->volume = 75;
    player->volume_min = 0;
    player->volume_max = 100;
    player->playback_speed = 1.0f;

#if USE_AVDEVICE
    printf("[audio] Initializing audio player with avdevice...\n");
#else
    printf("[audio] Initializing audio player with ALSA PCM...\n");
#endif

    // 初始化 ALSA Mixer（用于音量控制）
    if (audio_mixer_init() < 0) {
        printf("[audio] Warning: Failed to initialize mixer, volume control may not work\n");
    }

#if USE_AVDEVICE
    // 初始化 avdevice
    avdevice_register_all();

    // 打开 ALSA 输出设备
    int ret = avformat_alloc_output_context2(&out_fmt_ctx, NULL, "alsa", "default");
    if (ret < 0 || !out_fmt_ctx) {
        printf("[audio] Error creating output context: %s\n", av_err2str(ret));
        audio_mixer_deinit();
        free(player);
        return NULL;
    }

    // 设置音频参数
    AVStream *out_stream = avformat_new_stream(out_fmt_ctx, NULL);
    if (!out_stream) {
        printf("[audio] Error creating output stream\n");
        avformat_free_context(out_fmt_ctx);
        audio_mixer_deinit();
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
            audio_mixer_deinit();
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
        audio_mixer_deinit();
        free(player);
        return NULL;
    }

    // 设置初始音量
    audio_mixer_set_volume(player->volume);

    printf("[audio] Audio player initialized successfully with avdevice\n");
#else
    // 初始化 ALSA PCM（直接输出）
    if (audio_pcm_init() < 0) {
        printf("[audio] Error: Failed to initialize ALSA PCM\n");
        audio_mixer_deinit();
        free(player);
        return NULL;
    }

    printf("[audio] Audio player initialized successfully with ALSA PCM\n");
#endif

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

    // 输出设备已经在 audio_player_init 中初始化（ALSA PCM）
    // 不需要额外的检查

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

    // 停止播放标志
    is_playing = 0;
    is_paused = 0;

    // 等待播放线程结束（确保线程不再使用任何资源）
    if (audio_thread != 0) {
        pthread_join(audio_thread, NULL);
        audio_thread = 0;
    }

    // 现在可以安全地清理资源
    if (player->pkt) av_packet_unref(player->pkt);
    if (player->frame) av_frame_unref(player->frame);

    // 刷新输出缓冲区（在线程停止后）
    if (out_fmt_ctx) {
        av_write_trailer(out_fmt_ctx);
    }
}

void audio_player_set_volume(audio_player_t *player, int volume) {
    player->volume = LV_CLAMP(player->volume_min, volume, player->volume_max);
    
    // 使用 ALSA Mixer 控制硬件音量
    audio_mixer_set_volume(player->volume);
    
    if (player->volume_slider) {
        lv_slider_set_value(player->volume_slider, player->volume, LV_ANIM_ON);
    }
}

int audio_player_get_position(audio_player_t *player) {
    if (!player->fmt_ctx || !player->stream) return 0;
    
    // 使用 frame->pts 而不是 pkt->dts，因为 frame->pts 是显示时间戳
    if (player->frame && player->frame->pts != AV_NOPTS_VALUE) {
        int64_t pts = player->frame->pts;
        int64_t start_time = player->fmt_ctx->start_time;
        int64_t position = av_rescale_q(pts - start_time, 
                                         player->stream->time_base, 
                                         AV_TIME_BASE_Q);
        
        // 调试：打印计算过程
        static int debug_count = 0;
        if (debug_count < 3) {
            printf("[audio] Position calculation: pts=%lld, start_time=%lld, time_base=%d/%d, position=%lldms\n",
                   (long long)pts, (long long)start_time,
                   player->stream->time_base.num, player->stream->time_base.den,
                   (long long)(position / 1000));
            debug_count++;
        }
        
        return (int)(position / 1000);
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
    
#if USE_AVDEVICE
    // 清理 avdevice 输出
    if (out_fmt_ctx) {
        av_write_trailer(out_fmt_ctx);
        if (!(out_fmt_ctx->oformat->flags & AVFMT_NOFILE)) {
            avio_closep(&out_fmt_ctx->pb);
        }
        avformat_free_context(out_fmt_ctx);
        out_fmt_ctx = NULL;
    }
#else
    // 清理 ALSA PCM
    audio_pcm_deinit();
#endif
    
    // 清理 ALSA Mixer
    audio_mixer_deinit();
    
    free(player);
}