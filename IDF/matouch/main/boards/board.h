#pragma once

#include "bsp_config.h"


typedef struct board_t* board_handle_t;

struct board_t{

#if CONFIG_PCF85063A_ENABLE
    bsp_pcf85063a_handle_t pcf85063a;
#endif

#if CONFIG_PCF8563_ENABLE
    bsp_pcf8563_handle_t pcf8563;
#endif

#if CONFIG_QMI8658_ENABLE
    qmi8658_handle_t qmi8658;
#endif

#if CONFIG_AUDIO_ENABLE
    audio_handle_t audio;
#endif


    int reverse;

};

extern board_handle_t board_handle;
extern i2c_master_bus_handle_t _i2c_bus;    // for mpu6050 test


void board_init(void);