#include "lvgl.h"
static void lv_linux_disp_init(void)
{
    const char *device = getenv_default("LV_LINUX_FBDEV_DEVICE", "/dev/fb0");
    lv_display_t * disp = lv_linux_fbdev_create();

    lv_linux_fbdev_set_file(disp, device);
}
static void lv_linux_touch_init(){
    lv_indev_t *touch = lv_evdev_create(LV_INDEV_TYPE_POINTER, "/dev/input/event0");
    lv_indev_set_display(touch, disp);
}

int main(void)
{
    lv_init();

    /*Linux display device init*/
    lv_linux_disp_init();
    lv_linux_touch_init();
    

    while(1) {
        lv_timer_handler();
        usleep(5000);
    }

    return 0;
}