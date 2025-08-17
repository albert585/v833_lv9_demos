#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <time.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <string.h>
#include "lvgl/lvgl.h"
#include "lib/file_manager.h"
#include "lib/container.h"
#include "lib/button.h"
#include "lib/settings.h"
lv_display_t * disp = NULL;


#define PATH_MAX_LENGTH 256
char homepath[PATH_MAX_LENGTH] = {0};
int disphd  = 0;    
int fbd = 0;
int homed  = 0;
int powerd  = 0;
uint32_t sleepTs     = -1;
uint32_t homeClickTs = -1;
uint32_t backgroundTs = -1;
extern uint32_t custom_tick_get(void);
bool deepsleep  = false;
#include "./main.h"
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
            printf("[key]home_down\n");
        }
    }
}
void readKeyPower(void) {  
    char buffer[16] = {0};
    while (read(powerd, buffer, 0x10u) > 0) {
                if(buffer[10] != 0x74) return;

                if(buffer[12] == 0x00) {
                        //printf("[key]power_up\n");
                        if(sleepTs == -1) {
                                sysSleep();   
                        }
                        else {
                                sysWake();
                        }
                }
                else {
                        //printf("[key]power_down\n");
                }
        }
}
void sysSleep(void){
        deepsleep = false;
        sleepTs = custom_tick_get();
        touchClose();   
        lcdClose();
}
void switchBackground(void){
    if(backgroundTs != -1) return;
    backgroundTs = custom_tick_get();
    sleepTs    = -1;
}
void switchForeground(void)
{
    if(backgroundTs == -1) return;

    chdir(homepath);
    system("/mnt/app/back.sh");
    sleep(114514);
}
void switchRobot(){
    switchBackground();

    chdir("/mnt/app");
    close(disphd);
    close(fbd);
    close(powerd);
    system("killall wpa_supplicant");
    system("./robot_run_1&");
}

void lcdRefresh(void) {
    int buffer[8] = {0};
        ioctl(fbd, 0x4606u, buffer);
}
void lcdOpen(void) {
    int buffer[8] = {0};
    buffer[1] = 1;
    ioctl(disphd, 0xFu, buffer);
    printf("[lcd]opened\n");
}

void lcdClose(void) {
    char buffer[24] = {0};
    ioctl(disphd, 0xFu, buffer);
    printf("[lcd]closed\n");
}
void touchOpen(void) {
        int tpd = open("/proc/sprocomm_tpInfo", 526338);
    write(tpd, "1", 1u);
    close(tpd);
    printf("[tp]opened\n");
}

void touchClose(void) {
    int tpd = open("/proc/sprocomm_tpInfo", 526338);
    write(tpd, "0", 1u);
    close(tpd);
    printf("[tp]closed\n");
}
void sysDeepSleep(void){
        deepsleep = true;
    sleepTs   = -1;
    // 这是真睡死过去，相当省电，按电源键还能回来
    system("echo \"0\" >/sys/class/rtc/rtc0/wakealarm");
    system("echo \"mem\" > /sys/power/state");
}
void sysWake(void){
        deepsleep = false;
        sleepTs = -1;
        touchOpen();
    lcdOpen();
}

int main()
{
  system("killall robotd");
  system("killall robot_run_1");
  powerd = open("/dev/input/event1", O_RDWR);
  fcntl(powerd, 4,2048);
  homed = open("/dev/input/event2", O_RDWR);
  fcntl(homed, 4,2048);
  disphd = open("/dev/disp", O_RDWR);
  fbd = open("/dev/fb0" , O_RDWR);
  getcwd(homepath, PATH_MAX_LENGTH);
  daemon(1,0);
  lcdRefresh();
  lv_init();
  lv_linux_disp_init();
  printf("display OK!\n");
  lv_display_set_rotation(disp,LV_DISPLAY_ROTATION_90);
  lv_linux_touch_init();
  printf("init OK\n");
  /*Initialize LVGL*/
  


  lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0x808080), 0);

  create_container();

  button();
  //lv_demo_widgets(); 

  while(1) {
        readKeyHome();
        if(backgroundTs == -1){
            readKeyPower();
                if(sleepTs == -1) {
                    lv_timer_handler();
                    usleep(5000);
            }
            else {
                    if (!deepsleep && custom_tick_get() - sleepTs >= 60000) {
                            sysDeepSleep();
                    }
                usleep(25000);
            }
        }
        else {
            if(custom_tick_get() - backgroundTs >= 300000) {
                switchForeground();
            }
            usleep(25000);
        }
    } 

    while(1){
        lv_timer_handler();
        usleep(5000);
    }
  close(disphd);
  close(powerd);
  close(homed);
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
