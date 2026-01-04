#include <esp_log.h>
#include <esp_lcd_panel_vendor.h>
#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_ops.h>
#include <driver/spi_common.h>
#include <driver/ledc.h>
#include <driver/i2c_master.h>

#include "esp_lcd_gc9a01.h"
#include "esp_lcd_touch_cst816s.h"



#include "config.h"
#include "board.h"

#include "esp_lvgl_port.h"
#include "lcd_display.h"

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



const static gc9a01_lcd_init_cmd_t lcd_init_cmds[] = {
    {0xEF, NULL, 0, 0},
    {0xEB, (uint8_t []){0x14}, 1, 0},
    {0xFE, NULL, 0, 0},
    {0xEF, NULL, 0, 0},
    {0xEB, (uint8_t []){0x14}, 1, 0},
    {0x84, (uint8_t []){0x40}, 1, 0},
    {0x85, (uint8_t []){0xFF}, 1, 0},
    {0x86, (uint8_t []){0xFF}, 1, 0},
    {0x87, (uint8_t []){0xFF}, 1, 0},
    {0x88, (uint8_t []){0x0A}, 1, 0},
    {0x89, (uint8_t []){0x21}, 1, 0},
    {0x8A, (uint8_t []){0x00}, 1, 0},
    {0x8B, (uint8_t []){0x80}, 1, 0},
    {0x8C, (uint8_t []){0x01}, 1, 0},
    {0x8D, (uint8_t []){0x01}, 1, 0},
    {0x8E, (uint8_t []){0xFF}, 1, 0},
    {0x8F, (uint8_t []){0xFF}, 1, 0},
    {0xB6, (uint8_t []){0x00, 0x20}, 2, 0},
    {0x3A, (uint8_t []){0x05}, 1, 0}, // Pixel Format 16bit (RGB565)
    {0x90, (uint8_t []){0x08, 0x08, 0x08, 0x08}, 4, 0},
    {0xBD, (uint8_t []){0x06}, 1, 0},
    {0xBC, (uint8_t []){0x00}, 1, 0},
    {0xFF, (uint8_t []){0x60, 0x01, 0x04}, 3, 0},
    {0xC3, (uint8_t []){0x13}, 1, 0},
    {0xC4, (uint8_t []){0x13}, 1, 0},
    {0xC9, (uint8_t []){0x22}, 1, 0},
    {0xBE, (uint8_t []){0x11}, 1, 0},
    {0xE1, (uint8_t []){0x10, 0x0E}, 2, 0},
    {0xDF, (uint8_t []){0x21, 0x0C, 0x02}, 3, 0},
    {0xF0, (uint8_t []){0x45, 0x09, 0x08, 0x08, 0x26, 0x2A}, 6, 0},
    {0xF1, (uint8_t []){0x43, 0x70, 0x72, 0x36, 0x37, 0x6F}, 6, 0},
    {0xF2, (uint8_t []){0x45, 0x09, 0x08, 0x08, 0x26, 0x2A}, 6, 0},
    {0xF3, (uint8_t []){0x43, 0x70, 0x72, 0x36, 0x37, 0x6F}, 6, 0},
    {0xED, (uint8_t []){0x1B, 0x0B}, 2, 0},
    {0xAE, (uint8_t []){0x77}, 1, 0},
    {0xCD, (uint8_t []){0x63}, 1, 0},
    {0x70, (uint8_t []){0x07, 0x07, 0x04, 0x0E, 0x0F, 0x09, 0x07, 0x08, 0x03}, 9, 0},
    {0xE8, (uint8_t []){0x34}, 1, 0},
    {0x62, (uint8_t []){0x18, 0x0D, 0x71, 0xED, 0x70, 0x70, 0x18, 0x0F, 0x71, 0xEF, 0x70, 0x70}, 12, 0},
    {0x63, (uint8_t []){0x18, 0x11, 0x71, 0xF1, 0x70, 0x70, 0x18, 0x13, 0x71, 0xF3, 0x70, 0x70}, 12, 0},
    {0x64, (uint8_t []){0x28, 0x29, 0xF1, 0x01, 0xF1, 0x00, 0x07}, 7, 0},
    {0x66, (uint8_t []){0x3C, 0x00, 0xCD, 0x67, 0x45, 0x45, 0x10, 0x00, 0x00, 0x00}, 10, 0},
    {0x67, (uint8_t []){0x00, 0x3C, 0x00, 0x00, 0x00, 0x01, 0x54, 0x10, 0x32, 0x98}, 10, 0},
    {0x74, (uint8_t []){0x10, 0x85, 0x80, 0x00, 0x00, 0x4E, 0x00}, 7, 0},
    {0x98, (uint8_t []){0x3E, 0x07}, 2, 0},
    {0x35, NULL, 0, 0}, // TEON (Tearing Effect Line On)
    
    // 退出睡眠模式，延时 120ms (0x11 = Sleep Out)
    {0x11, NULL, 0, 120},
    
    // 开启显示，延时 20ms (0x29 = Display On)
    {0x29, NULL, 0, 20},
};

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
    const spi_bus_config_t spi2_cfg = {
        .mosi_io_num    = SPI2_MOSI_PIN,
        .miso_io_num    = SPI2_MISO_PIN,
        .sclk_io_num    = SPI2_CLK_PIN,
        .quadhd_io_num  = -1,
        .quadwp_io_num  = -1,
        .data4_io_num   = -1, 
        .data5_io_num   = -1,
        .data6_io_num   = -1,
        .data7_io_num   = -1,
        .max_transfer_sz = DISPLAY_WIDTH * DISPLAY_HEIGHT * sizeof(uint16_t)
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

    ESP_LOGI(TAG, "Install panel IO");

    esp_lcd_panel_io_handle_t io_handle = NULL;
    const esp_lcd_panel_io_spi_config_t io_config = GC9A01_PANEL_IO_SPI_CONFIG(
                                                        DISPLAY_CS_PIN, 
                                                        DISPLAY_DC_PIN, 
                                                        NULL, 
                                                        NULL);
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi(SPI2_HOST, &io_config, &io_handle));

    ESP_LOGI(TAG, "Install GC9A01 panel driver");
    esp_lcd_panel_handle_t disp_panel = NULL;
    
    gc9a01_vendor_config_t vendor_config = {  // Uncomment these lines if use custom initialization commands
        .init_cmds = lcd_init_cmds,
        .init_cmds_size = sizeof(lcd_init_cmds) / sizeof(gc9a01_lcd_init_cmd_t)
    };
    const esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = DISPLAY_RST_PIN,          // Set to -1 if not use
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_BGR,  // RGB element order: R-G-B
        .bits_per_pixel = 16,                        // Implemented by LCD command `3Ah` (16/18)
        .vendor_config = &vendor_config          // Uncomment this line if use custom initialization commands
    };

    ESP_ERROR_CHECK(esp_lcd_new_panel_gc9a01(io_handle, &panel_config, &disp_panel));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(disp_panel));
    ESP_ERROR_CHECK(esp_lcd_panel_init(disp_panel));
    ESP_ERROR_CHECK(esp_lcd_panel_invert_color(disp_panel, true));
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(disp_panel, true));


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

    esp_lcd_touch_handle_t tp = NULL;
    esp_lcd_panel_io_handle_t tp_io_handle = NULL;
    esp_lcd_panel_io_i2c_config_t  tp_io_cgf = ESP_LCD_TOUCH_IO_I2C_CST816S_CONFIG();
    tp_io_cgf.scl_speed_hz = 400000;
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c(_i2c_bus, &tp_io_cgf, &tp_io_handle));
    ESP_LOGI(TAG, "Initialize touch controller");
    ESP_ERROR_CHECK(esp_lcd_touch_new_i2c_cst816s(tp_io_handle, &tp_cfg, &tp));

    const lvgl_port_touch_cfg_t touch_cfg = {
        .disp = disp,
        .handle = tp,
    };
    lvgl_port_add_touch(&touch_cfg);
    ESP_LOGI(TAG, "Touch panel initialized successfully");
}





void board_init(void)
{
    board_handle = (board_handle_t)malloc(sizeof(struct board_t));
    assert(board_handle);

    spi_init();

    display_init();

    i2c_init();

    touch_init(); 

    /*********************************************** 
                    other function
    1. pcf85063a(rtc)
    2. sdcard
    ************************************************/

#if CONFIG_SDCARD_ENABLE
    ESP_ERROR_CHECK(bsp_sdcard_mount(SDCARD_MOUNT_POINT, SDCARD_CS_PIN));
#endif


#if CONFIG_PCF85063A_ENABLE
    ESP_ERROR_CHECK(bsp_pcf8563_init(&board_handle->pcf8563, _i2c_bus));
#endif

    ESP_LOGI(TAG, "Board initialized successfully");

}