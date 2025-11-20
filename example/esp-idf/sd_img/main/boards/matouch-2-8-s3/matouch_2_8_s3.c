#include <esp_log.h>
#include <esp_lcd_panel_vendor.h>
#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_ops.h>
#include <driver/spi_common.h>
#include <driver/ledc.h>
#include <esp_log.h>
#include <driver/i2c_master.h>



#include "config.h"
#include "board.h"
#include "esp_lcd_touch_gt911.h"
#include "esp_lvgl_port.h"


#include "lv_demos.h"
#include "lcd_display.h"



static const char *TAG = "board";

//@brief board handle
board_t board_handle = NULL;

// @brief i2c 
i2c_master_bus_handle_t _i2c_bus = NULL;

// @brief display&touch
static lv_display_t *disp = NULL;
static void i2c_init(void)
{
    // Initialize I2C peripheral
    i2c_master_bus_config_t i2c_bus_cfg = {
        .i2c_port = I2C_NUM_1,
        .sda_io_num = TOUCH_SDA_PIN,
        .scl_io_num = TOUCH_SCL_PIN,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .flags = {
            .enable_internal_pullup = 1,
        },
    };
    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_bus_cfg, &_i2c_bus));

    /*
     * =====================================================================================
     * 手动控制GT911复位时序以选择0x5D地址
     * =====================================================================================
     */
    // 1. 将RST(18)和INT(14)引脚配置为输出模式
    gpio_set_direction(TOUCH_RST_PIN, GPIO_MODE_OUTPUT); // RST
    gpio_set_direction(TOUCH_INT_PIN, GPIO_MODE_OUTPUT); // INT
    // 2. 将INT引脚拉低
    gpio_set_level(TOUCH_INT_PIN, 0);
    // 3. 执行复位操作：先拉低RST，短暂延时后拉高
    gpio_set_level(TOUCH_RST_PIN, 0);
    vTaskDelay(pdMS_TO_TICKS(10));
    gpio_set_level(TOUCH_RST_PIN, 1);
    vTaskDelay(pdMS_TO_TICKS(50));

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
    const spi_bus_config_t buscfg = {
        .sclk_io_num = DISPLAY_CLK_PIN,
        .mosi_io_num = DISPLAY_MOSI_PIN,
        .quadhd_io_num = -1,
        .quadwp_io_num = -1,
        .max_transfer_sz = DISPLAY_WIDTH * 50 * sizeof(uint16_t)
    };
    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO)); 
}


void display_init(void)
{
    // Setup LEDC peripheral for PWM backlight control
    const ledc_channel_config_t LCD_backlight_channel = {
        .gpio_num = DISPLAY_BACKLIGHT_PIN,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = 1,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = 1,
        .duty = 0,
        .hpoint = 0
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

    esp_lcd_panel_io_handle_t io_handle = NULL;
    esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = DISPLAY_DC_PIN,
        .cs_gpio_num = DISPLAY_CS_PIN,
        .pclk_hz = 40 * 1000 * 1000,
        .trans_queue_depth = 10,
        .lcd_cmd_bits = 8,
        .spi_mode = 0,
        .lcd_param_bits = 8,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi(SPI2_HOST, &io_config, &io_handle));

    esp_lcd_panel_handle_t disp_panel = NULL;

    ESP_LOGI(TAG, "Install ST7789V LCD control panel");

    esp_lcd_panel_dev_config_t lcd_dev_config = {
        .reset_gpio_num = DISPLAY_RST_PIN,
        .bits_per_pixel = 16,
        .rgb_ele_order = DISPLAY_RGB_ORDER,
        .data_endian = LCD_RGB_DATA_ENDIAN_BIG
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(io_handle, &lcd_dev_config, &disp_panel));
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
    const esp_lcd_touch_config_t tp_cfg = {
        .x_max = DISPLAY_WIDTH,
        .y_max = DISPLAY_HEIGHT,
        .rst_gpio_num = GPIO_NUM_NC, // 不能使用TOUCH_RST_PIN，因为 复用上面手动复位
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
    board_handle = (board_t)malloc(sizeof(struct board_handle_t));
    assert(board_handle);

    spi_init();
    display_init();

    i2c_init();
    touch_init();

    ESP_LOGI(TAG, "Setting LCD backlight: %d%%", 80);
    uint32_t duty_cycle = (1023 * 80) / 100; // LEDC resolution set to 10bits, thus: 100% = 1023
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, 1, duty_cycle));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, 1));

    audio_cfg_t audio_config = {
        .input_sample_rate  = AUDIO_INPUT_SAMPLE_RATE,
        .output_sample_rate = AUDIO_OUTPUT_SAMPLE_RATE,
        .spk_bclk           = AUDIO_I2S_SPK_GPIO_BCLK,
        .spk_ws             = AUDIO_I2S_SPK_GPIO_LRCK,
        .spk_dout           = AUDIO_I2S_SPK_GPIO_DOUT,
        .mic_sck            = AUDIO_I2S_MIC_GPIO_SCK,
        .mic_ws             = AUDIO_I2S_MIC_GPIO_WS,
        .mic_din            = AUDIO_I2S_MIC_GPIO_DIN,
        .spk_slot_mask      = I2S_STD_SLOT_LEFT,
        .mic_slot_mask      = I2S_STD_SLOT_LEFT,
    };
    audio_simplex_init(&board_handle->audio_handle, &audio_config);
}