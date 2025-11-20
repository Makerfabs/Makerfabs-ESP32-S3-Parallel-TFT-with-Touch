#include <esp_log.h>
#include <esp_lcd_panel_vendor.h>
#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_ops.h>
#include <driver/spi_common.h>
#include <driver/ledc.h>
#include <driver/i2c_master.h>
#include "esp_lcd_panel_io_additions.h"
#include "esp_lcd_st7701.h"



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
static lv_display_t *disp = NULL;

static const st7701_lcd_init_cmd_t lcd_init_cmds[] = 
{
  //   cmd   data        data_size  delay_ms 1
  {0xFF,(uint8_t []){0x77,0x01,0x00,0x00,0x13},5,0},
  {0xEF,(uint8_t []){0x08},1,0},
  {0xFF,(uint8_t []){0x77,0x01,0x00,0x00,0x10},5,0},
  {0xC0,(uint8_t []){0xE5,0x02},2,0},
  {0xC1,(uint8_t []){0x15,0x0A},2,0},
  {0xC2,(uint8_t []){0x07,0x02},2,0},
  {0xCC,(uint8_t []){0x10},1,0},
  {0xB0,(uint8_t []){0x00,0x08,0x51,0x0D,0xCE,0x06,0x00,0x08,0x08,0x24,0x05,0xD0,0x0F,0x6F,0x36,0x1F},16,0},
  {0xB1,(uint8_t []){0x00,0x10,0x4F,0x0C,0x11,0x05,0x00,0x07,0x07,0x18,0x02,0xD3,0x11,0x6E,0x34,0x1F},16,0},
  {0xFF,(uint8_t []){0x77,0x01,0x00,0x00,0x11},5,0},
  {0xB0,(uint8_t []){0x4D},1,0},
  {0xB1,(uint8_t []){0x37},1,0},
  {0xB2,(uint8_t []){0x87},1,0},
  {0xB3,(uint8_t []){0x80},1,0},
  {0xB5,(uint8_t []){0x4A},1,0},
  {0xB7,(uint8_t []){0x85},1,0},
  {0xB8,(uint8_t []){0x21},1,0},
  {0xB9,(uint8_t []){0x00,0x13},2,0},
  {0xC0,(uint8_t []){0x09},1,0},
  {0xC1,(uint8_t []){0x78},1,0},
  {0xC2,(uint8_t []){0x78},1,0},
  {0xD0,(uint8_t []){0x88},1,0},
  {0xE0,(uint8_t []){0x80,0x00,0x02},3,100},
  {0xE1,(uint8_t []){0x0F,0xA0,0x00,0x00,0x10,0xA0,0x00,0x00,0x00,0x60,0x60},11,0},
  {0xE2,(uint8_t []){0x30,0x30,0x60,0x60,0x45,0xA0,0x00,0x00,0x46,0xA0,0x00,0x00,0x00},13,0},
  {0xE3,(uint8_t []){0x00,0x00,0x33,0x33},4,0},
  {0xE4,(uint8_t []){0x44,0x44},2,0},
  {0xE5,(uint8_t []){0x0F,0x4A,0xA0,0xA0,0x11,0x4A,0xA0,0xA0,0x13,0x4A,0xA0,0xA0,0x15,0x4A,0xA0,0xA0},16,0},
  {0xE6,(uint8_t []){0x00,0x00,0x33,0x33},4,0},
  {0xE7,(uint8_t []){0x44,0x44},2,0},
  {0xE8,(uint8_t []){0x10,0x4A,0xA0,0xA0,0x12,0x4A,0xA0,0xA0,0x14,0x4A,0xA0,0xA0,0x16,0x4A,0xA0,0xA0},16,0},
  {0xEB,(uint8_t []){0x02,0x00,0x4E,0x4E,0xEE,0x44,0x00},7,0},
  {0xED,(uint8_t []){0xFF,0xFF,0x04,0x56,0x72,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x27,0x65,0x40,0xFF,0xFF},16,0},
  {0xEF,(uint8_t []){0x08,0x08,0x08,0x40,0x3F,0x64},6,0},
  {0xFF,(uint8_t []){0x77,0x01,0x00,0x00,0x13},5,0},
  {0xE8,(uint8_t []){0x00,0x0E},2,0},
  {0xFF,(uint8_t []){0x77,0x01,0x00,0x00,0x00},5,0},
  {0x11,(uint8_t []){0x00},0,120},
  {0xFF,(uint8_t []){0x77,0x01,0x00,0x00,0x13},5,0},
  {0xE8,(uint8_t []){0x00,0x0C},2,10},
  {0xE8,(uint8_t []){0x00,0x00},2,0},
  {0xFF,(uint8_t []){0x77,0x01,0x00,0x00,0x00},5,0},
  {0x3A,(uint8_t []){0x55},1,0},
  {0x36,(uint8_t []){0x00},1,0},
  {0x35,(uint8_t []){0x00},1,0},
  {0x29,(uint8_t []){0x00},0,20},
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
    const spi_bus_config_t buscfg = {
        .mosi_io_num = DISPLAY_MOSI_PIN,
        .miso_io_num = DISPLAY_MISO_PIN,
        .sclk_io_num = DISPLAY_CLK_PIN,
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

    spi_line_config_t line_config = {
        .cs_io_type = IO_TYPE_GPIO,             // Set to `IO_TYPE_GPIO` if using GPIO, same to below
        .cs_gpio_num = EXAMPLE_LCD_IO_SPI_CS,
        .scl_io_type = IO_TYPE_GPIO,
        .scl_gpio_num = EXAMPLE_LCD_IO_SPI_SCK,
        .sda_io_type = IO_TYPE_GPIO,
        .sda_gpio_num = EXAMPLE_LCD_IO_SPI_SDO,
        .io_expander = NULL,                        // Set to NULL if not using IO expander
    };
    esp_lcd_panel_io_3wire_spi_config_t io_config = ST7701_PANEL_IO_3WIRE_SPI_CONFIG(line_config, 0);
    esp_lcd_panel_io_handle_t io_handle = NULL;
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_3wire_spi(&io_config, &io_handle));

    ESP_LOGI(TAG, "Install ST7701 panel driver");

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
            .pclk_hz = 18 * 1000 * 1000,
            .h_res = DISPLAY_WIDTH,
            .v_res = DISPLAY_HEIGHT,
            .hsync_back_porch = 30,
            .hsync_front_porch = 30, // 30
            .hsync_pulse_width = 6,
            .vsync_back_porch = 20,  // 10-100 40
            .vsync_front_porch = 20, // 10-100 70
            .vsync_pulse_width = 40,
        },

    };

    st7701_vendor_config_t vendor_config = {
        .rgb_config = &rgb_config,
        .init_cmds = lcd_init_cmds,      // Uncomment these line if use custom initialization commands
        .init_cmds_size = sizeof(lcd_init_cmds) / sizeof(st7701_lcd_init_cmd_t),
        .flags = 
        {
        .mirror_by_cmd = 0,       // Only work when `enable_io_multiplex` is set to 0
        .enable_io_multiplex = 1, /**
                                    * Set to 1 if panel IO is no longer needed after LCD initialization.
                                    * If the panel IO pins are sharing other pins of the RGB interface to save GPIOs,
                                    * Please set it to 1 to release the pins.
                                    * ENABLED: Allow SD card to use SPI after LCD init (RGB mode)
                                    */
        },
    };

    const esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = EXAMPLE_LCD_IO_RGB_RESET,     // Set to -1 if not use
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,     // Implemented by LCD command `36h`
        .bits_per_pixel = 16,    // Implemented by LCD command `3Ah` (16/18/24)
        .vendor_config = &vendor_config,
    };  

    esp_lcd_panel_handle_t disp_panel = NULL;

    ESP_LOGI(TAG, "Install ST7701S LCD control panel");

    ESP_ERROR_CHECK(esp_lcd_new_panel_st7701(io_handle, &panel_config, &disp_panel));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(disp_panel));
    ESP_ERROR_CHECK(esp_lcd_panel_init(disp_panel));

    disp = rgb_lcd_display(io_handle, disp_panel,
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
}





void board_init(void)
{
    board = (board_t)malloc(sizeof(struct board_handle_t));
    assert(board);

    display_init();

    i2c_init();
    touch_init(); 

    /*        other init      */

    spi_init();
    ESP_ERROR_CHECK(bsp_sdcard_mount(SDCARD_MOUNT_POINT, SDCARD_CS_PIN));

    ESP_ERROR_CHECK(bsp_qmi8658_init(&board->qmi8658_handle, _i2c_bus));
    ESP_ERROR_CHECK(bsp_pcf85063a_init(&board->pcf85063a_handle, _i2c_bus));

    audio_cfg_t audio_config = {
        .input_sample_rate  = AUDIO_INPUT_SAMPLE_RATE,
        .output_sample_rate = AUDIO_OUTPUT_SAMPLE_RATE,
        .spk_bclk           = AUDIO_I2S_GPIO_BCLK,
        .spk_ws             = AUDIO_I2S_GPIO_WS,
        .spk_dout           = AUDIO_I2S_GPIO_DOUT,
        .mic_sck            = AUDIO_I2S_GPIO_BCLK,
        .mic_ws             = AUDIO_I2S_GPIO_WS,
        .mic_din            = AUDIO_I2S_GPIO_DIN,
        .spk_slot_mask      = I2S_STD_SLOT_RIGHT,
        .mic_slot_mask      = I2S_STD_SLOT_RIGHT,
    };
    audio_duplex_init(&board->audio_handle, &audio_config);


    // lvgl_port_lock(0);

    // lv_demo_widgets();
    // lv_demo_benchmark();

    // lvgl_port_unlock();

}