#include "lvgl.h"
#include "esp_lvgl_port.h"

#include <esp_log.h>
#include <freertos/task.h>
#include <freertos/FreeRTOS.h>

#include "boards/matouch-2-8-s3/board.h"

static const char *TAG = "debug";



static void debug_task(void *arg)
{
    bsp_spiffs_mount();
    
   // 1. 测试文件
    lv_fs_file_t file;
    lv_fs_res_t res = lv_fs_open(&file, "A:/111.png", LV_FS_MODE_RD);
    if (res != LV_FS_RES_OK) {
        ESP_LOGE(TAG, "Cannot open 111.png, error: %d", res);
        vTaskDelete(NULL);
        return;
    }
    
    uint32_t size;
    lv_fs_seek(&file, 0, LV_FS_SEEK_END);
    lv_fs_tell(&file, &size);
    ESP_LOGI(TAG, "✓ File opened: %lu bytes", size);
    lv_fs_close(&file);

    lvgl_port_lock(0);
    
    lv_obj_t * img;

    img = lv_image_create(lv_screen_active());
    /* Assuming a File system is attached to letter 'A'
     * E.g. set LV_USE_FS_STDIO 'A' in lv_conf.h */
    lv_image_set_src(img, "A:/111.png");
    lv_obj_align(img, LV_ALIGN_CENTER, 0, 0);

    lvgl_port_unlock();

    

    vTaskDelete(NULL);
}

void debug_demo()
{
    xTaskCreate(debug_task,
                "test_task",
                1024 * 10,
                NULL,
                5,
                NULL);
    
}