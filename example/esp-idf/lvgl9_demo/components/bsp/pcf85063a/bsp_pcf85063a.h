#pragma once

#pragma once

#include "pcf85063a.h"
#include <esp_err.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

#define PCF85063A_LOCK_TIMEOUT_MS 2000

typedef struct bsp_pcf85063a_handle_t* bsp_pcf85063a_t;

struct bsp_pcf85063a_handle_t {
    pcf85063a_dev_t dev;

    esp_err_t (*get_time_data)(pcf85063a_datetime_t *time_data);
    esp_err_t (*set_time_data)(pcf85063a_datetime_t *time_data);

    /* mutex */
    SemaphoreHandle_t lock;
};

/* Create/Destroy */
esp_err_t bsp_pcf85063a_init(bsp_pcf85063a_t *handle, i2c_master_bus_handle_t bus_handle);