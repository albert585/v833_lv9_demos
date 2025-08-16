
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "lvgl/lvgl.h"
#include "lvgl/demos/lv_demos.h"

lv_display_t * disp = NULL;
static lv_obj_t *parent = NULL;
static lv_obj_t *manager = NULL;


extern void  file_manager(void);
extern void  create_container(void);

static void event_open_manager(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
      lv_obj_add_flag(parent, LV_OBJ_FLAG_HIDDEN);
      lv_obj_set_style_opa(parent, LV_OPA_COVER, 0);
      file_manager();
    }
}
static void event_close_manager(lv_event_t * e){
  lv_obj_t * obj = lv_event_get_target(e);
  lv_obj_t * manager = lv_event_get_user_data(e);
  lv_obj_del(manager);
  lv_obj_remove_flag(parent,LV_OBJ_FLAG_HIDDEN);
  lv_obj_set_style_opa(parent, LV_OPA_COVER, 255);

}

//设置屏幕与输入设备
const char *getenv_default(const char *name, const char *default_val)
{
    const char* value = getenv(name);
    return value ? value : default_val;
}

static void lv_linux_disp_init(void)
{
    const char *device = getenv_default("LV_LINUX_FBDEV_DEVICE", "/dev/fb0");
    disp= lv_linux_fbdev_create();
    lv_linux_fbdev_set_file(disp, device);
}

static void lv_linux_touch_init(){
    lv_indev_t *touch = lv_evdev_create(LV_INDEV_TYPE_POINTER, "/dev/input/event0");
    lv_indev_set_display(touch, disp);
    lv_evdev_set_calibration(touch,-40,940,310,25);
}


void create_container(void) { //创建显示区域


    parent = lv_obj_create(lv_screen_active());
    lv_obj_set_size(parent, 960, 240);

    lv_obj_set_style_bg_color(parent, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_bg_opa(parent, LV_OPA_100, 0);
    lv_obj_set_style_border_width(parent, 0, 0);

    lv_obj_center(parent);

    lv_obj_set_scrollbar_mode(parent, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_scroll_dir(parent, LV_DIR_NONE);
}

void file_manager(void) {
    manager = lv_menu_create(lv_screen_active());
    lv_obj_set_size(manager,960,240);
    lv_obj_center(manager);
    lv_menu_set_mode_root_back_button(manager,LV_MENU_ROOT_BACK_BUTTON_ENABLED);
    lv_obj_add_event_cb(manager,event_close_manager,LV_EVENT_CLICKED,manager);
    lv_obj_t *fmg =lv_file_explorer_create(manager);
    lv_file_explorer_set_sort(fmg, LV_EXPLORER_SORT_KIND);
    lv_obj_set_size(fmg,960,240);
    lv_file_explorer_open_dir(fmg,"A:/");}



void button(void)
{
    lv_obj_t * label;

    lv_obj_t * btn1 = lv_button_create(parent);
    lv_obj_add_event_cb(btn1, event_open_manager, LV_EVENT_ALL, NULL);
    lv_obj_align(btn1, LV_ALIGN_LEFT_MID, 40, 0);
    lv_obj_remove_flag(btn1, LV_OBJ_FLAG_PRESS_LOCK);

    label = lv_label_create(btn1);
    lv_label_set_text(label, "File Manager");
    lv_obj_center(label);
}



int main()
{
  lv_init();
  lv_linux_disp_init();
  lv_display_set_rotation(disp,LV_DISPLAY_ROTATION_90);
  lv_linux_touch_init();
  /*Initialize LVGL*/
  


  lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0x808080), 0);

  create_container();
  button();
  //lv_demo_widgets(); 

  while(1) {
    lv_timer_handler();
    usleep(5 * 1000);
  }

  return 0;
}
