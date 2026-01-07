#include <stdio.h>
#include <stdlib.h>
#include "data_parser.h"

int main() {
    // 解析JSON文件
    story_config_t *story = parse_story_json("data/story.json");
    if (story == NULL) {
        fprintf(stderr, "解析JSON文件失败\n");
        return 1;
    }
    
    // 打印故事基本信息
    printf("故事标题: %s\n", story->title);
    printf("作者: %s\n", story->author);
    printf("页面数量: %d\n\n", story->page_count);
    
    // 打印每个页面的信息
    for (int i = 0; i < story->page_count; i++) {
        printf("=== 页面 %d: %s ===\n", i + 1, story->pages[i].id);
        printf("背景图片: %s\n", story->pages[i].background ? story->pages[i].background : "无");
        printf("文本内容: %s\n", story->pages[i].text);
        printf("下一页: %s\n", story->pages[i].next_page ? story->pages[i].next_page : "无");
        printf("角色数量: %d\n", story->pages[i].character_count);
        
        // 打印角色信息
        for (int j = 0; j < story->pages[i].character_count; j++) {
            printf("  角色 %d: %s\n", j + 1, story->pages[i].characters[j].id);
            printf("    图片: %s\n", story->pages[i].characters[j].image ? story->pages[i].characters[j].image : "无");
            printf("    位置: (%d, %d)\n", story->pages[i].characters[j].x, story->pages[i].characters[j].y);
            printf("    缩放: %.2f\n", story->pages[i].characters[j].scale);
            printf("    可见: %s\n", story->pages[i].characters[j].visible ? "是" : "否");
        }
        
        // 打印文本框信息
        printf("文本框配置:\n");
        printf("  可见: %s\n", story->pages[i].textbox.visible ? "是" : "否");
        printf("  位置: (%d, %d)\n", story->pages[i].textbox.x, story->pages[i].textbox.y);
        printf("  尺寸: %dx%d\n", story->pages[i].textbox.width, story->pages[i].textbox.height);
        printf("  背景色: %s\n", story->pages[i].textbox.bg_color);
        printf("  文字色: %s\n", story->pages[i].textbox.text_color);
        printf("  字体大小: %d\n\n", story->pages[i].textbox.font_size);
    }
    
    // 释放资源
    free_story_config(story);
    
    printf("JSON解析测试成功！\n");
    return 0;
}