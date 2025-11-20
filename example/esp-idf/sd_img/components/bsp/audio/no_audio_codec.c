#include "no_audio_codec.h"

#include <esp_log.h>
#include <esp_check.h>
#include <esp_heap_caps.h>
#include <freertos/FreeRTOS.h>


#include "bsp_config.h"

#define TAG "audio"

static i2s_chan_handle_t _tx_handle = NULL;
static i2s_chan_handle_t _rx_handle = NULL;


// @brief read&write buffer

void no_audio_codec_simplex(audio_cfg_t *config)
{
    // Create a new channel for speaker
    i2s_chan_config_t chan_cfg = {
        .id = I2S_NUM_0,
        .role = I2S_ROLE_MASTER,
        .dma_desc_num = 6,
        .dma_frame_num = 240,
        .auto_clear_after_cb = true,
        .auto_clear_before_cb = false,
        .allow_pd = false,
        .intr_priority = 0,
    };
    ESP_ERROR_CHECK(i2s_new_channel(&chan_cfg, &_tx_handle, NULL));

    i2s_std_config_t std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(config->input_sample_rate),
        .slot_cfg = {
            .data_bit_width = I2S_DATA_BIT_WIDTH_32BIT,
            .slot_bit_width = AUDIO_DATA_BIT_WIDTH,
            .slot_mode = I2S_SLOT_MODE_MONO,
            .slot_mask = config->spk_slot_mask,
            .ws_width = I2S_DATA_BIT_WIDTH_32BIT,
            .ws_pol = false,
            .bit_shift = false,
            #if SOC_I2S_HW_VERSION_2
            .left_align = false,
            .big_endian = false,
            .bit_order_lsb = false
            #endif

        },
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,
            .bclk = config->spk_bclk,
            .ws = config->spk_ws,
            .dout = config->spk_dout,
            .din = I2S_GPIO_UNUSED,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv = false
            }
        }
    };
    ESP_ERROR_CHECK(i2s_channel_init_std_mode(_tx_handle, &std_cfg));

    // Create a new channel for MIC
    chan_cfg.id = I2S_NUM_1;
    ESP_ERROR_CHECK(i2s_new_channel(&chan_cfg, NULL, &_rx_handle));
    std_cfg.clk_cfg.sample_rate_hz = (uint32_t)config->input_sample_rate;
    std_cfg.slot_cfg.slot_mask = config->mic_slot_mask;
    std_cfg.gpio_cfg.bclk = config->mic_sck;
    std_cfg.gpio_cfg.ws = config->mic_ws;
    std_cfg.gpio_cfg.dout = I2S_GPIO_UNUSED;
    std_cfg.gpio_cfg.din = config->mic_din;
    ESP_ERROR_CHECK(i2s_channel_init_std_mode(_rx_handle, &std_cfg));

    if(_tx_handle != NULL) i2s_channel_enable(_tx_handle);
    if(_rx_handle != NULL) i2s_channel_enable(_rx_handle);

    ESP_LOGI(TAG, "Simplex channels created");
}

void no_audio_codec_duplex(audio_cfg_t *config)
{
    i2s_chan_config_t chan_cfg = {
        .id = I2S_NUM_0,
        .role = I2S_ROLE_MASTER,
        .dma_desc_num = 6,
        .dma_frame_num = 240,
        .auto_clear_after_cb = true,
        .auto_clear_before_cb = false,
        .allow_pd = false,
        .intr_priority = 0,
    };
    ESP_ERROR_CHECK(i2s_new_channel(&chan_cfg, &_tx_handle, &_rx_handle));

    i2s_std_config_t std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(config->output_sample_rate),
        .slot_cfg = {
            .data_bit_width = AUDIO_DATA_BIT_WIDTH,
            .slot_bit_width = AUDIO_DATA_BIT_WIDTH,
            .slot_mode = I2S_SLOT_MODE_MONO,
            .slot_mask = config->spk_slot_mask,
            .ws_width = AUDIO_DATA_BIT_WIDTH,
            .ws_pol = false,
            .bit_shift = false,
            #if SOC_I2S_HW_VERSION_2
            .left_align = false,
            .big_endian = false,
            .bit_order_lsb = false
            #endif

        },
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,
            .bclk = config->spk_bclk,
            .ws = config->spk_ws,
            .dout = config->spk_dout,
            .din = config->mic_din,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv = false
            }
        }
    };
    ESP_ERROR_CHECK(i2s_channel_init_std_mode(_tx_handle, &std_cfg));
    ESP_ERROR_CHECK(i2s_channel_init_std_mode(_rx_handle, &std_cfg));

    if(_tx_handle != NULL) i2s_channel_enable(_tx_handle);
    if(_rx_handle != NULL) i2s_channel_enable(_rx_handle);

    ESP_LOGI(TAG, "Duplex channels created");
}

static int _read(int16_t* dest, int samples)
{
    size_t bytes_read = 0;
    if(samples > AUDIO_SAMPLES) samples = AUDIO_SAMPLES;
    if(i2s_channel_read(_rx_handle, dest, samples * sizeof(int16_t), &bytes_read, portMAX_DELAY) != ESP_OK) {
        ESP_LOGE(TAG, "read error");
        return 0;
    }

    return bytes_read / sizeof(int16_t);
}

static esp_err_t _write(void *src, size_t len, size_t *bytes_written, uint32_t timeout_ms)
{
    return i2s_channel_write(_tx_handle, src, len, bytes_written, timeout_ms);   
}

static esp_err_t _mute( AUDIO_PLAYER_MUTE_SETTING setting)
{
    if(setting == AUDIO_PLAYER_MUTE) {
        int32_t buffer[128] = {0};
        for (int i = 0; i < 100; i++) {
            i2s_channel_write(_tx_handle, buffer, sizeof(buffer), NULL, portMAX_DELAY);
        }
    }
    return ESP_OK;
}

static esp_err_t _play(const char *file_path)
{
    esp_err_t ret = ESP_OK;

    FILE *fp = fopen(file_path, "rb");
    if(fp == NULL) {
        ESP_LOGE(TAG, "Failed to open file for playback");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Playing '%s'", file_path);

    ret = audio_player_play(fp);
    if(ret != ESP_OK) {
        ESP_LOGE(TAG, "audio_player_play failed");
        fclose(fp);
    }
    return ret;
}

static esp_err_t _stop()
{
    return audio_player_stop();
}

static esp_err_t _set_output_format(uint32_t sample_rate, uint32_t bits_per_sample, i2s_slot_mode_t ch)
{
    esp_err_t ret = ESP_OK;

    if(_tx_handle == NULL) {
        return ESP_ERR_INVALID_STATE;
    }

    if(ch < 1 || ch > 2) {
        return ESP_ERR_INVALID_ARG;
    }

    if(bits_per_sample != 16 && bits_per_sample != 24 && bits_per_sample != 32) {
        ESP_LOGE(TAG, "unsupported bit depth %u", (unsigned)bits_per_sample);
        return ESP_ERR_INVALID_ARG;
    }

    if(sample_rate < 8000 || sample_rate > 48000) {
        ESP_LOGE(TAG, "unsupported sample rate %u", (unsigned)sample_rate);
        return ESP_ERR_INVALID_ARG;
    }

    i2s_std_config_t std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(sample_rate),
        .slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG((i2s_data_bit_width_t)bits_per_sample, ch),
    };

    ret |= i2s_channel_disable(_tx_handle);
    ret |= i2s_channel_reconfig_std_clock(_tx_handle, &std_cfg.clk_cfg);
    ret |= i2s_channel_reconfig_std_slot(_tx_handle, &std_cfg.slot_cfg);
    ret |= i2s_channel_enable(_tx_handle);

    return ret;
}

audio_player_state_t _get_player_state()
{
    return audio_player_get_state();
}

void audio_simplex_init(audio_t *handle, audio_cfg_t *config) 
{
    audio_t audio_handle = (audio_t)malloc(sizeof(struct audio_handle_t));
    assert(audio_handle != NULL);

    no_audio_codec_simplex(config);

    audio_player_config_t player_config = {
        .mute_fn    = _mute,
        .write_fn   = _write,
        .clk_set_fn = _set_output_format,
        .priority   = 5,
    };
    audio_player_new(player_config);

    audio_handle->read  = _read;
    audio_handle->write = _write;
    audio_handle->mute  = _mute;
    audio_handle->play  = _play;
    audio_handle->stop  = _stop;
    audio_handle->set_output_format = _set_output_format;

    audio_handle->mute(AUDIO_PLAYER_MUTE);

    *handle = audio_handle;
}


void audio_duplex_init(audio_t *handle, audio_cfg_t *config) 
{
    audio_t audio_handle = (audio_t)malloc(sizeof(struct audio_handle_t));
    assert(audio_handle != NULL);

    no_audio_codec_duplex(config);

    audio_player_config_t player_config = {
        .mute_fn    = _mute,
        .write_fn   = _write,
        .clk_set_fn = _set_output_format,
        .priority   = 5,
    };
    audio_player_new(player_config);

    audio_handle->read  = _read;
    audio_handle->write = _write;
    audio_handle->mute  = _mute;
    audio_handle->play  = _play;
    audio_handle->stop  = _stop;
    audio_handle->set_output_format = _set_output_format;
    audio_handle->get_player_state = _get_player_state;
    

    // audio_handle->mute(AUDIO_PLAYER_MUTE);

    *handle = audio_handle;
}