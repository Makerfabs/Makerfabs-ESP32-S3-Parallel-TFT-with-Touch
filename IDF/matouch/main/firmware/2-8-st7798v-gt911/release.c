#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "lvgl.h"
#include "esp_lvgl_port.h"

#include "board.h"

static const char *TAG = "RELEASE";

static void release_task(void *arg)
{
    // extern void image_switch_lvgl(const char *path);
    // image_switch_lvgl("/sdcard/images");
    extern void template_lvgl_demos_test();
    template_lvgl_demos_test();
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