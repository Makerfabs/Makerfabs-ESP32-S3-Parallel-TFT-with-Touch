#pragma once

#include "bsp_config.h"


typedef struct board_handle_t* board_t;

struct board_handle_t{
};

extern board_t board;


void board_init(void);