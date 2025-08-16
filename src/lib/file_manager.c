#include "file_manager.h"
#include "container.h"
#include "events.h"

void file_manager(void) {
    manager = lv_menu_create(parent);
    lv_obj_set_size(manager,960,240);
    lv_obj_center(manager);
    lv_menu_set_mode_root_back_button(manager,LV_MENU_ROOT_BACK_BUTTON_ENABLED);
    lv_obj_add_event_cb(manager,event_close_manager,LV_EVENT_CLICKED,manager);
    lv_obj_t *fmg =lv_file_explorer_create(manager);
    lv_file_explorer_set_sort(fmg, LV_EXPLORER_SORT_KIND);
    lv_obj_set_size(fmg,960,240);
    lv_file_explorer_open_dir(fmg,"A:/");
  }