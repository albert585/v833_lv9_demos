#include "./settings.h"
#include "./container.h"
#include "./events.h"
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <string.h>

lv_obj_t *setting = NULL;
lv_obj_t *subpage;
lv_obj_t *backlight_cont;
lv_obj_t *slider_label;
lv_obj_t *slider;
int dispd = 0 ;
unsigned int arg[3];
unsigned int bl;
extern void backlight_slider(void);
void settings(void){
    setting = lv_menu_create(parent);
    subpage = lv_menu_page_create(setting,NULL);
    lv_obj_t *line;
    lv_obj_t *label;


    lv_obj_set_size(setting,960,240);
    lv_obj_center(setting);
    lv_menu_set_mode_root_back_button(setting,LV_MENU_ROOT_BACK_BUTTON_ENABLED);
    lv_obj_add_event_cb(setting,event_close_manager,LV_EVENT_CLICKED,setting);
    lv_obj_t * mainpage = lv_menu_page_create(setting, NULL);

    backlight_cont= lv_menu_cont_create(subpage);
    backlight_slider();

    line = lv_menu_cont_create(mainpage);
    label = lv_label_create(line);
    lv_label_set_text(label, "BackLight 背光调节");
    lv_menu_set_load_page_event(setting, line, subpage);
    lv_menu_set_page(setting,mainpage);
}

    


void backlight_slider(void)
{
    dispd = open("/dev/disp", O_RDWR);
    arg[0] = 0; 
    bl = ioctl(dispd, 0x103u, arg);
    slider = lv_slider_create(backlight_cont);
    lv_obj_center(slider);
    lv_obj_add_event_cb(slider, slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_slider_set_value(slider,bl,LV_ANIM_OFF);
    lv_slider_set_range(slider, 255, 0); //反了
    lv_obj_set_style_anim_duration(slider, 2000, 0);
    /*Create a label below the slider*/
    lv_obj_center(slider);
}


void slider_event_cb(lv_event_t * e)
{
    lv_obj_t * slider = lv_event_get_target(e);
    bl = (unsigned int)lv_slider_get_value(slider);
    arg[0] = 0;  // 显示通道
    arg[1] = bl; // 亮度值
    ioctl(dispd, 0x102u, arg);


}
