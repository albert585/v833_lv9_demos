#include "./button.h"
#include "./container.h"
#include "./file_manager.h"
#include "./events.h"


void button(void)
{
    lv_obj_t * label1;
    lv_obj_t * label2;

    lv_obj_t * btn1 = lv_button_create(parent);
    lv_obj_t * btn2 = lv_button_create(parent);

    lv_obj_add_event_cb(btn1, event_open_manager, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(btn2,event_open_settings, LV_EVENT_ALL, NULL);
    lv_obj_align(btn1, LV_ALIGN_LEFT_MID, 40, 0);
    lv_obj_align(btn2, LV_ALIGN_LEFT_MID, 180, 0);

    lv_obj_remove_flag(btn1, LV_OBJ_FLAG_PRESS_LOCK);
    lv_obj_remove_flag(btn1, LV_OBJ_FLAG_PRESS_LOCK);

    label1 = lv_label_create(btn1);
    label2 = lv_label_create(btn2);

    lv_label_set_text(label1, "File Manager");
    lv_label_set_text(label2, "TESTING");

    lv_obj_center(label1);
    lv_obj_center(label2);
    
    
}

