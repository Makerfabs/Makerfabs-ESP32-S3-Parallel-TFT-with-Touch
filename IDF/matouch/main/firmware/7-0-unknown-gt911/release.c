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

#include "../../boards/7-0-unknown-gt911/board.h"

static const char *TAG = "RELEASE";

static void release_task(void *arg)
{
    extern void lvgl_demos_test();
    lvgl_demos_test();
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
