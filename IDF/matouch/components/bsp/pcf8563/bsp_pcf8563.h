#pragma once

#include <esp_err.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <driver/i2c_master.h>
#include <time.h>

#define PCF8563_LOCK_TIMEOUT_MS 2000

typedef struct bsp_pcf8563_t *bsp_pcf8563_handle_t;

struct bsp_pcf8563_t {
    esp_err_t (*get_time)(bsp_pcf8563_handle_t self, struct tm *timeinfo);
    esp_err_t (*set_time)(bsp_pcf8563_handle_t self, const struct tm *timeinfo);
    esp_err_t (*destroy)(bsp_pcf8563_handle_t self);
    
    void *_priv;
}; 

/**
 * @brief 初始化 PCF8563 RTC
 * 
 * @param handle 输出句柄指针
 * @param bus_handle I2C 总线句柄
 * @return esp_err_t 
 */
esp_err_t bsp_pcf8563_init(bsp_pcf8563_handle_t *handle, i2c_master_bus_handle_t bus_handle);

/**
 * @brief 销毁 PCF8563 RTC 实例
 * 
 * @param handle RTC 句柄
 * @return esp_err_t 
 */
esp_err_t bsp_pcf8563_destroy(bsp_pcf8563_handle_t handle);