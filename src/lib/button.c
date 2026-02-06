#include "button.h"
#include "file_manager.h"
#include "events.h"
#include "container.h"
#include "src/lib/lv_lib_100ask/src/lv_100ask_2048/lv_100ask_2048.h"
void button(void)
{
    lv_obj_t * label1;
    lv_obj_t * label2;
    lv_obj_t * btn_label_vn;
    lv_obj_t * btn_label_robot;
    lv_obj_t * btn_label_audio;

    lv_obj_t * btn1 = lv_btn_create(parent);
    lv_obj_t * btn2 = lv_btn_create(parent);
    lv_obj_t * btn_vn = lv_btn_create(parent);
    lv_obj_t * btn_robot = lv_btn_create(parent);
    lv_obj_t * btn_2048 = lv_btn_create(parent);

    lv_obj_add_event_cb(btn1, event_open_manager, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(btn2,event_open_settings, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(btn_robot, btn_robot_click, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(btn_vn, event_open_visual_novel, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(btn_2048, event_open_2048, LV_EVENT_CLICKED, NULL);

    lv_obj_clear_flag(btn1, LV_OBJ_FLAG_PRESS_LOCK);

    label1 = lv_label_create(btn1);
    label2 = lv_label_create(btn2);

    btn_label_robot = lv_label_create(btn_robot);
    btn_label_vn = lv_label_create(btn_vn);
    btn_label_2048 = lv_label_create(btn_2048);

    lv_label_set_text(label1, "File Manager");
    lv_label_set_text(label2, "TESTING");
    lv_label_set_text(btn_label_robot, "robot");
    lv_label_set_text(btn_label_vn, "Visual Novel");
    lv_label_set_text(btn_label_2048, "2048");

}
