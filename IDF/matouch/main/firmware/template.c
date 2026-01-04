#include <stdio.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <esp_check.h>
#include <driver/jpeg_decode.h>
#include <math.h>

#include "file_iterator.h"
#include "lvgl.h"
#include "esp_lvgl_port.h"
#include "lv_demos.h"

#include "board.h"

#include "driver/i2c_master.h"


static char *TAG = "template";



// @brief image switch 
static lv_obj_t *img_obj = NULL;
static file_iterator_instance_t *file_iterator = NULL;
static int img_index = 0;
static int img_count = 0;


/* =============================================================================================================
                                                image switch
    ============================================================================================================ */

static char* image_switch_get_path(int index)
{
    const char *image_name = file_iterator_get_name_from_index(file_iterator,index);

    static char image_path[256];
    // file_iterator_get_full_path_from_index(file_iterator,index,image_path,256);
    snprintf(image_path, sizeof(image_path), "A:/images/%s", image_name);

    return image_path;
}

static void image_switch_rb()
{
    lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
    if(dir != LV_DIR_LEFT && dir != LV_DIR_RIGHT) return; // only handle left/right

    int new_index = img_index + (dir == LV_DIR_LEFT ? 1 : -1);
    if(new_index < 0) new_index = img_count - 1;
    if(new_index >= img_count) new_index = 0;

    img_index = new_index;

    ESP_LOGI(TAG, "%s", image_switch_get_path(img_index));

    // Load image from SD card
    lvgl_port_lock(0);
    lv_img_set_src(img_obj, image_switch_get_path(img_index));
    lvgl_port_unlock();
}

void template_image_switch_lvgl(const char *image_dir_path)
{
    file_iterator = file_iterator_new(image_dir_path);
    assert(file_iterator);

    img_count = file_iterator_get_count(file_iterator);
    if (img_count) 
        ESP_LOGI(TAG, "image count: %d", img_count);
    else {
        ESP_LOGW(TAG, "no image found");
        return ;
    }

    lv_obj_add_event(lv_scr_act(), image_switch_rb, LV_EVENT_GESTURE, NULL);

    for(int i = 0; i < img_count; i++) {
        ESP_LOGI(TAG, "%s", image_switch_get_path(i));
    }

    lvgl_port_lock(0);
    img_obj = lv_img_create(lv_scr_act());
    lv_obj_center(img_obj);
    lv_image_set_src(img_obj, image_switch_get_path(img_index));
    lvgl_port_unlock();
}


/* =============================================================================================================
                                                LVGL Demos
    ============================================================================================================ */


void template_lvgl_demos_test()
{
    lvgl_port_lock(0);

    lv_demo_widgets();

    lvgl_port_unlock();
}

