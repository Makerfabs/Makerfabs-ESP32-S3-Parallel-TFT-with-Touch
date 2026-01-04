#include <esp_log.h>
#include <esp_lcd_mipi_dsi.h>
#include <esp_lcd_panel_ops.h>
#include <esp_ldo_regulator.h>
#include <driver/i2c_master.h>
#include <driver/gpio.h>
#include <driver/ledc.h>
#include <esp_check.h>
#include <sd_pwr_ctrl_by_on_chip_ldo.h>

#include "esp_lcd_touch_gt911.h"

#include "config.h"
#include "board.h"

#include "esp_lvgl_port.h"

#include "lcd_display.h"

#include "esp_lcd_jd9165.h"

#if CONFIG_SPIFFS_ENABLE
#include "bsp_spiffs.h"
#endif



static const char *TAG = "board";

//@brief board handle
board_handle_t board_handle = NULL;

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


esp_err_t display_init(void)
{
    esp_err_t err = ESP_OK;

    if (DISPLAY_BACKLIGHT_PIN != GPIO_NUM_NC) {
        // gpio_config_t bl_cfg = {
        //     .pin_bit_mask = 1ULL << DISPLAY_BACKLIGHT_PIN,
        //     .mode = GPIO_MODE_OUTPUT,
        //     .pull_up_en = GPIO_PULLUP_DISABLE,
        //     .pull_down_en = GPIO_PULLDOWN_DISABLE,
        //     .intr_type = GPIO_INTR_DISABLE,
        // };
        // ESP_ERROR_CHECK(gpio_config(&bl_cfg));
        // gpio_set_level(DISPLAY_BACKLIGHT_PIN, 0);
        const ledc_channel_config_t LCD_backlight_channel = {
            .gpio_num = DISPLAY_BACKLIGHT_PIN,
            .speed_mode = LEDC_LOW_SPEED_MODE,
            .channel = 1,
            .intr_type = LEDC_INTR_DISABLE,
            .timer_sel = 1,
            .duty = 0,
            .hpoint = 0,
            .flags.output_invert = 1
        };
        const ledc_timer_config_t LCD_backlight_timer = {
            .speed_mode = LEDC_LOW_SPEED_MODE,
            .duty_resolution = LEDC_TIMER_10_BIT,
            .timer_num = 1,
            .freq_hz = 5000,
            .clk_cfg = LEDC_AUTO_CLK
        };

        ESP_ERROR_CHECK(ledc_timer_config(&LCD_backlight_timer));
        ESP_ERROR_CHECK(ledc_channel_config(&LCD_backlight_channel));

        uint32_t duty_cycle = (1023 * 100) / 100; // LEDC resolution set to 10bits, thus: 100% = 1023
        ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, 1, duty_cycle));
        ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, 1));
    }

    ESP_LOGI(TAG, "Install Unkonwn panel driver");

    static esp_ldo_channel_handle_t phy_pwr_chan = NULL;
    esp_ldo_channel_config_t ldo_cfg = {
        .chan_id = 3,
        .voltage_mv = 2500,
    };
    ESP_RETURN_ON_ERROR(esp_ldo_acquire_channel(&ldo_cfg, &phy_pwr_chan), TAG, "Acquire LDO channel for DPHY failed");
    ESP_LOGI(TAG, "MIPI DSI PHY Powered on");

    esp_lcd_dsi_bus_handle_t mipi_dsi_bus;
    esp_lcd_dsi_bus_config_t bus_config = {
        .bus_id = 0,
        .num_data_lanes = 2,
        .phy_clk_src = MIPI_DSI_PHY_CLK_SRC_DEFAULT,
        .lane_bit_rate_mbps = 900,
    };
    ESP_RETURN_ON_ERROR(esp_lcd_new_dsi_bus(&bus_config, &mipi_dsi_bus), TAG, "New DSI bus init failed");

    ESP_LOGI(TAG, "Install MIPI DSI LCD control panel");
    // we use DBI interface to send LCD commands and parameters
    esp_lcd_panel_io_handle_t io;
    esp_lcd_dbi_io_config_t dbi_config = {
        .virtual_channel = 0,
        .lcd_cmd_bits = 8,   // according to the LCD ILI9881C spec
        .lcd_param_bits = 8, // according to the LCD ILI9881C spec
    };
    ESP_RETURN_ON_ERROR(esp_lcd_new_panel_io_dbi(mipi_dsi_bus, &dbi_config, &io), TAG, "New panel IO failed");

    esp_lcd_panel_handle_t disp_panel = NULL;

    ESP_LOGI(TAG, "Install JD9165 LCD control panel");

    esp_lcd_dpi_panel_config_t dpi_config = JD9165_1024_600_PANEL_60HZ_DPI_CONFIG(LCD_COLOR_PIXEL_FORMAT_RGB565);
    dpi_config.num_fbs = 2;

    jd9165_vendor_config_t vendor_config = {
        .mipi_config = {
            .dsi_bus = mipi_dsi_bus,
            .dpi_config = &dpi_config,
        },
    };
    esp_lcd_panel_dev_config_t lcd_dev_config = {
        .bits_per_pixel = 16,
        .rgb_ele_order = ESP_LCD_COLOR_SPACE_RGB,
        .reset_gpio_num = DISPLAY_RST_PIN,
        .vendor_config = &vendor_config,
    };
    ESP_RETURN_ON_ERROR(esp_lcd_new_panel_jd9165(io, &lcd_dev_config, &disp_panel), TAG, "New LCD panel EK79007 failed");
    ESP_RETURN_ON_ERROR(esp_lcd_panel_reset(disp_panel), TAG, "LCD panel reset failed");
    ESP_RETURN_ON_ERROR(esp_lcd_panel_init(disp_panel), TAG, "LCD panel init failed");

    disp = mipi_lcd_display(io, disp_panel,
                           DISPLAY_WIDTH, DISPLAY_HEIGHT,
                           DISPLAY_OFFSET_X, DISPLAY_OFFSET_Y,
                           DISPLAY_MIRROR_X, DISPLAY_MIRROR_Y,
                           DISPLAY_SWAP_XY);
    if(disp == NULL) {
        ESP_LOGE(TAG, "Failed to add display to LVGL");
        err = ESP_FAIL;
    }

    return err;
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
    // board_handle = (board_handle_t)malloc(sizeof(struct board_t));
    // assert(board_handle);

    spi_init();

    display_init();

    i2c_init();

    touch_init(); 

    /* ========== other init =========== */

    // ESP_ERROR_CHECK(bsp_sdcard_mount(SDCARD_MOUNT_POINT, SDCARD_CS_PIN));


    ESP_LOGI(TAG, "Board initialized successfully");

}