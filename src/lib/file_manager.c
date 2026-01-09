#include "audio.h"
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include "./file_manager.h"
#include "./container.h"
#include "./events.h"

lv_obj_t *manager = NULL;
static lv_obj_t *file_list = NULL;
static char current_path[PATH_MAX] = "/mnt/app";



void file_manager(void)
{
    manager = lv_menu_create(parent);
    lv_obj_set_size(manager, 240, 720);
    lv_obj_center(manager);

    lv_obj_add_event_cb(manager, event_close_manager, LV_EVENT_CLICKED, manager);
}