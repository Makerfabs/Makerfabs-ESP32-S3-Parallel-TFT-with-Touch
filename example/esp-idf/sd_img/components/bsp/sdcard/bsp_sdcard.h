#pragma once

#include <esp_err.h>
#include <driver/gpio.h>


esp_err_t bsp_sdcard_mount(const char *mount_point, gpio_num_t sd_cs);