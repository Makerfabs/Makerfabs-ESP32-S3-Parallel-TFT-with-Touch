#pragma once

#include <driver/gpio.h>
#include <driver/i2s_std.h>
#include <esp_err.h>

#include "audio_player.h"


typedef struct audio_handle_t* audio_t;

struct audio_handle_t {
    int (*read)(int16_t* dest, int samples);
    esp_err_t (*write)(void* src, size_t len, size_t *bytes_written, uint32_t timeout_ms);
    esp_err_t (*mute)(AUDIO_PLAYER_MUTE_SETTING mute_setting);
    esp_err_t (*play)(const char *file_path);
    esp_err_t (*stop)();
    esp_err_t (*set_output_format)(uint32_t sample_rate, uint32_t bits_per_sample, i2s_slot_mode_t ch);
    audio_player_state_t (*get_player_state)();
};

typedef struct  {
    int input_sample_rate;
    int output_sample_rate;

    gpio_num_t spk_bclk;
    gpio_num_t spk_ws;
    gpio_num_t spk_dout;

    gpio_num_t mic_sck;
    gpio_num_t mic_ws;
    gpio_num_t mic_din;

    i2s_std_slot_mask_t spk_slot_mask;
    i2s_std_slot_mask_t mic_slot_mask;

}audio_cfg_t;



#define AUDIO_MIC_I2S_GPIO_CFG(_blck, _ws, _din)        \
    {                                                   \
        .mclk = I2S_GPIO_UNUSED,                        \
        .bclk = _blck,                                  \
        .ws = _ws,                                      \
        .dout = I2S_GPIO_UNUSED,                        \
        .din = _din,                                    \
        .invert_flags = {                               \
            .mclk_inv = false,                          \
            .bclk_inv = false,                          \
            .ws_inv = false,                            \
        },                                              \
    }

#define AUDIO_SPK_I2S_GPIO_CFG(_bclk, _ws, _dout)       \
    {                                                   \
        .mclk = I2S_GPIO_UNUSED,                        \
        .bclk = _bclk,                                  \
        .ws = _ws,                                      \
        .dout = _dout,                                  \
        .din = I2S_GPIO_UNUSED,                         \
        .invert_flags = {                               \
            .mclk_inv = false,                          \
            .bclk_inv = false,                          \
            .ws_inv = false,                            \
        },                                              \
    }


void audio_simplex_init(audio_t *handle, audio_cfg_t *config);
void audio_duplex_init(audio_t *handle, audio_cfg_t *config);