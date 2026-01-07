#include "audio.h"
#include <string.h>
#include "./file_manager.h"
#include "./container.h"
#include "./events.h"
#include "lv_lib_100ask.h"

lv_obj_t *manager = NULL;
lv_obj_t *file_explorer = NULL;

void file_manager(void) {
    manager = lv_menu_create(parent);
    lv_obj_set_size(manager, 960, 240);
    lv_obj_center(manager);
    
    // 添加关闭按钮事件
    lv_obj_add_event_cb(manager, event_close_manager, LV_EVENT_CLICKED, manager);
    
    // 使用 lv_100ask_file_explorer 创建文件浏览器
    file_explorer = lv_100ask_file_explorer_create(manager);
    
    // 设置文件浏览器大小
    lv_obj_set_size(file_explorer, 960, 220);
    
    // 设置排序方式（按类型排序）
    lv_100ask_file_explorer_set_sort(file_explorer, LV_100ASK_EXPLORER_SORT_KIND);
    
    // 打开默认目录（根目录）
    lv_100ask_file_explorer_open_dir(file_explorer, "/");
    
    // 添加文件选择事件
    lv_obj_add_event_cb(file_explorer, file_select_event, LV_EVENT_VALUE_CHANGED, NULL);
}