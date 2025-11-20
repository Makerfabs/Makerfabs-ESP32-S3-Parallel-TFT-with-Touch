#pragma once

#include "bsp_config.h"

typedef struct board_handle_t* board_t;

struct board_handle_t{
    audio_t audio_handle;
};

extern board_t board_handle;


void board_init(void);