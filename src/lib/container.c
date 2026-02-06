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

    /* Set flex layout for automatic horizontal alignment */
    lv_obj_set_layout(parent, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(parent, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(parent, 40, 0);
}


