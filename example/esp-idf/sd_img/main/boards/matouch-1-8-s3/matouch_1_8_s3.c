#include <esp_log.h>
#include <esp_lcd_panel_vendor.h>
#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_ops.h>
#include <driver/spi_common.h>
#include <esp_log.h>
#include <driver/i2c_master.h>



#include "config.h"
#include "esp_lcd_sh8601.h"
#include "esp_lvgl_port.h"
#include "esp_lcd_touch_ft5x06.h"

#include "lv_demos.h"
#include "lcd_display.h"

// #include "bsp_spiffs.h"



static const char *TAG = "board";

// @brief i2c 
i2c_master_bus_handle_t _i2c_bus = NULL;

// @brief display&touch
static lv_display_t *disp = NULL;


static const sh8601_lcd_init_cmd_t lcd_init_cmds[] = {
    // set display to qspi mode
    {0x11, (uint8_t[]){0x00}, 0, 120}, // SH8601_C_SLPOUT, 延时 SH8601_SLPOUT_DELAY
    
    // 2. Normal Display mode on
    {0x13, (uint8_t[]){0x00}, 0, 0},   // SH8601_C_NORON
    
    // 3. Inversion Off
    {0x20, (uint8_t[]){0x00}, 0, 0},   // SH8601_C_INVOFF
    
    // 4. Interface Pixel Format (16bit/pixel)
    {0x3A, (uint8_t[]){0x05}, 1, 0},   // SH8601_W_PIXFMT, Data: 0x05
    
    // 5. Display on
    {0x29, (uint8_t[]){0x00}, 0, 0},   // SH8601_C_DISPON
    
    // 6. Write CTRL Display1 (Brightness Control On & Display Dimming On)
    {0x53, (uint8_t[]){0x28}, 1, 0},   // SH8601_W_WCTRLD1, Data: 0x28
    
    // 7. Brightness adjustment (设置为 0x00)
    {0x51, (uint8_t[]){0x00}, 1, 0},   // SH8601_W_WDBRIGHTNESSVALNOR, Data: 0x00
    
    // 8. Write CE (Contrast Off)
    {0x58, (uint8_t[]){0x00}, 1, 10},  // SH8601_W_WCE, Data: 0x00, 延时 10ms

    // ---- 来自 setRotation(0) ----
    
    // 9. Memory data access control (Rotation 0: RGB, no flip)
    {0x36, (uint8_t[]){0x00}, 1, 0},   // SH8601_W_MADCTL, Data: 0x00 (SH8601_MADCTL_COLOR_ORDER)

    // ---- 必要的亮度设置 (基于 .ino 示例) ----
    
    // 10. 设置亮度为最亮 (0xFF)，否则屏幕将是黑色
    {0x51, (uint8_t[]){0xFF}, 1, 0},   // SH8601_W_WDBRIGHTNESSVALNOR, Data: 0xFF
};


void i2c_init(void)
{
    // Initialize I2C peripheral
    i2c_master_bus_config_t i2c_bus_cfg = {
        .i2c_port = I2C_NUM_0,
        .sda_io_num = TOUCH_SDA_PIN,
        .scl_io_num = TOUCH_SCL_PIN,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .flags = {
            .enable_internal_pullup = 1,
        },
    };
    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_bus_cfg, &_i2c_bus));
}


void spi_init(void)
{
    const spi_bus_config_t buscfg = SH8601_PANEL_BUS_QSPI_CONFIG(EXAMPLE_PIN_NUM_LCD_SCLK,
                                                                 EXAMPLE_PIN_NUM_LCD_DATA0,
                                                                 EXAMPLE_PIN_NUM_LCD_DATA1,
                                                                 EXAMPLE_PIN_NUM_LCD_DATA2,
                                                                 EXAMPLE_PIN_NUM_LCD_DATA3,
                                                                 DISPLAY_WIDTH * 50 * sizeof(uint16_t));
    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO)); 
}


void display_init(void)
{ 
    esp_lcd_panel_io_handle_t io_handle = NULL;
    const esp_lcd_panel_io_spi_config_t io_config = SH8601_PANEL_IO_QSPI_CONFIG(EXAMPLE_PIN_NUM_LCD_CS,
                                                    NULL,
                                                    NULL);
    ESP_LOGI(TAG, "Install LCD driver");
    sh8601_vendor_config_t vendor_config = {
        .init_cmds = lcd_init_cmds,
        .init_cmds_size = sizeof(lcd_init_cmds) / sizeof(lcd_init_cmds[0]),
        .flags = {
            .use_qspi_interface = 1,
        },
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi(SPI2_HOST, &io_config, &io_handle));

    esp_lcd_panel_handle_t panel_handle = NULL;
    const esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = EXAMPLE_PIN_NUM_LCD_RST,
        .rgb_ele_order = DISPLAY_RGB_ORDER,
        .bits_per_pixel = 16,
        .vendor_config = &vendor_config,
    };
    ESP_LOGI(TAG, "Install SH8601 panel driver");
    ESP_ERROR_CHECK(esp_lcd_new_panel_sh8601(io_handle, &panel_config, &panel_handle));
    // ESP_ERROR_CHECK(esp_lcd_panel_set_gap(panel_handle, 0x06, 0));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
    // user can flush pre-defined pattern to the screen before we turn on the screen or backlight
    // ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, false));

    gpio_config_t en_conf = {
        .pin_bit_mask = 1ULL << GPIO_NUM_9,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&en_conf);

    gpio_set_level(GPIO_NUM_9, 1);

    vTaskDelay(pdMS_TO_TICKS(200));

    disp = spi_lcd_display(io_handle, panel_handle,
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
        .rst_gpio_num = EXAMPLE_PIN_NUM_TOUCH_RST,
        .int_gpio_num = EXAMPLE_PIN_NUM_TOUCH_INT,
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
    esp_lcd_panel_io_i2c_config_t tp_io_config = ESP_LCD_TOUCH_IO_I2C_FT5x06_CONFIG();
    tp_io_config.scl_speed_hz = 400*  1000;
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c(_i2c_bus, &tp_io_config, &tp_io_handle));
    ESP_LOGI(TAG, "Initialize touch controller");
    ESP_ERROR_CHECK(esp_lcd_touch_new_i2c_ft5x06(tp_io_handle, &tp_cfg, &tp));
    const lvgl_port_touch_cfg_t touch_cfg = {
        .disp = disp,
        .handle = tp,
    };
    lvgl_port_add_touch(&touch_cfg);
    ESP_LOGI(TAG, "Touch panel initialized successfully");
}





void board_init(void)
{

    #if CONFIG_BSP_SPIFFS
    bsp_spiffs_mount();
    #endif

    i2c_init();

    spi_init();

    display_init();

    touch_init();


    // lvgl_port_lock(0);

    // lv_demo_widgets();

    // lvgl_port_unlock();

}