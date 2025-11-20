#pragma once

#include "bsp_config.h"


typedef struct board_handle_t* board_t;

struct board_handle_t{
    audio_t         audio_handle;
    bsp_qmi8658_t   qmi8658_handle;
    bsp_pcf85063a_t pcf85063a_handle;
};

extern board_t board;


void board_init(void);