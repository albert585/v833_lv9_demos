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
#include "lvgl/lvgl.h"
#include "lib/file_manager.h"
#include "lib/container.h"
#include "lib/button.h"
#include "lib/settings.h"
#error "the programme is unavalible"
#if 0
lv_display_t * disp = NULL;
extern int homed  = 0;
extern int powerd  = 0;
extern uint32_t sleepTs     = -1;
extern uint32_t homeClickTs = -1;
extern uint32_t backgroundTs = -1;
extern uint32_t custom_tick_get(void);

getcwd(homepath, PATH_MAX_LENGTH);
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

void switchBackground(void){
    if(backgroundTs != -1) return;
    backgroundTs = custom_tick_get();
    sleepTs    = -1;
}

void readKeyHome(void) {
        char buffer[16] = {0};
        while (read(homed, buffer, 0x10u) > 0) {
                if(buffer[10] != 0x73) return;

                if(buffer[12] == 0x00) {
                        printf("[key]home_up\n");
            uint32_t ts = custom_tick_get();
            if(homeClickTs != -1 && ts - homeClickTs <= 300){
                switchForeground();
                homeClickTs = -1;
            } else {
                homeClickTs = ts;
            }
        } else {
            //printf("[key]home_down\n");
        }
    }
}

void switchRobot(){
    switchBackground();

    chdir("/mnt/app");
    system("./robot_run &");
}




int main()
{
  daemon(1,0);
  lv_init();
  lv_linux_disp_init();
  lv_display_set_rotation(disp,LV_DISPLAY_ROTATION_90);
  lv_linux_touch_init();
  printf("init OK");
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
uint32_t custom_tick_get(void)
{
    static uint64_t start_ms = 0;
    if(start_ms == 0) {
        struct timeval tv_start;
        gettimeofday(&tv_start, NULL);
        start_ms = (tv_start.tv_sec * 1000000 + tv_start.tv_usec) / 1000;
    }

    struct timeval tv_now;
    gettimeofday(&tv_now, NULL);
    uint64_t now_ms;
    now_ms = (tv_now.tv_sec * 1000000 + tv_now.tv_usec) / 1000;

    uint32_t time_ms = now_ms - start_ms;
    return time_ms;
}
#endif