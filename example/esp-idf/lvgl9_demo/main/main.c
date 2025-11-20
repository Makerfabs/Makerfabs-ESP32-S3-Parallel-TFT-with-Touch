#include <stdio.h>
#include <nvs.h>
#include <nvs_flash.h>
#include <esp_log.h>
#include <esp_event.h>
#include <esp_sntp.h>
#include <time.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "protocol_examples_common.h"

static const char *TAG = "main";

static void get_sntp_time(void *arg)
{
    // Initialize ntp
    setenv("TZ", "CST-8", 1);
    tzset();
    ESP_LOGI(TAG, "Timezone set to CST-8");

    ESP_LOGI(TAG, "Initializing SNTP");
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "ntp.aliyun.com");
    esp_sntp_init();
    ESP_LOGI(TAG, "SNTP initialized");

    time_t now = 0;
    struct tm timeinfo = { 0 };
    int retry = 0;
    const int retry_count = 20; // 每次 2 秒，总计 40 秒
    while (retry < retry_count) {
        time(&now);
        localtime_r(&now, &timeinfo);
        if (timeinfo.tm_year > (2016 - 1900)) break; // 时间已被设置（>2016）
        ESP_LOGI(TAG, "等待 SNTP 同步... (%d/%d)", retry + 1, retry_count);
        vTaskDelay(pdMS_TO_TICKS(1000));
        retry++;
    }

    char strftime_buf[64];

    time(&now);
    localtime_r(&now, &timeinfo);

    // 格式化时间
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    ESP_LOGI(TAG, "The current date/time in Shanghai is: %s", strftime_buf);

    vTaskDelete(NULL);
}
void app_main(void)
{
    // Initialize the default event loop
    // ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Initialize NVS flash for WiFi configuration
    // esp_err_t ret = nvs_flash_init();
    // if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    //     ESP_LOGW(TAG, "Erasing NVS flash to fix corruption");
    //     ESP_ERROR_CHECK(nvs_flash_erase());
    //     ret = nvs_flash_init();
    // }
    // ESP_ERROR_CHECK(ret);

    extern void board_init(void);
    board_init();

#ifdef CONFIG_DEVELOPMENT_MODE_DEBUG
    extern void debug_demo(void);
    debug_demo();
#endif

#ifdef CONFIG_DEVELOPMENT_MODE_RELEASE
    extern void release_demo(void);
    release_demo();
#endif

    // ESP_ERROR_CHECK(esp_netif_init());
    // ESP_ERROR_CHECK(esp_event_loop_create_default());
    // ESP_ERROR_CHECK(example_connect());

    // xTaskCreate(get_sntp_time, "get_sntp_time", 4096, NULL, 6, NULL);

}