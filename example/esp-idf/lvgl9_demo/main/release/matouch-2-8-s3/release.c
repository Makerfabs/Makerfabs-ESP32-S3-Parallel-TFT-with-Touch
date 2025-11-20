#include "lvgl.h"
#include "esp_lvgl_port.h"

#include <esp_log.h>
#include <freertos/task.h>
#include <freertos/FreeRTOS.h>

#include "boards/matouch-2-8-s3/board.h"

static void test_task(void *arg)
{
    int32_t data[1024];
    while(true) {
        int samples = board_handle->audio_handle->read(data, 1024);
        if(samples) board_handle->audio_handle->write(data, samples);
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void market_demo()
{
    xTaskCreate(test_task, "test_task", 1024 * 10, NULL, 5, NULL);
}