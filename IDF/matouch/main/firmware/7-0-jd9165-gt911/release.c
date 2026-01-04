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

#include "board.h"

static const char *TAG = "RELEASE";

static void release_task(void *arg)
{
    // extern void template_image_switch_lvgl(const char *path);
    // template_image_switch_lvgl("/sdcard/images");

    extern void template_lvgl_demos_test();
    template_lvgl_demos_test();

    extern void template_dht11_test();
    template_dht11_test();
    
    vTaskDelete(NULL);
}


void release_demo(void)
{
    ESP_LOGI(TAG, "release demo");
    
    xTaskCreate(release_task,
                "release_task",
                1024 * 10,
                NULL,
                5,
                NULL);

    // xTaskCreatePinnedToCore(release_task, 
    //                         "release_task",
    //                         1024 * 10,
    //                         NULL,
    //                         3,
    //                         NULL,
    //                         0);
}
