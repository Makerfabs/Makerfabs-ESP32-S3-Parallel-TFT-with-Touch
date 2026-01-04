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

extern void template_image_switch_lvgl(const char *path);
extern void template_lvgl_demos_test(void);


static void release_task(void *arg)
{
    template_image_switch_lvgl("/sdcard/images");
    // template_lvgl_demos_test();
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