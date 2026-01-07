# 基于LVGL 9.2的视觉小说引擎

这是一个基于LVGL 9.2的视觉小说引擎，通过JSON配置文件管理图片路径和文字内容，实现灵活的视觉小说制作与展示。

## 功能特点

- **JSON配置**：通过JSON文件定义每页的图片路径、文字内容、角色位置等信息
- **动态资源加载**：根据JSON配置动态加载图片资源，无需重新编译即可更新内容
- **页面管理**：支持多页视觉小说内容切换，每页包含背景图、角色图和文字内容
- **交互控制**：支持点击或按键切换页面，实现故事推进

## 项目结构

```
visual_novel/
├── main.c                 # 主程序入口
├── visual_novel_engine.c  # 视觉小说引擎核心代码
├── visual_novel_engine.h  # 引擎头文件
├── resource_manager.c     # 资源管理器
├── resource_manager.h     # 资源管理器头文件
├── data_parser.c          # JSON解析器
├── data_parser.h          # JSON解析器头文件
├── assets/                # 资源文件夹
│   ├── backgrounds/       # 背景图片
│   ├── characters/        # 角色图片
│   └── images/            # 其他图片资源
└── data/                  # JSON数据文件夹
    └── story.json         # 故事配置文件
```

## JSON配置文件格式

```json
{
  "title": "樱花季节的回忆",
  "author": "视觉小说开发者",
  "pages": [
    {
      "id": "page1",
      "background": "背景图片路径",
      "characters": [
        {
          "id": "char1",
          "image": "角色图片路径",
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

## 编译和运行

### 依赖项

- LVGL 9.2
- 标准C库

### 编译命令

```bash
gcc -o visual_novel main.c visual_novel_engine.c resource_manager.c data_parser.c -I/path/to/lvgl/include -L/path/to/lvgl/lib -llvgl
```

### 运行

```bash
./visual_novel
```

## 使用说明

1. 准备背景图片和角色图片资源
2. 创建或修改`data/story.json`文件，定义故事内容
3. 编译并运行程序
4. 点击屏幕或按确认键切换页面

## 注意事项

- 当前版本的JSON解析器是模拟实现的，实际应用中需要集成cJSON等JSON解析库
- 图片资源支持本地文件路径和网络URL
- 字体加载功能尚未完全实现，当前使用LVGL默认字体

## 扩展功能（待实现）

- 多语言支持
- 音效支持
- 选择分支功能
- 动画效果