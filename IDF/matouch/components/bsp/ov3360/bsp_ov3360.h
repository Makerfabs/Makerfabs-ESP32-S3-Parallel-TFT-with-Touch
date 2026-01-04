#pragma once

#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include <"reertos/semphr.h"

typedef struct bsp_ov3660_t *bsp_ov3600_handle_t;


typedef struct {
    int pin_pwdn;                   /*!< GPIO pin for camera power down line */
    int pin_reset;                  /*!< GPIO pin for camera reset line */
    int pin_xclk;                   /*!< GPIO pin for camera XCLK line */

    int pin_sccb_sda;               /*!< GPIO pin for camera SDA line */
    int pin_sccb_scl;               /*!< GPIO pin for camera SCL line */

    int pin_d7;                     /*!< GPIO pin for camera D7 line */
    int pin_d6;                     /*!< GPIO pin for camera D6 line */
    int pin_d5;                     /*!< GPIO pin for camera D5 line */
    int pin_d4;                     /*!< GPIO pin for camera D4 line */
    int pin_d3;                     /*!< GPIO pin for camera D3 line */
    int pin_d2;                     /*!< GPIO pin for camera D2 line */
    int pin_d1;                     /*!< GPIO pin for camera D1 line */
    int pin_d0;                     /*!< GPIO pin for camera D0 line */
    int pin_vsync;                  /*!< GPIO pin for camera VSYNC line */
    int pin_href;                   /*!< GPIO pin for camera HREF line */
    int pin_pclk;                   /*!< GPIO pin for camera PCLK line */
} bsp_ov3660_config_t;




struct bsp_ov3600_t {
    
    bsp_ov3660_config_t config;
    
     
    /* mutex */
    // SemaphoreHandle_t lock;
};

/* Create/Destroy */
esp_err_t bsp_ov3600_init(bsp_ov3600_handle_t *handle, i2c_master_bus_handle_t bus_handle);