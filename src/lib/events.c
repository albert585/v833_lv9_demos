#include "./events.h"
#include "./file_manager.h"
#include "./container.h"
#include "./settings.h"
void event_open_manager(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
      file_manager();
    }
}

void event_close_manager(lv_event_t * e){
  lv_obj_t * obj = lv_event_get_target(e);
  lv_obj_t * manager = lv_event_get_user_data(e);
  lv_obj_del(manager);
  lv_obj_remove_flag(parent,LV_OBJ_FLAG_HIDDEN);
  lv_obj_set_style_opa(parent, LV_OPA_COVER, 255);

}
void event_open_settings(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_CLICKED) {
      settings();
    }

}
