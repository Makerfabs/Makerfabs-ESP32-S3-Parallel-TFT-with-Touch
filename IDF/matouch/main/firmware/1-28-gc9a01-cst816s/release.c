#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/ringbuf.h>
#include <freertos/event_groups.h>
#include <math.h>
#include <esp_err.h>
#include <esp_log.h>
#include <string.h>
#include <dirent.h>
#include <esp_check.h>
#include "lvgl.h"
#include "esp_lvgl_port.h"
#include "lv_demos.h"
#include "driver/gpio.h"
#include "driver/jpeg_decode.h"
#include "driver/pulse_cnt.h"
#include "file_iterator.h"

#include "board.h"

static const char *TAG = "RELEASE";


/***************** 旋钮编码器 *****************/

#define EC11_GPIO_A     47
#define EC11_GPIO_B     48
#define EC11_GPIO_KEY   17

static pcnt_unit_handle_t pcnt_unit = NULL;
static lv_indev_t * encoder_indev = NULL;
static lv_group_t * input_group = NULL;
/*******************************************/


/* =============================================================================================================
                                                    旋钮编码器
    ============================================================================================================ */

static void encoder_read_cb(lv_indev_t * indev, lv_indev_data_t * data)
{
    int count = 0;
    pcnt_unit_get_count(pcnt_unit, &count);
    pcnt_unit_clear_count(pcnt_unit);

    data->enc_diff = count / 2; 

    // 读取按键
    if (gpio_get_level(EC11_GPIO_KEY) == 0) {
        data->state = LV_INDEV_STATE_PRESSED;
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}
static void xuanniu_test()
{
    // init pcnt
    pcnt_unit_config_t unit_config = {
        .high_limit = 32000, 
        .low_limit = -32000,
        .flags.accum_count = false,
    };
    ESP_ERROR_CHECK(pcnt_new_unit(&unit_config, &pcnt_unit));

    pcnt_glitch_filter_config_t filter_config = {
        .max_glitch_ns = 1000,
    };
    ESP_ERROR_CHECK(pcnt_unit_set_glitch_filter(pcnt_unit, &filter_config));

    // channel A
    pcnt_chan_config_t chan_a_config = { .edge_gpio_num = EC11_GPIO_A, .level_gpio_num = EC11_GPIO_B };
    pcnt_channel_handle_t pcnt_chan_a = NULL;
    ESP_ERROR_CHECK(pcnt_new_channel(pcnt_unit, &chan_a_config, &pcnt_chan_a));

    // channel B
    pcnt_chan_config_t chan_b_config = { .edge_gpio_num = EC11_GPIO_B, .level_gpio_num = EC11_GPIO_A };
    pcnt_channel_handle_t pcnt_chan_b = NULL;
    ESP_ERROR_CHECK(pcnt_new_channel(pcnt_unit, &chan_b_config, &pcnt_chan_b));

    // 完整的正交解码逻辑 (x4 模式)
    ESP_ERROR_CHECK(pcnt_channel_set_edge_action(pcnt_chan_a, 
                        PCNT_CHANNEL_EDGE_ACTION_DECREASE, PCNT_CHANNEL_EDGE_ACTION_INCREASE));
    ESP_ERROR_CHECK(pcnt_channel_set_level_action(pcnt_chan_a, 
                        PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE));
    ESP_ERROR_CHECK(pcnt_channel_set_edge_action(pcnt_chan_b, 
                        PCNT_CHANNEL_EDGE_ACTION_INCREASE, PCNT_CHANNEL_EDGE_ACTION_DECREASE));
    ESP_ERROR_CHECK(pcnt_channel_set_level_action(pcnt_chan_b, 
                        PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE));

    ESP_ERROR_CHECK(pcnt_unit_enable(pcnt_unit));
    ESP_ERROR_CHECK(pcnt_unit_clear_count(pcnt_unit));
    ESP_ERROR_CHECK(pcnt_unit_start(pcnt_unit));

    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << EC11_GPIO_KEY),
        .pull_down_en = 0,
        .pull_up_en = 1,
    };
    gpio_config(&io_conf);


    lvgl_port_lock(0);
    encoder_indev = lv_indev_create();
    lv_indev_set_type(encoder_indev, LV_INDEV_TYPE_ENCODER);
    lv_indev_set_read_cb(encoder_indev, encoder_read_cb);
    lvgl_port_unlock();

    lvgl_port_lock(0);

    lv_obj_t * scr = lv_scr_act();

    lv_obj_set_style_bg_color(scr, lv_color_black(), 0);

    lv_group_t * input_group = lv_group_create();
    lv_group_set_default(input_group); 
    if (encoder_indev) {
        lv_indev_set_group(encoder_indev, input_group);
    }

    lv_obj_set_layout(scr, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(scr, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(scr, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(scr, 0, 0); 
    lv_obj_set_style_pad_row(scr, 25, 0);

    lv_obj_t * slider1 = lv_slider_create(scr);
    lv_obj_set_width(slider1, 160);       
    lv_obj_set_height(slider1, 12);
    lv_slider_set_range(slider1, 0, 100);
    lv_slider_set_value(slider1, 40, LV_ANIM_OFF);
    
    lv_obj_set_style_bg_color(slider1, lv_palette_main(LV_PALETTE_GREY), LV_PART_MAIN);
    lv_obj_set_style_bg_color(slider1, lv_palette_main(LV_PALETTE_BLUE), LV_PART_INDICATOR);
    
    lv_obj_set_style_border_width(slider1, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(slider1, 3, LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_set_style_border_color(slider1, lv_palette_main(LV_PALETTE_ORANGE), LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_set_style_border_color(slider1, lv_palette_darken(LV_PALETTE_ORANGE, 2), LV_PART_MAIN | LV_STATE_EDITED);
    lv_obj_set_style_pad_all(slider1, 2, LV_PART_MAIN);

    lv_group_add_obj(input_group, slider1);

    lv_obj_t * slider2 = lv_slider_create(scr);
    lv_obj_set_width(slider2, 160);       
    lv_obj_set_height(slider2, 12);       
    lv_slider_set_range(slider2, 0, 100);
    lv_slider_set_value(slider2, 75, LV_ANIM_OFF);

    lv_obj_set_style_bg_color(slider2, lv_palette_darken(LV_PALETTE_GREY, 2), LV_PART_MAIN); 
    lv_obj_set_style_bg_color(slider2, lv_palette_main(LV_PALETTE_GREEN), LV_PART_INDICATOR);

    lv_obj_set_style_border_width(slider2, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(slider2, 3, LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_set_style_border_color(slider2, lv_palette_main(LV_PALETTE_ORANGE), LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_set_style_border_color(slider2, lv_palette_darken(LV_PALETTE_ORANGE, 2), LV_PART_MAIN | LV_STATE_EDITED);
    lv_obj_set_style_pad_all(slider2, 2, LV_PART_MAIN);

    lv_group_add_obj(input_group, slider2);

    lv_group_focus_obj(slider1);

    lvgl_port_unlock();
}


static void release_task(void *arg)
{
    ESP_LOGI(TAG, "Starting Release Task");

    xuanniu_test();

    // extern void template_image_switch_lvgl(const char *path);
    // template_image_switch_lvgl("/sdcard/images");

    // extern void template_lvgl_demos_test();
    // template_lvgl_demos_test();

    // extern void template_pcf8563_test(void);
    // template_pcf8563_test();
  
    vTaskDelete(NULL); 
}

void release_demo(void)
{
    xTaskCreate(release_task,
                "release_task",
                10 * 1024,
                NULL,
                5,
                NULL);
}

