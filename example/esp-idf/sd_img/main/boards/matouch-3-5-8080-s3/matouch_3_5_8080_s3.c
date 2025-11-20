#include <esp_log.h>
#include <esp_lcd_panel_vendor.h>
#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_ops.h>
#include <driver/spi_common.h>
#include <driver/ledc.h>
#include <driver/i2c_master.h>

#include "esp_lcd_ili9488.h"
#include "esp_lcd_touch_ft6x36.h"



#include "config.h"
#include "board.h"

#include "esp_lvgl_port.h"


#include "lv_demos.h"
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
    // for sdcard
    const spi_bus_config_t spi2_cfg = {
        .sclk_io_num = SPI2_CLK_PIN,
        .mosi_io_num = SPI2_MOSI_PIN,
        .miso_io_num = SPI2_MISO_PIN,
        .quadhd_io_num = -1,
        .quadwp_io_num = -1,
        .max_transfer_sz = 4000,
    };
    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &spi2_cfg, SPI_DMA_CH_AUTO)); 
}


void display_init(void)
{
    gpio_config_t bl_cfg = {
        .pin_bit_mask = 1ULL << DISPLAY_BACKLIGHT_PIN,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&bl_cfg));
    gpio_set_level(DISPLAY_BACKLIGHT_PIN, 1);

    gpio_config_t rd_cfg = {
        .pin_bit_mask = 1ULL << DISPLAY_RD_PIN,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&rd_cfg));
    gpio_set_level(DISPLAY_RD_PIN, 1);

    esp_lcd_i80_bus_handle_t i80_bus = NULL;
    esp_lcd_i80_bus_config_t bus_config = {
        .clk_src = LCD_CLK_SRC_DEFAULT,
        .dc_gpio_num = DISPLAY_DC_PIN,
        .wr_gpio_num = DISPLAY_CLK_PIN,
        .data_gpio_nums = {
            EXAMPLE_LCD_IO_DATA0,
            EXAMPLE_LCD_IO_DATA1,
            EXAMPLE_LCD_IO_DATA2,
            EXAMPLE_LCD_IO_DATA3,
            EXAMPLE_LCD_IO_DATA4,
            EXAMPLE_LCD_IO_DATA5,
            EXAMPLE_LCD_IO_DATA6,
            EXAMPLE_LCD_IO_DATA7,
            EXAMPLE_LCD_IO_DATA8,
            EXAMPLE_LCD_IO_DATA9,
            EXAMPLE_LCD_IO_DATA10,
            EXAMPLE_LCD_IO_DATA11,
            EXAMPLE_LCD_IO_DATA12,
            EXAMPLE_LCD_IO_DATA13,
            EXAMPLE_LCD_IO_DATA14,
            EXAMPLE_LCD_IO_DATA15,
        },
        .bus_width = 16,
        .max_transfer_bytes = DISPLAY_WIDTH * DISPLAY_HEIGHT * sizeof(uint16_t),
    };
    ESP_ERROR_CHECK(esp_lcd_new_i80_bus(&bus_config, &i80_bus));

    esp_lcd_panel_io_handle_t io_handle = NULL;
    esp_lcd_panel_io_i80_config_t io_cfg = {
        .cs_gpio_num = DISPLAY_CS_PIN,
        .pclk_hz = 10 * 1000 * 1000,
        .trans_queue_depth = 10,
        .dc_levels = {
            .dc_idle_level = 0,
            .dc_cmd_level = 0,
            .dc_dummy_level = 0,
            .dc_data_level = 1,
        },
        .on_color_trans_done = NULL,
        .user_ctx = NULL,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i80(i80_bus, &io_cfg, &io_handle));

    ESP_LOGI(TAG, "Install ILLI9488 LCD control panel");

    esp_lcd_panel_handle_t disp_panel = NULL;
    esp_lcd_panel_dev_config_t lcd_dev_config = {
        .reset_gpio_num = DISPLAY_RST_PIN,
        .bits_per_pixel = 16,
        .rgb_ele_order = DISPLAY_RGB_ORDER,
        .flags = {
            .reset_active_high = 0
        }
    };

    ESP_ERROR_CHECK(esp_lcd_new_panel_ili9488(io_handle, &lcd_dev_config, DISPLAY_WIDTH * 20 * sizeof(uint16_t), &disp_panel));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(disp_panel));
    ESP_ERROR_CHECK(esp_lcd_panel_init(disp_panel));
    // ESP_ERROR_CHECK(esp_lcd_panel_invert_color(disp_panel, true));
    // ESP_ERROR_CHECK(esp_lcd_panel_swap_xy(disp_panel, false));
    // ESP_ERROR_CHECK(esp_lcd_panel_mirror(disp_panel, false, false));
    // ESP_ERROR_CHECK(esp_lcd_panel_set_gap(disp_panel, 0, 0));
    // ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(disp_panel, true));

    disp = spi_lcd_display(io_handle, disp_panel,
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
    esp_lcd_touch_config_t tp_cfg = {
        .x_max = DISPLAY_WIDTH,
        .y_max = DISPLAY_HEIGHT,
        .rst_gpio_num = GPIO_NUM_NC,
        .int_gpio_num = GPIO_NUM_NC,
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

    esp_lcd_touch_handle_t tp = NULL;
    esp_lcd_panel_io_handle_t tp_io_handle = NULL;
    esp_lcd_panel_io_i2c_config_t  tp_io_cgf = ESP_LCD_TOUCH_IO_I2C_FT6x36_CONFIG();
    tp_io_cgf.scl_speed_hz = 400000;
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c(_i2c_bus, &tp_io_cgf, &tp_io_handle));
    ESP_LOGI(TAG, "Initialize touch controller");
    ESP_ERROR_CHECK(esp_lcd_touch_new_i2c_ft6x36(tp_io_handle, &tp_cfg, &tp));

    const lvgl_port_touch_cfg_t touch_cfg = {
        .disp = disp,
        .handle = tp,
    };
    lvgl_port_add_touch(&touch_cfg);
    ESP_LOGI(TAG, "Touch panel initialized successfully");
}





void board_init(void)
{
    // board = (board_t)malloc(sizeof(struct board_handle_t));
    // assert(board);

    spi_init();
    display_init();

    i2c_init();
    touch_init(); 

    /*        other init      */

#if CONFIG_SDCARD_ENABLE
    ESP_ERROR_CHECK(bsp_sdcard_mount(SDCARD_MOUNT_POINT, SDCARD_CS_PIN));
#endif
    ESP_LOGI(TAG, "Board initialized successfully");
}