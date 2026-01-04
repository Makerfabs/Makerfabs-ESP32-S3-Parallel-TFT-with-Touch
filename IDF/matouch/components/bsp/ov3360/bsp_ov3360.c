#include "bsp_ov3360.h"

#include "esp_camera.h"


static esp_err_t _init(bsp_ov3660_config_t *config)
{
    camera_config_t camera_config = {
        .pin_pwdn       = config->pin_pwdn,
        .pin_reset      = config->pin_reset,
        .pin_xclk       = config->pin_xclk,
        .pin_sccb_scl   = config->pin_sccb_scl,
        .pin_sccb_sda   = config->pin_sccb_sda,

        .pin_d0         = config->pin_d0,
        .pin_d1         = config->pin_d1,
        .pin_d2         = config->pin_d2,
        .pin_d3         = config->pin_d3,
        .pin_d4         = config->pin_d4,
        .pin_d5         = config->pin_d5,
        .pin_d6         = config->pin_d6,
        .pin_d7         = config->pin_d7,
        .pin_vsync      = config->pin_vsync,
        .pin_href       = config->pin_href,
        .pin_pclk       = config->pin_pclk,

        .xclk_freq_hz   = 20000000,
        .ledc_timer     = LEDC_TIMER_0,
        .ledc_channel   = LEDC_CHANNEL_0,

        .pixel_format   = PIXFORMAT_RGB565, //YUV422,GRAYSCALE,RGB565,JPEG
        .frame_size     = FRAMESIZE_QVGA,    //QQVGA-UXGA, For ESP32, do not use sizes above QVGA when not JPEG. The performance of the ESP32-S series has improved a lot, but JPEG mode always gives better frame rates.

        .jpeg_quality   = 12, //0-63, for OV series camera sensors, lower number means higher quality
        .fb_count       = 1,       //When jpeg mode is used, if fb_count more than one, the driver will work in continuous mode.
        .fb_location    = CAMERA_FB_IN_PSRAM,
        .grab_mode      = CAMERA_GRAB_WHEN_EMPTY,
    };

    return esp_camera_init(&camera_config);
}
