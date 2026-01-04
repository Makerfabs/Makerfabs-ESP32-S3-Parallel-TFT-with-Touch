#include <esp_log.h>
#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_ops.h>
#include <esp_lcd_panel_rgb.h>
#include <driver/spi_common.h>
#include <driver/i2c_master.h>
#include <driver/gpio.h>

#include "esp_lcd_touch_gt911.h"

#include "config.h"
#include "board.h"

#include "esp_lvgl_port.h"

#include "lcd_display.h"

#if CONFIG_SPIFFS_ENABLE
#include "bsp_spiffs.h"
#endif



static const char *TAG = "board";

//@brief board handle
board_t board = NULL;

// @brief i2c 
i2c_master_bus_handle_t _i2c_bus = NULL;

// @brief display&touch
static lv_disp_t *disp = NULL;

static void i2c_init(void)
{
    // Initialize I2C peripheral
    i2c_master_bus_config_t i2c_bus_cfg = {
        .i2c_port = I2C_NUM_1,
        .sda_io_num = TOUCH_SDA_PIN,
        .scl_io_num = TOUCH_SCL_PIN,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .intr_priority = 0,
        .trans_queue_depth = 0,
        .flags = {
            .enable_internal_pullup = 1,
        },
    };
    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_bus_cfg, &_i2c_bus));

    uint8_t address;
    printf("     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\r\n");
    for (int i = 0; i < 128; i += 16) {
        printf("%02x: ", i);
        for (int j = 0; j < 16; j++) {
            fflush(stdout);
            address = i + j;
            esp_err_t ret = i2c_master_probe(_i2c_bus, address, 50);
            if (ret == ESP_OK) {
                printf("%02x ", address);
            } else if (ret == ESP_ERR_TIMEOUT) {
                printf("UU ");
            } else {
                printf("-- ");
            }
        }
        printf("\r\n");
    }
}


void spi_init(void)
{
    // sdcard
    // const spi_bus_config_t spi2_cfg = {
    //     .mosi_io_num    = SPI2_MOSI_PIN,
    //     .miso_io_num    = SPI2_MISO_PIN,
    //     .sclk_io_num    = SPI2_CLK_PIN,
    //     .quadhd_io_num  = -1,
    //     .quadwp_io_num  = -1,
    // };
    // ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &spi2_cfg, SPI_DMA_CH_AUTO));

}


void display_init(void)
{
    if (DISPLAY_BACKLIGHT_PIN != GPIO_NUM_NC) {
        gpio_config_t bl_cfg = {
            .pin_bit_mask = 1ULL << DISPLAY_BACKLIGHT_PIN,
            .mode = GPIO_MODE_OUTPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE,
        };
        ESP_ERROR_CHECK(gpio_config(&bl_cfg));
        gpio_set_level(DISPLAY_BACKLIGHT_PIN, 0);
    }

    ESP_LOGI(TAG, "Install Unkonwn panel driver");

    esp_lcd_rgb_panel_config_t rgb_config ={
        .clk_src = LCD_CLK_SRC_DEFAULT,
        .psram_trans_align = 64,
        .bounce_buffer_size_px = 10 * DISPLAY_WIDTH,
        .num_fbs = 2,
        .data_width = 16,
        .bits_per_pixel = 16,
        .de_gpio_num = EXAMPLE_LCD_IO_RGB_DE,
        .pclk_gpio_num = EXAMPLE_LCD_IO_RGB_PCLK,
        .vsync_gpio_num = EXAMPLE_LCD_IO_RGB_VSYNC,
        .hsync_gpio_num = EXAMPLE_LCD_IO_RGB_HSYNC,
        .flags.fb_in_psram = true,
        .disp_gpio_num = -1,
        .data_gpio_nums = {
            // BGR
            EXAMPLE_LCD_IO_RGB_B0,
            EXAMPLE_LCD_IO_RGB_B1,
            EXAMPLE_LCD_IO_RGB_B2,
            EXAMPLE_LCD_IO_RGB_B3,
            EXAMPLE_LCD_IO_RGB_B4,
            EXAMPLE_LCD_IO_RGB_G0,
            EXAMPLE_LCD_IO_RGB_G1,
            EXAMPLE_LCD_IO_RGB_G2,
            EXAMPLE_LCD_IO_RGB_G3,
            EXAMPLE_LCD_IO_RGB_G4,
            EXAMPLE_LCD_IO_RGB_G5,
            EXAMPLE_LCD_IO_RGB_R0,
            EXAMPLE_LCD_IO_RGB_R1,
            EXAMPLE_LCD_IO_RGB_R2,
            EXAMPLE_LCD_IO_RGB_R3,
            EXAMPLE_LCD_IO_RGB_R4,
        },
        .timings = {
            .pclk_hz = 16 * 1000 * 1000,
            .h_res = DISPLAY_WIDTH,
            .v_res = DISPLAY_HEIGHT,
            .hsync_back_porch = 16,
            .hsync_front_porch = 80,
            .hsync_pulse_width = 4,
            .vsync_back_porch = 4,
            .vsync_front_porch =22,
            .vsync_pulse_width = 4,
            .flags.pclk_idle_high = 1,
        },

    };

    esp_lcd_panel_handle_t disp_panel = NULL;
    ESP_ERROR_CHECK(esp_lcd_new_rgb_panel(&rgb_config, &disp_panel));

    ESP_ERROR_CHECK(esp_lcd_panel_reset(disp_panel));
    ESP_ERROR_CHECK(esp_lcd_panel_init(disp_panel));

    disp = rgb_lcd_display(NULL, disp_panel,
                           DISPLAY_WIDTH, DISPLAY_HEIGHT,
                           DISPLAY_OFFSET_X, DISPLAY_OFFSET_Y,
                           DISPLAY_MIRROR_X, DISPLAY_MIRROR_Y,
                           DISPLAY_SWAP_XY);
    if(disp == NULL) {
        ESP_LOGE(TAG, "Failed to add display to LVGL");
        return;
    }
}

void touch_init(void)
{
    const esp_lcd_touch_config_t tp_cfg = {
        .x_max = DISPLAY_WIDTH,
        .y_max = DISPLAY_HEIGHT,
        .rst_gpio_num = TOUCH_RST_PIN,
        .int_gpio_num = TOUCH_INT_PIN,
        .levels = {
            .reset = 0,
            .interrupt = 0,
        },
        .flags = {
            .swap_xy = 0,
            .mirror_x = 0,
            .mirror_y = 0,
        },
    };
    esp_lcd_touch_handle_t tp;
    esp_lcd_panel_io_handle_t tp_io_handle = NULL;
    esp_lcd_panel_io_i2c_config_t tp_io_config = ESP_LCD_TOUCH_IO_I2C_GT911_CONFIG();
    tp_io_config.scl_speed_hz = 100000;
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c(_i2c_bus, &tp_io_config, &tp_io_handle));
    ESP_ERROR_CHECK(esp_lcd_touch_new_i2c_gt911(tp_io_handle, &tp_cfg, &tp));
    assert(tp);

    const lvgl_port_touch_cfg_t touch_cfg = {
        .disp = disp,
        .handle = tp,
    };
    lvgl_port_add_touch(&touch_cfg);
    ESP_LOGI(TAG, "Touch panel initialized successfully");
}





void board_init(void)
{
    ESP_LOGI(TAG, "Initialize 7-0-unknown-gt911 board");
    // board = (board_t)malloc(sizeof(struct board_handle_t));
    // assert(board);

    spi_init();

    display_init();

    i2c_init();

    touch_init(); 

    /* ========== other init =========== */

    // ESP_ERROR_CHECK(bsp_sdcard_mount(SDCARD_MOUNT_POINT, SDCARD_CS_PIN));


    ESP_LOGI(TAG, "Board initialized successfully");

}