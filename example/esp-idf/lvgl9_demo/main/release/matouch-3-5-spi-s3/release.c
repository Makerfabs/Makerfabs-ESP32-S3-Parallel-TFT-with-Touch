#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/ringbuf.h>
#include <freertos/event_groups.h>
#include <math.h>
#include <esp_err.h>
#include <esp_log.h>
#include <string.h>
#include <dirent.h>
#include "lvgl.h"
#include "esp_lvgl_port.h"
#include "lv_demos.h"

#include "../../boards/matouch-3-5-spi-s3/board.h"

static const char *TAG = "RELEASE";



#define MAX_IMAGES 10
static lv_obj_t *img_obj;
static char image_paths[MAX_IMAGES][512];
static int image_count = 0;
static int img_index = 0;

static void gesture_event_cb(lv_event_t *e)
{
    lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
    if(dir != LV_DIR_LEFT && dir != LV_DIR_RIGHT) return; // only handle left/right

    if (image_count <= 0) return;

    int new_index = img_index + (dir == LV_DIR_LEFT ? 1 : -1);
    if(new_index < 0) new_index = image_count - 1;
    if(new_index >= image_count) new_index = 0;

    img_index = new_index;
    
    // Load image from SD card
    lvgl_port_lock(0);
    lv_img_set_src(img_obj, image_paths[img_index]);
    lvgl_port_unlock();
}
static void scan_images_from_sdcard(void)
{
    DIR *dir;
    struct dirent *entry;
    const char *mount_point = "/sdcard";
    
    dir = opendir(mount_point);
    if (dir == NULL) {
        ESP_LOGE(TAG, "Failed to open directory %s", mount_point);
        return;
    }

    ESP_LOGI(TAG, "Scanning for PNG images in %s", mount_point);
    image_count = 0;
    
    while ((entry = readdir(dir)) != NULL && image_count < MAX_IMAGES) {
        // Check if file ends with .png (case insensitive)
        char *ext = strrchr(entry->d_name, '.');
        if (ext && (strcmp(ext, ".bin") == 0 || strcmp(ext, ".BIN") == 0)) {
            snprintf(image_paths[image_count], sizeof(image_paths[image_count]), 
                     "A:/%s", entry->d_name);
            image_paths[image_count][sizeof(image_paths[image_count]) - 1] = '\0';
            ESP_LOGI(TAG, "Found image: %s", image_paths[image_count]);
            image_count++;
        }
    }
    
    closedir(dir);
    ESP_LOGI(TAG, "Found %d PNG images", image_count);
}


static void lv_demo_purecolor_png_switch()
{
    static const char* manual_image_paths[] = {
        "A:/9.bin",
        "A:/6.bin", 
        "A:/7.bin",
        "A:/8.bin",
        "A:/10.bin"
    };
    image_count = sizeof(manual_image_paths) / sizeof(manual_image_paths[0]);

    for(int i = 0; i < image_count && i < MAX_IMAGES; i++) {
        strncpy(image_paths[i], manual_image_paths[i], sizeof(image_paths[i]) - 1);
        image_paths[i][sizeof(image_paths[i]) - 1] = '\0';
        ESP_LOGI(TAG, "Added image path: %s", image_paths[i]);
    }
    ESP_LOGI(TAG, "Total images added: %d", image_count);
    if (image_count > 0) {
        // create a single image object and switch src immediately on gesture
        lvgl_port_lock(0);
        img_obj = lv_img_create(lv_scr_act());
        lv_img_set_src(img_obj, image_paths[0]);
        lvgl_port_unlock();

        // register gesture handler on the screen
        lv_obj_add_event(lv_scr_act(), gesture_event_cb, LV_EVENT_GESTURE, NULL);
    } else {
        ESP_LOGW(TAG, "No PNG images found on SD card");
        // Display a message
        lvgl_port_lock(0);
        lv_obj_t* label = lv_label_create(lv_scr_act());
        lv_label_set_text(label, "No PNG images found on SD card");
        lv_obj_center(label);
        lvgl_port_unlock();
    }
}

static void lvgl_demos()
{
    lvgl_port_lock(0);
    lv_demo_widgets();
    lvgl_port_unlock();
}

static void release_task(void *arg)
{
    lvgl_demos();
    // lv_demo_purecolor_png_switch();
    vTaskDelete(NULL); 
}


void release_demo(void)
{
    xTaskCreate(release_task,
                "release_task",
                1024 * 10,
                NULL,
                5,
                NULL);
}