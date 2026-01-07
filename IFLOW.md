# v833_lv9_demos 项目说明

## 项目信息

- **项目名称**: v833_lv9_demos
- **目标平台**: 全志 V833 主控芯片
- **操作系统**: Tina Linux
- **Git 仓库**: https://github.com/albert585/v833_lv9_demos
- **LVGL 版本**: 8.4.0（作为 Git 子模块）

## 项目概述

这是一个基于全志 V833 主控芯片的 LVGL 图形界面演示项目，运行在 Tina Linux 系统上。项目使用 LVGL 8.4.0 版本构建图形界面，并集成了音频播放、文件管理、设置、视觉小说引擎等功能模块。

### 主要技术栈
- **GUI 框架**: LVGL 8.4.0（作为 Git 子模块）
- **构建系统**: CMake 3.10+
- **编程语言**: C (C99 标准), C++ (C17 标准)
- **目标平台**: ARM Linux (arm-unknown-linux-musleabihf)
- **显示驱动**: Linux Framebuffer (FBDEV)
- **输入设备**: EVDEV (触摸屏)
- **音频处理**: FFmpeg, ALSA
- **加密库**: OpenSSL

### 核心功能模块
- **多媒体播放器**: 支持音频文件播放、进度控制、音量调节、播放速度控制，基于 FFmpeg 和 ALSA
- **文件管理器**: 浏览和管理文件系统，支持文件选择事件
- **设置界面**: 系统配置和参数调整
- **视觉小说引擎**: 基于 JSON 配置的视觉小说系统，支持背景图、角色图、文本框等多媒体元素，支持网络图片资源
- **按键处理**: Home 键和 Power 键的特殊功能（双击 Home 切换前台/后台，Power 键控制睡眠/唤醒）
- **电源管理**: 支持浅睡眠和深睡眠模式，自动切换机制
- **机器人模式**: 切换到机器人运行模式，关闭当前应用并启动机器人程序

## 架构设计

### 系统集成
项目采用模块化架构，各功能模块通过事件系统进行交互：
1. **主循环**（main.c）：负责 LVGL 初始化、设备初始化和主事件循环
2. **容器系统**（container.c/h）：提供主界面容器，管理各功能模块的显示/隐藏
3. **事件系统**（events.c/h）：统一处理所有用户交互事件，协调各模块切换
4. **功能模块**：各功能模块独立实现，通过事件回调与主系统集成

### 视觉小说引擎集成
视觉小说引擎作为独立模块集成到主系统中：
- 通过 `event_open_visual_novel` 事件启动，隐藏主容器并初始化引擎
- 通过 `event_close_visual_novel` 事件关闭，释放资源并显示主容器
- 使用 JSON 配置文件（`src/lib/virsual_novel/data/story.json`）定义故事内容
- 支持网络图片资源，实现动态内容加载

## 项目结构

```
v833_lv9_demos/
├── src/                      # 主程序源代码
│   ├── main.c               # 主程序入口
│   ├── main.h               # 主程序头文件
│   └── lib/                 # 核心功能库
│       ├── audio.c/h        # 音频播放功能
│       ├── button.c/h       # 按钮组件
│       ├── container.c/h    # 容器组件
│       ├── events.c/h       # 事件处理
│       ├── file_manager.c/h # 文件管理器
│       ├── player.c/h       # 播放器组件
│       ├── settings.c/h     # 设置界面
│       └── virsual_novel/   # 视觉小说引擎
│           ├── visual_novel_engine.c/h  # 核心引擎
│           ├── resource_manager.c/h    # 资源管理器
│           ├── data_parser.c/h         # JSON 数据解析器
│           ├── cJSON.c/h               # JSON 解析库
│           ├── simple_json.c/h         # 简单 JSON 实现
│           ├── test_parser.c           # 解析器测试
│           ├── data/                   # 故事数据目录
│           │   └── story.json          # 故事配置文件
│           └── README.md               # 引擎说明文档
├── lvgl/                    # LVGL 源码（Git 子模块）
├── lv_drivers/              # LVGL 驱动（Git 子模块）
├── config/                  # 不同设备的配置文件
│   ├── t01pro/             # T01Pro 设备配置
│   └── t3/                 # T3 设备配置
├── scripts/                 # 脚本文件
│   └── back.sh             # 后台返回脚本
├── bin/                     # 二进制输出目录
├── build/                   # 构建输出目录
├── include/                 # 头文件目录
│   └── sunxi_display2.h    # 显示驱动头文件
├── CMakeLists.txt           # CMake 构建配置
├── user_cross_compile_setup.cmake  # 交叉编译工具链配置
├── lv_conf.h               # LVGL 配置文件
├── lv_drv_conf.h           # LVGL 驱动配置文件
├── Makefile                # Makefile 构建配置
├── test_ffmpeg_simple.c    # FFmpeg 最小测试程序
├── test_ffmpeg_alsa.c      # FFmpeg + ALSA 集成测试程序
├── test_audio_direct.c     # 音频直接测试程序
└── README.md               # 项目说明文档
```

## 构建和运行

### 环境要求
- CMake 3.10 或更高版本
- ARM 交叉编译工具链: `/usr/arm-unknown-linux-musleabihf/`
- 依赖库: evdev, libdrm, gbm, libinput, freetype2, SDL2 (可选), FFmpeg, ALSA, OpenSSL, zlib

### 构建步骤

1. **克隆项目（包含子模块）**
   ```bash
   git clone --recursive https://github.com/albert585/v833_lv9_demos
   cd v833_lv9_demos
   ```

2. **配置 CMake 构建环境**
   ```bash
   cmake -DCMAKE_TOOLCHAIN_FILE=./user_cross_compile_setup.cmake -B build -S .
   ```

3. **编译项目**
   ```bash
   make -C build -j$(nproc)
   ```

4. **运行程序（在目标设备上）**
   ```bash
   ./build/bin/lvglsim
   ```

### 构建目标
- `lvglsim`: 主可执行文件，位于 `build/bin/` 目录
- `lvgl_linux`: 静态库（包含自定义 LVGL 扩展）
- `lvgl`: LVGL 核心库
- `lvgl_examples`: LVGL 示例库
- `lvgl_demos`: LVGL 演示库
- `test_ffmpeg_simple`: FFmpeg 最小测试程序（不依赖 ALSA）
- `test_ffmpeg_alsa`: FFmpeg + ALSA 集成测试程序
- `test_audio_direct`: 音频直接测试程序
- `run`: 构建并运行（仅用于本地测试）
- `all`: 构建所有目标（默认）
- `clean`: 清理构建产物

### 清理构建
```bash
make -C build clean
```

## 视觉小说引擎

### 概述
视觉小说引擎是基于 LVGL 8.4.0 构建的独立模块，通过 JSON 配置文件管理图片路径和文字内容，实现灵活的视觉小说制作与展示。

### 主要功能
- **JSON 配置驱动**: 通过 JSON 文件定义每页的背景图、角色图、文字内容、文本框样式等
- **动态资源加载**: 支持本地文件路径和网络 URL 图片资源
- **页面管理**: 支持多页视觉小说内容切换，每页可包含多个角色
- **状态管理**: 引擎状态包括 IDLE、INIT、RUNNING、PAUSED、FINISHED
- **交互控制**: 支持点击或按键切换页面，实现故事推进
- **资源管理**: 引用计数机制管理资源生命周期，自动释放未使用资源

### JSON 配置格式
```json
{
  "title": "故事标题",
  "author": "作者",
  "pages": [
    {
      "id": "page1",
      "background": "背景图片路径或URL",
      "characters": [
        {
          "id": "char1",
          "image": "角色图片路径或URL",
          "x": 100,
          "y": 200,
          "scale": 1.0,
          "visible": true
        }
      ],
      "text": "页面文字内容",
      "textbox": {
        "visible": true,
        "x": 50,
        "y": 400,
        "width": 700,
        "height": 150,
        "bg_color": "#000000",
        "text_color": "#FFFFFF",
        "font_size": 16
      },
      "next_page": "page2"
    }
  ]
}
```

### API 接口
- `vn_engine_init(json_path)`: 初始化引擎，加载 JSON 配置
- `vn_engine_start()`: 启动视觉小说引擎
- `vn_engine_pause()`: 暂停引擎
- `vn_engine_resume()`: 恢复引擎
- `vn_engine_stop()`: 停止引擎
- `vn_engine_load_page(page_id)`: 加载指定页面
- `vn_engine_load_next_page()`: 加载下一页
- `vn_engine_get_state()`: 获取当前引擎状态
- `vn_engine_free_current_resources()`: 释放当前页面资源
- `vn_engine_deinit()`: 反初始化引擎，释放资源

### 使用示例
```c
// 打开视觉小说
event_open_visual_novel(e);

// 关闭视觉小说
event_close_visual_novel(e);
```

### 扩展功能（待实现）
- 多语言支持
- 音效支持
- 选择分支功能
- 动画效果
- 存档/读档功能

## 开发约定

### 代码风格
- 使用 C99 标准编写 C 代码
- 使用 C17 标准编写 C++ 代码
- 编译选项包含 `-Wall -Wextra -Wpedantic` 以启用严格警告
- 函数和变量命名采用小写加下划线的方式（snake_case）

### 目录组织
- `src/main.c`: 主程序入口，负责 LVGL 初始化、显示和输入设备设置、主循环
- `src/lib/`: 所有功能模块的源代码和头文件
- 每个功能模块成对出现 `.c` 和 `.h` 文件

### 模块化设计
- **audio**: 封装 FFmpeg 音频解码和 ALSA 播放功能，支持播放速度控制
  - API: `audio_player_init`, `audio_player_open`, `audio_player_play`, `audio_player_pause`, `audio_player_stop`, `audio_player_set_volume`, `audio_player_set_position`, `audio_player_set_speed`, `audio_player_deinit`
  - 额外 API: `audio_player_get_position`, `audio_player_get_duration`
- **player**: 基于 audio 模块构建的高级播放器 UI 组件，包含进度条、音量控制等
  - API: `player_create`, `player_set_file`, `player_toggle_play_pause`, `player_stop`, `player_get_state`, `player_get_position_pct`, `player_destroy`
  - 额外 API: `player_preinit_alsa`, `player_destroy_callback`
- **file_manager**: 文件浏览和管理功能，支持文件选择事件
- **settings**: 系统设置界面
- **button/container/events**: UI 组件和事件处理系统
- **virsual_novel**: 独立的视觉小说引擎模块，包含：
  - **visual_novel_engine**: 核心引擎，管理页面切换和渲染
  - **resource_manager**: 资源加载和管理，支持引用计数
  - **data_parser**: JSON 配置文件解析
  - **cJSON**: JSON 解析库
  - **simple_json**: 简单 JSON 实现

### 硬件相关代码
- 显示设备: `/dev/fb0` (framebuffer), `/dev/disp` (display 控制器)
- 输入设备: `/dev/input/event0` (触摸屏), `/dev/input/event1` (电源键), `/dev/input/event2` (Home 键)
- 触摸屏校准参数: x=-40, y=940, x_max=310, y_max=25

### 事件系统
系统事件处理模块（events.c/h）提供以下功能：
- **event_open_manager**: 打开文件管理器
- **event_close_manager**: 关闭文件管理器和设置界面
- **event_open_settings**: 打开设置界面
- **btn_robot_click**: 切换到机器人运行模式
- **file_select_event**: 文件选择事件处理
- **slider_event_cb**: 滑块事件回调
- **event_open_visual_novel**: 打开视觉小说引擎
- **event_close_visual_novel**: 关闭视觉小说引擎
- **event_close_player**: 关闭播放器
- **event_audio_test**: 音频测试
- **player_destroy_callback**: 播放器销毁回调

### 电源管理
- **浅睡眠**: 关闭 LCD 和触摸屏，60 秒后自动进入深睡眠
- **深睡眠**: 写入 `/sys/power/state` 进入内存睡眠模式，按电源键可唤醒
- **唤醒**: 通过 Power 键唤醒，重新打开 LCD 和触摸屏
- **后台模式**: 双击 Home 键进入后台，5 分钟后自动切回前台

### 机器人模式
- **switchRobot()**: 切换到机器人运行模式
  - 切换到后台模式
  - 关闭显示和输入设备
  - 终止 WiFi 连接（wpa_supplicant）
  - 启动机器人程序（robot_run_1）
- **switchForeground()**: 从后台切回前台
  - 切换到原工作目录
  - 执行返回脚本（/mnt/app/back.sh）
- 主程序启动时会自动终止现有的 robotd 和 robot_run_1 进程

### 依赖库路径
- 头文件路径:
  - `/srv/alsa/include`: ALSA 音频库头文件
  - `/srv/ffmpeg/include`: FFmpeg 音视频编解码库头文件
  - `/srv/zlib-armv7/include`: zlib 压缩库头文件
- 库文件路径:
  - `/srv/evdev/lib`: Linux 输入设备事件库
  - `/srv/openssl/lib`: OpenSSL 加密库
  - `/srv/zlib/lib`: zlib 压缩库
  - `/srv/ffmpeg/lib`: FFmpeg 库（avcodec, avutil, avformat, swscale, swresample）
  - `/srv/alsa/lib`: ALSA 音频库

### 链接库
编译时会链接以下库：
- evdev: 输入设备事件处理
- ssl, crypto: OpenSSL 加密功能
- avcodec, avutil, avformat, swscale, swresample: FFmpeg 音视频编解码
- m: 数学库
- z: zlib 压缩库
- pthread: 多线程支持
- asound: ALSA 音频库
- g2d: 2D 图形加速（如果启用 CONFIG_LV_USE_DRAW_G2D）

## 配置文件

### LVGL 配置 (lv_conf.h)
- 颜色深度: 32-bit (ARGB8888)
- 内存池大小: 5MB
- 默认字体: Montserrat 14
- 支持的 LVGL 组件: 大部分基础组件已启用
- 渲染引擎: 软件渲染 (LV_USE_DRAW_SW)
- 操作系统: LV_OS_NONE（无操作系统支持）
- 文本编码: UTF-8
- 字符集: ASCII（支持阿拉伯语/波斯语处理可选）
- 双向文本: 禁用（LV_USE_BIDI = 0）
- FFmpeg 支持: 已启用（LV_USE_FFMPEG = 1）
- 文件系统: POSIX 文件系统支持已启用（LV_USE_FS_POSIX = 1）

### LVGL 驱动配置 (lv_drv_conf.h)
- **FBDEV**: 已启用（USE_FBDEV = 1），设备路径 `/dev/fb0`
- **EVDEV**: 已启用（USE_EVDEV = 1），设备路径 `/dev/input/event0`
- 其他驱动（SDL、DRM、libinput 等）均未启用

### 可选后端支持
CMakeLists.txt 支持多种后端（通过 CONFIG_LV_USE_* 宏控制）：
- **EVDEV**: Linux 输入设备事件支持
- **Linux DRM**: Direct Rendering Manager 显示支持
- **GBM**: Generic Buffer Manager 支持
- **libinput**: 高级输入设备支持
- **Freetype**: 字体渲染支持
- **SDL2**: 跨平台图形和输入支持
- **Wayland**: Wayland 显示协议支持
- **X11**: X Window System 支持
- **OpenGLES**: OpenGL ES 加速支持
- **G2D**: NXP G2D 硬件加速支持

### 设备配置
- `config/t01pro/`: T01Pro 设备的 LVGL 和驱动配置
- `config/t3/`: T3 设备的 LVGL 和驱动配置

## 测试和调试

### 视觉小说引擎测试
- **test_parser.c**: JSON 解析器测试程序
- **数据文件**: `src/lib/virsual_novel/data/story.json` 包含示例故事（"樱花季节的回忆"）

### 音频测试程序
- **test_ffmpeg_simple.c**: FFmpeg 最小测试程序（不依赖 ALSA）
  - 用于测试 FFmpeg 基本解码功能
  - 编译目标: `test_ffmpeg_simple`
- **test_ffmpeg_alsa.c**: FFmpeg + ALSA 集成测试程序
  - 测试完整的音频播放流程
  - 编译目标: `test_ffmpeg_alsa`
- **test_audio_direct.c**: 音频直接测试程序
  - 直接测试音频模块功能
  - 编译目标: `test_audio_direct`

### 调试输出
- 主程序包含关键操作的 printf 输出：
  - `[key]home_up/home_down`: Home 键事件
  - `[lcd]opened/closed`: LCD 显示开关
  - `[tp]opened/closed`: 触摸屏开关
  - `display OK!`: 显示初始化成功
  - `init OK`: 系统初始化完成

### 构建调试
- 编译选项包含 `-Wall -Wextra -Wpedantic -g` 以启用严格警告和调试信息
- 可通过修改 CMakeLists.txt 中的 `set(CMAKE_BUILD_TYPE Debug)` 启用调试模式

## 开发工作流

### 修改代码后重新编译
```bash
# 清理并重新编译
make -C build clean
cmake -DCMAKE_TOOLCHAIN_FILE=./user_cross_compile_setup.cmake -B build -S .
make -C build -j$(nproc)
```

### 部署到目标设备
```bash
# 将可执行文件传输到目标设备
scp build/bin/lvglsim user@device:/path/to/destination/

# 在目标设备上运行
ssh user@device
./path/to/destination/lvglsim
```

### Git 工作流
```bash
# 拉取最新代码（包含子模块）
git pull --recurse-submodules

# 更新子模块
git submodule update --remote

# 添加新文件
git add src/lib/new_module.c
git add src/lib/new_module.h

# 提交更改
git commit -m "Add new module feature"

# 推送到远程仓库
git push
```

## 注意事项

1. **交叉编译**: 项目使用 musl libc 的 ARM 工具链，确保工具链路径正确
2. **Git 子模块**: LVGL 作为子模块包含，首次克隆时使用 `--recursive` 参数
3. **硬件依赖**: 程序依赖特定的硬件设备节点，只能在目标 V833 设备上运行
4. **显示旋转**: 默认显示旋转 90 度（代码注释中提及，但实际未启用）
5. **触摸校准**: 触摸屏坐标需要校准以匹配显示旋转后的坐标
6. **网络资源**: 视觉小说引擎支持网络图片资源，需要设备有网络连接
7. **进程管理**: 主程序启动时会终止现有的 robotd 和 robot_run_1 进程
8. **电源管理**: 深睡眠模式需要 RTC 唤醒支持，写入 `/sys/class/rtc/rtc0/wakealarm`
9. **内存限制**: LVGL 内存池大小为 5MB，注意内存使用
10. **编译警告**: 项目使用严格编译选项（-Wall -Wextra -Wpedantic），确保代码质量
11. **LVGL 版本**: 当前使用 LVGL 8.4.0，注意 API 兼容性
12. **颜色深度**: 使用 32-bit 颜色深度（ARGB8888），而非 16-bit

## 已知问题

- README 中提到有大量 TODO 待完成
- 视觉小说引擎的扩展功能（多语言、音效、选择分支、动画效果、存档/读档）尚未实现
- 文件选择事件处理逻辑（file_select_event）中的 TODO 尚未完成
- 字体加载功能尚未完全实现，当前使用 LVGL 默认字体
- 显示旋转功能在代码注释中提及但未实际启用

## 未来计划

### 短期目标
- 完成文件选择事件处理逻辑
- 实现视觉小说引擎的存档/读档功能
- 添加音效支持
- 优化内存使用

### 中期目标
- 实现视觉小说引擎的选择分支功能
- 添加动画效果支持
- 实现多语言支持
- 优化图片加载性能
- 启用显示旋转功能

### 长期目标
- 添加视频播放支持
- 实现网络流媒体播放
- 添加更多 UI 组件和主题
- 支持更多硬件平台

## 贡献指南

欢迎贡献代码和改进建议！请遵循以下指南：

1. **代码风格**: 遵循项目现有的代码风格（C99 标准，snake_case 命名）
2. **提交信息**: 使用清晰简洁的提交信息，说明更改的目的
3. **测试**: 在提交前确保代码在目标设备上正常工作
4. **文档**: 更新相关文档和注释
5. **问题报告**: 使用 GitHub Issues 报告 bug 和提出功能请求

## 许可证

请参考项目根目录下的 LICENSE 文件了解许可证信息。

## 联系方式

- 项目主页: https://github.com/albert585/v833_lv9_demos
- 问题反馈: 通过 GitHub Issues