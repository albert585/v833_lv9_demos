
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "lvgl/lvgl.h"
#include "lib/buttons.h"
#include "lib/events.h"
#include "lib/file_manager.h"
#include "lib/container.h"

lv_display_t * disp = NULL;



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
