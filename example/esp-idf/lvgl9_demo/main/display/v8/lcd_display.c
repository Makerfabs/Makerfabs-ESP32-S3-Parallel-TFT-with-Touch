#include <esp_log.h>


#include "lcd_display.h"
#include "esp_lvgl_port.h"

static const char *TAG = "lcd_display";

lv_disp_t *spi_lcd_display(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_handle_t panel,
                           int width, int height, int offset_x, int offset_y,
                           bool mirror_x, bool mirror_y, bool swap_xy)
{
    ESP_LOGI(TAG, "Initialize LVGL library");
    lv_init();

    ESP_LOGI(TAG, "Initialize LVGL port");
    lvgl_port_cfg_t port_cfg = ESP_LVGL_PORT_INIT_CONFIG();
    port_cfg.task_priority = 1;
#if CONFIG_SOC_CPU_CORES_NUM > 1
    port_cfg.task_affinity = 1;
#endif
    lvgl_port_init(&port_cfg);

    ESP_LOGI(TAG, "Add display screen");
    const lvgl_port_display_cfg_t disp_cfg = {
        .io_handle = panel_io,
        .panel_handle = panel,
        .control_handle = NULL,
        .buffer_size = 50 * width,
        .double_buffer = 1,
        .hres = (uint32_t)width,
        .vres = (uint32_t)height,
        .monochrome = false,
        /* Rotation values must be same as used in esp_lcd for initial settings of the screen */
        .rotation = {
            .swap_xy = swap_xy,
            .mirror_x = mirror_x,
            .mirror_y = mirror_y,
        },
        .flags = {
            .buff_dma = 1,
            .buff_spiram = 0,
            .sw_rotate = 0,
            .full_refresh = 0,
            .direct_mode = 0,
        }
    };
    lv_display_t *display = lvgl_port_add_disp(&disp_cfg);

    if(offset_x != 0 || offset_y != 0) {
        lv_disp_drv_t *disp_drv = display->driver;
        disp_drv->offset_x = offset_x;
        disp_drv->offset_y = offset_y;
    }

    return display;
}


lv_disp_t *rgb_lcd_display(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_handle_t panel,
                           int width, int height, int offset_x, int offset_y,
                           bool mirror_x, bool mirror_y, bool swap_xy)
{
    ESP_LOGI(TAG, "Initialize LVGL library");
    lv_init();

    ESP_LOGI(TAG, "Initialize LVGL port");
    lvgl_port_cfg_t port_cfg = ESP_LVGL_PORT_INIT_CONFIG();
    port_cfg.task_priority = 1;
    port_cfg.timer_period_ms = 50;
    lvgl_port_init(&port_cfg);

    ESP_LOGI(TAG, "Add display screen");
    const lvgl_port_display_cfg_t disp_cfg = {
        .io_handle = panel_io,
        .panel_handle = panel,
        .control_handle = NULL,
        .buffer_size = 20 * width,
        .double_buffer = 1,
        .hres = (uint32_t)width,
        .vres = (uint32_t)height,
        .monochrome = false,
        /* Rotation values must be same as used in esp_lcd for initial settings of the screen */
        .rotation = {
            .swap_xy = swap_xy,
            .mirror_x = mirror_x,
            .mirror_y = mirror_y,
        },
        .flags = {
            .buff_dma = 1,
            .buff_spiram = 0,
            .sw_rotate = 0,
            .full_refresh = 1,
            .direct_mode = 1,
        }
    };

    const lvgl_port_display_rgb_cfg_t rgb_cfg = {
        .flags = {
            .bb_mode = true,
            .avoid_tearing = true,
        }
    };
    lv_display_t *display = lvgl_port_add_disp_rgb(&disp_cfg, &rgb_cfg);

    if(offset_x != 0 || offset_y != 0) {
        lv_disp_drv_t *disp_drv = display->driver;
        disp_drv->offset_x = offset_x;
        disp_drv->offset_y = offset_y;
    }

    return display;
}

lv_disp_t *i80_lcd_display(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_handle_t panel,
                           int width, int height, int offset_x, int offset_y,
                           bool mirror_x, bool mirror_y, bool swap_xy)
{
    ESP_LOGI(TAG, "Initialize LVGL library");
    lv_init();

    ESP_LOGI(TAG, "Initialize LVGL port");
    lvgl_port_cfg_t port_cfg = ESP_LVGL_PORT_INIT_CONFIG();
    port_cfg.task_priority = 1;
#if CONFIG_SOC_CPU_CORES_NUM > 1
    port_cfg.task_affinity = 1;
#endif
    /* i80 并行总线，不使用 RGB 特殊接口 */
    lvgl_port_init(&port_cfg);

    ESP_LOGI(TAG, "Add display screen (i80 16-bit)");
    const lvgl_port_display_cfg_t disp_cfg = {
        .io_handle = panel_io,
        .panel_handle = panel,
        .control_handle = NULL,
        .buffer_size = 20 * width,
        .double_buffer = 1,
        .hres = (uint32_t)width,
        .vres = (uint32_t)height,
        .monochrome = false,
        .rotation = {
            .swap_xy = swap_xy,
            .mirror_x = mirror_x,
            .mirror_y = mirror_y,
        },
        .flags = {
            .buff_dma = 0,
            .buff_spiram = 1,
            .sw_rotate = 0,
            .full_refresh = 0,
            .direct_mode = 0,
        }
    };
    lv_display_t *display = lvgl_port_add_disp(&disp_cfg);

    if(offset_x != 0 || offset_y != 0) {
        lv_disp_drv_t *disp_drv = display->driver;
        disp_drv->offset_x = offset_x;
        disp_drv->offset_y = offset_y;
    }

    return display;
}

