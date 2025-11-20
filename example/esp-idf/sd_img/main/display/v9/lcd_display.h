#pragma once

#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_ops.h>

#include "lvgl.h"


lv_display_t *spi_lcd_display(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_handle_t panel,
                           int width, int height, int offset_x, int offset_y,
                           bool mirror_x, bool mirror_y, bool swap_xy);

lv_display_t *rgb_lcd_display(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_handle_t panel,
                           int width, int height, int offset_x, int offset_y,
                           bool mirror_x, bool mirror_y, bool swap_xy);
                           