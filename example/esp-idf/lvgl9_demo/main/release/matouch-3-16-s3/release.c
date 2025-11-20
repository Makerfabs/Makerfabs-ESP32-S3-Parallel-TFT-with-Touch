#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/ringbuf.h>
#include <freertos/event_groups.h>
#include <math.h>
#include <esp_err.h>
#include <esp_log.h>
#include "lvgl.h"
#include "esp_lvgl_port.h"

#include "../../boards/matouch-3-16-s3/board.h"

static const char *TAG = "RELEASE";

extern const lv_image_dsc_t img1;
extern const lv_image_dsc_t img2;
extern const lv_image_dsc_t img3;
extern const lv_image_dsc_t img4;
extern const lv_image_dsc_t img5;


#define AUDIO_EVENT_RECORDING           (1 << 0)
#define AUDIO_EVENT_SAVING              (1 << 1)
#define AUDIO_EVENT_PLAYING             (1 << 2)
#define AUDIO_EVENT_PLAYING_STOP        (1 << 3)

#define ONE_G                           9.80665f    // 重力加速度 (m/s^2)
#define ACCEL_TOLERANCE_G               2.7f        // 容忍度（公差），例如 4.5 m/s^2

#define QMI8658_ORIENTATION_DURATION_MS 1000

typedef enum {
    ORIENTATION_FLAT_Z,      // 平放（Z轴垂直于地面）
    ORIENTATION_UPRIGHT_Y,   // 竖立（Y轴垂直于地面）
    ORIENTATION_SIDELAY_X,   // 侧放（X轴垂直于地面）
    ORIENTATION_UNKNOWN      // 倾斜或运动中
} qmi8658_orientation_e;

typedef struct {
    qmi8658_orientation_e current_stable_orientation; // 当前持续稳定的姿态
    qmi8658_orientation_e last_stable_orientation;  // 上一次检测到的姿态
    TickType_t stable_start_tick;                    // 姿态开始稳定的时间点
    TickType_t required_ticks;                       // 保持姿态所需的时钟周期数
} qmi8658_orientation_info_t;

static qmi8658_orientation_info_t _g_orientation_info;
static qmi8658_orientation_e _g_orientation = ORIENTATION_UNKNOWN;
static EventGroupHandle_t _g_audio_event_group = NULL;
static RingbufHandle_t _g_audio_ringbuffer = NULL;

static lv_obj_t *state_label = NULL;


static void lv_update_state()
{
    if(_g_orientation_info.current_stable_orientation != _g_orientation_info.last_stable_orientation) {
        char text[64] = {0};
        switch(_g_orientation_info.current_stable_orientation) {
            case ORIENTATION_UPRIGHT_Y:
                snprintf(text, sizeof(text), "Recording...");
                break;
            case ORIENTATION_SIDELAY_X:
                snprintf(text, sizeof(text), "Playing...");
                break;
            default:
                snprintf(text, sizeof(text), "Idle...");
                break;
        }
        lvgl_port_lock(0);
        lv_label_set_text(state_label, text);
        lvgl_port_unlock();
    }
}
/*
 * @brief  for record example
 */
static void _detect_orientation(float ax, float ay, float az)
{
    // 我们只关注竖立（Y 轴）和侧放（X 轴）。其他情况归为 UNKNOWN。
    qmi8658_orientation_e raw_orientation = ORIENTATION_UNKNOWN;
    if (fabs(ay) > (ONE_G - ACCEL_TOLERANCE_G) && fabs(ax) <= ACCEL_TOLERANCE_G && fabs(az) <= ACCEL_TOLERANCE_G) {
        raw_orientation = ORIENTATION_UPRIGHT_Y;
    } else if (fabs(ax) > (ONE_G - ACCEL_TOLERANCE_G) && fabs(ay) <= ACCEL_TOLERANCE_G && fabs(az) <= ACCEL_TOLERANCE_G) {
        raw_orientation = ORIENTATION_SIDELAY_X;
    } else {
        raw_orientation = ORIENTATION_UNKNOWN;
    }

    TickType_t now = xTaskGetTickCount();

    // 如果检测到的状态与上次不同，重置稳定计时器
    if (_g_orientation != raw_orientation) {
        _g_orientation = raw_orientation;
        _g_orientation_info.stable_start_tick = now;
        // 不立即改变 current_stable_orientation，除非新状态稳定足够长时间
    } else {
        // 同样的检测结果持续出现，检查是否达到稳定时长
        if (raw_orientation == ORIENTATION_UPRIGHT_Y || raw_orientation == ORIENTATION_SIDELAY_X) {
            if ((now - _g_orientation_info.stable_start_tick) >= _g_orientation_info.required_ticks) {
                // 只有在稳定时间满足后，才将当前稳定姿态设置为检测到的姿态
                _g_orientation_info.last_stable_orientation = _g_orientation_info.current_stable_orientation;
                _g_orientation_info.current_stable_orientation = raw_orientation;
            }
        }
    }
}


static void player_task(void *arg)
{
    while(true) {
        EventBits_t bits = xEventGroupWaitBits(_g_audio_event_group, 
                            AUDIO_EVENT_PLAYING | AUDIO_EVENT_PLAYING_STOP,
                            pdFALSE,
                            pdFALSE,
                            portMAX_DELAY);

        if(bits & AUDIO_EVENT_PLAYING) {
            xEventGroupClearBits(_g_audio_event_group, AUDIO_EVENT_PLAYING);
            board->audio_handle->play("/sdcard/record.wav");
            ESP_LOGI(TAG, "Playing recorded audio...");
        }

        if(bits & AUDIO_EVENT_PLAYING_STOP) {
            xEventGroupClearBits(_g_audio_event_group, AUDIO_EVENT_PLAYING_STOP);
            if(board->audio_handle->get_player_state() == AUDIO_PLAYER_STATE_PLAYING) {
                ESP_LOGI(TAG, "Stopping playing recorded audio...");
                board->audio_handle->stop();
            }
        }

    }
}

static void sd_card_writer_task(void *arg) 
{  
    ESP_LOGI(TAG, "SD Card Writer started.");

    size_t bytes_written;
    size_t total_bytes_written = 0;

    while (true) {
        xEventGroupWaitBits(_g_audio_event_group, AUDIO_EVENT_SAVING, pdTRUE, pdFALSE, portMAX_DELAY);

        ESP_LOGI(TAG, "Saving audio to SD card...");

        FILE *f = fopen("/sdcard/record.wav", "wb");
        if (f == NULL) {
            ESP_LOGE(TAG, "Failed to open record.wav for writing!");
            vTaskDelete(NULL);
        }

        bsp_wav_header_t wav_header;
        fwrite(&wav_header, sizeof(wav_header), 1, f);

        while(true) {
            int16_t *data = (int16_t *)xRingbufferReceive(_g_audio_ringbuffer, &bytes_written, pdMS_TO_TICKS(500));
            if(data != NULL) {
                // ESP_LOGI(TAG, "Writing %d bytes to SD card", bytes_written);
                bytes_written = fwrite(data, 1, bytes_written, f);
                if(bytes_written) total_bytes_written += bytes_written;
                vRingbufferReturnItem(_g_audio_ringbuffer, (void *)data);
            } else {
                if(_g_orientation_info.current_stable_orientation != ORIENTATION_UPRIGHT_Y) {
                    wav_header = (bsp_wav_header_t)WAV_HEADER_PCM_DEFAULT(total_bytes_written, AUDIO_DATA_BIT_WIDTH, 16000, AUDIO_SLOT_MODE);
                    if(f) fseek(f, 0, SEEK_SET), fwrite(&wav_header, sizeof(wav_header), 1, f), fclose(f);
                    ESP_LOGI(TAG, "Finished saving audio to SD card, total bytes=%d", total_bytes_written);
                    total_bytes_written = 0;
                    // bsp_extra_player_play_file("/sdcard/record.wav");
                    xEventGroupSetBits(_g_audio_event_group, AUDIO_EVENT_PLAYING);
                    break;
                }
            }

            vTaskDelay(pdMS_TO_TICKS(30));
        }
        xEventGroupClearBits(_g_audio_event_group, AUDIO_EVENT_SAVING);
        f = NULL;
        ESP_LOGI(TAG, "SD Card Writer stopped.");
    }
    vTaskDelete(NULL);
}



static void recorder_task(void *arg) 
{
    int16_t buffer[AUDIO_SAMPLES];
    while(true) {
        xEventGroupWaitBits(_g_audio_event_group, AUDIO_EVENT_RECORDING, pdTRUE, pdFALSE, portMAX_DELAY);

        ESP_LOGI(TAG, "I2S Recorder started.");

        while(_g_orientation_info.current_stable_orientation == ORIENTATION_UPRIGHT_Y) {
            int samples = board->audio_handle->read(buffer, AUDIO_SAMPLES);
            if(samples)  xRingbufferSend(_g_audio_ringbuffer, (void *)buffer, samples * sizeof(int16_t), pdMS_TO_TICKS(500));
            else ESP_LOGI(TAG, "No audio samples read.");

            vTaskDelay(pdMS_TO_TICKS(50));
        }
        xEventGroupClearBits(_g_audio_event_group, AUDIO_EVENT_RECORDING);
        ESP_LOGI(TAG, "I2S Recorder stopped.");
    }
    vTaskDelete(NULL);
}

static void imu_mic_spk_sd_test(void *arg)
{
    lvgl_port_lock(0);
    state_label = lv_label_create(lv_scr_act());
    lv_obj_set_align(state_label, LV_ALIGN_CENTER);
    lv_obj_set_width(state_label, LV_HOR_RES - 40);
    lv_obj_set_style_text_align(state_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_font(state_label, &lv_font_montserrat_24, 0);
    // lv_label_set_long_mode(state_label, LV_LABEL_LONG_SCROLL);
    lv_label_set_text(state_label, "Idle...");
    lvgl_port_unlock();
    
    _g_audio_ringbuffer = xRingbufferCreate(21 * 1024, RINGBUF_TYPE_NOSPLIT);
    assert(_g_audio_ringbuffer != NULL);

    _g_audio_event_group = xEventGroupCreate();
    assert(_g_audio_event_group != NULL);

    xTaskCreate(recorder_task, "i2s_task", 1024 * 4, NULL, tskIDLE_PRIORITY + 2, NULL);
    xTaskCreate(sd_card_writer_task, "sd_task", 1024 * 4, NULL, tskIDLE_PRIORITY + 3, NULL);
    xTaskCreate(player_task, "play_task", 1024 * 4, NULL, tskIDLE_PRIORITY + 1, NULL);

    while (true) {
        float ax, ay, az;
        if(board->qmi8658_handle->read_accel(&ax, &ay, &az) == ESP_OK) {
            _detect_orientation(ax, ay, az);
            ESP_LOGI(TAG, "cur Orientation: %d, cur stabel state = %d", _g_orientation, _g_orientation_info.current_stable_orientation);
        }

        switch (_g_orientation_info.current_stable_orientation) {
            case ORIENTATION_SIDELAY_X:
                break;
            case ORIENTATION_UPRIGHT_Y:
                xEventGroupSetBits(_g_audio_event_group, AUDIO_EVENT_RECORDING | AUDIO_EVENT_SAVING | AUDIO_EVENT_PLAYING_STOP);
                break;
            default:
                break;
        }
        lv_update_state();
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    vTaskDelete(NULL);
    return;
}

static void imu_switch_jpg_task(void)
{
    float change_rate = 2.5f;
    /* baseline 用于低通滤波噪声，避免因瞬时方向相消导致误判 */
    float baseline = 0.0f;
    /* 切换后的最小冷却时间，避免一次挥动来回导致两次切换（ms） */
    const TickType_t SWITCH_COOLDOWN_MS = 600;
    TickType_t last_switch_tick = 0;

    const lv_image_dsc_t *imgs[] = { &img1, &img2, &img3, &img4, &img5 };
    const size_t img_cnt = sizeof(imgs) / sizeof(imgs[0]);
    size_t idx = 0;

    lvgl_port_lock(0);
    lv_obj_t *scr = lv_scr_act();
    lv_obj_clean(scr);

    lv_obj_t *img_obj = lv_image_create(scr);
    lv_image_set_src(img_obj, imgs[idx]);
    lvgl_port_unlock();

    while(true) {
        float ax, ay, az;
        float gx, gy, gz;
        if(board->qmi8658_handle->read_accel(&ax, &ay, &az) == ESP_OK &&
           board->qmi8658_handle->read_gyro(&gx, &gy, &gz) == ESP_OK) {
            /* 用绝对值之和作为 motion magnitude，避免分量相互抵消 */
            float mag = fabs(ax) + fabs(ay) + fabs(az) + fabs(gx) + fabs(gy) + fabs(gz);

            /* 简单一阶低通求 baseline（可调） */
            const float alpha = 0.12f; /* 低通系数，越小响应越慢 */
            baseline = (1.0f - alpha) * baseline + alpha * mag;

            TickType_t now = xTaskGetTickCount();

            /* 只有当 mag 明显高于 baseline 且超过冷却时间时才切换 */
            if ((mag - baseline) > change_rate && (now - last_switch_tick) >= pdMS_TO_TICKS(SWITCH_COOLDOWN_MS)) {
                idx = (idx + 1) % img_cnt;
                lvgl_port_lock(0);
                lv_image_set_src(img_obj, imgs[idx]);
                lvgl_port_unlock();
                last_switch_tick = now;
                ESP_LOGI(TAG, "Significant motion detected, switched image index=%d.", idx);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}








static void release_task(void *arg)
{
    // imu_switch_jpg_task();
    imu_mic_spk_sd_test(NULL);
    vTaskDelete(NULL); 
}


void release_demo(void)
{
    xTaskCreate(release_task,
                "release_task",
                1024 * 10,
                NULL,
                5,
                NULL);
}