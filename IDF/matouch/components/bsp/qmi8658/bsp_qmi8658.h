#pragma once

#include "qmi8658.h"
#include <esp_err.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

#define QMI8658_LOCK_TIMEOUT_MS 2000

typedef struct bsp_qmi8658_handle_t* bsp_qmi8658_t;

struct bsp_qmi8658_handle_t {
    qmi8658_dev_t dev;

    esp_err_t (*enable_sensors)(uint8_t flags);
    esp_err_t (*read_accel)(float *x, float *y, float *z);
    esp_err_t (*read_gyro)(float *x, float *y, float *z);
    esp_err_t (*read_sensor_data)(qmi8658_data_t *data);
    esp_err_t (*set_arg)(void);
    // qmi8658_orientation_e (*detect_orientation)(float ax, float ay, float az);

    void (*delete)(void);

    /* mutex */
    SemaphoreHandle_t lock;
};

/* Create/Destroy */
esp_err_t bsp_qmi8658_init(bsp_qmi8658_t *handle, i2c_master_bus_handle_t bus_handle);