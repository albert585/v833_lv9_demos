#include "./container.h"
#include "stdio.h"

lv_obj_t *parent = NULL;  // 实际定义

void create_container(void) { //创建显示区域


    parent = lv_obj_create(lv_screen_active());
    lv_obj_set_size(parent, 960, 240);
    lv_obj_set_style_bg_color(parent, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_bg_opa(parent, LV_OPA_100, 0);
    lv_obj_set_style_border_width(parent, 0, 0);

    lv_obj_center(parent);

    lv_obj_set_scrollbar_mode(parent, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_scroll_dir(parent, LV_DIR_NONE);
    printf("container created!");
}


