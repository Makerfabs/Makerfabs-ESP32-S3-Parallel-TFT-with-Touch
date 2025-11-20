#include "lvgl.h"
#include "esp_lvgl_port.h"

#include <esp_log.h>

LV_IMG_DECLARE(img_1);
LV_IMG_DECLARE(img_2);
LV_IMG_DECLARE(img_3);
LV_IMG_DECLARE(img_4);
LV_IMG_DECLARE(img_5);
static lv_obj_t *img_obj;

static const lv_img_dsc_t *images[] = {&img_1, &img_2, &img_3, &img_4, &img_5};
static int img_index = 0;

// Hide the old image after animation and delete the timer
static void gesture_event_cb(lv_event_t *e)
{
    lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
    if(dir != LV_DIR_LEFT && dir != LV_DIR_RIGHT) return; // only handle left/right

    const int cnt = sizeof(images) / sizeof(images[0]);
    int new_index = img_index + (dir == LV_DIR_LEFT ? 1 : -1);
    if(new_index < 0) new_index = cnt - 1;
    if(new_index >= cnt) new_index = 0;

    img_index = new_index;
    lv_img_set_src(img_obj, images[img_index]);
}
static void lv_demo_purecolor_png_switch()
{
    struct {
        lv_color_t color;
        const char *name;
    } test_colors[] = {
        {lv_color_make(255, 0, 0),     "红色 (Red)"},
        {lv_color_make(0, 255, 0),     "绿色 (Green)"},
        {lv_color_make(0, 0, 255),     "蓝色 (Blue)"},
        {lv_color_make(255, 255, 255), "白色 (White)"},
        {lv_color_make(0, 0, 0),       "黑色 (Black)"},
    };

    // Cycle through the test colors, changing the background color every second
    for(int i = 0; i < sizeof(test_colors) / sizeof(test_colors[0]); i++) {
        lvgl_port_lock(0);
        lv_obj_set_style_bg_color(lv_scr_act(), test_colors[i].color, LV_PART_MAIN);
        lvgl_port_unlock();
        vTaskDelay(pdMS_TO_TICKS(1000));
    }


    // create a single image object and switch src immediately on gesture
    img_obj = lv_img_create(lv_scr_act());
    lv_img_set_src(img_obj, images[0]);

    // register gesture handler on the screen
    lv_obj_add_event(lv_scr_act(), gesture_event_cb, LV_EVENT_GESTURE, NULL);
}


void market_demo()
{
    lv_demo_purecolor_png_switch();
}