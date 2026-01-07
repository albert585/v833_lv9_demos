#include "./button.h"
#include "./container.h"
#include "./file_manager.h"
#include "./events.h"


void button(void)
{
    lv_obj_t * label1;
    lv_obj_t * label2;

    lv_obj_t * btn1 = lv_btn_create(parent);
    lv_obj_t * btn2 = lv_btn_create(parent);

    lv_obj_add_event_cb(btn1, event_open_manager, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(btn2,event_open_settings, LV_EVENT_ALL, NULL);

    lv_obj_clear_flag(btn1, LV_OBJ_FLAG_PRESS_LOCK);
    lv_obj_clear_flag(btn1, LV_OBJ_FLAG_PRESS_LOCK);

    label1 = lv_label_create(btn1);
    label2 = lv_label_create(btn2);

    lv_label_set_text(label1, "File Manager");
    lv_label_set_text(label2, "TESTING");

    lv_obj_center(label1);
    lv_obj_center(label2);

    // 水平排列所有按钮，居中对齐
    lv_obj_align(btn1, LV_ALIGN_CENTER, -180, 0);
    lv_obj_align(btn2, LV_ALIGN_CENTER, -60, 0);

    lv_obj_t * btn_robot = lv_btn_create(parent);
    lv_obj_align(btn_robot, LV_ALIGN_CENTER, 60, 0);
    lv_obj_t * btn_label_robot = lv_label_create(btn_robot);
    lv_label_set_text(btn_label_robot, "robot");
    lv_obj_center(btn_label_robot);
    lv_obj_add_event_cb(btn_robot, btn_robot_click, LV_EVENT_CLICKED, NULL);

    // 添加 Visual Novel 按钮
    lv_obj_t * btn_vn = lv_btn_create(parent);
    lv_obj_align(btn_vn, LV_ALIGN_CENTER, 180, 0);
    lv_obj_t * btn_label_vn = lv_label_create(btn_vn);
    lv_label_set_text(btn_label_vn, "Visual Novel");
    lv_obj_center(btn_label_vn);
    lv_obj_add_event_cb(btn_vn, event_open_visual_novel, LV_EVENT_CLICKED, NULL);

    // 添加音频测试按钮
    lv_obj_t * btn_audio_test = lv_btn_create(parent);
    lv_obj_align(btn_audio_test, LV_ALIGN_CENTER, 0, 100);
    lv_obj_t * btn_label_audio_test = lv_label_create(btn_audio_test);
    lv_label_set_text(btn_label_audio_test, "Audio Test");
    lv_obj_center(btn_label_audio_test);
    lv_obj_add_event_cb(btn_audio_test, event_audio_test, LV_EVENT_CLICKED, NULL);
}
