#include "lvgl.h"
#include "esp_lvgl_port.h"

#include <esp_log.h>
#include <time.h>
#include <freertos/task.h>
#include <freertos/FreeRTOS.h>

#include "../../boards/matouch-3-16-s3/board.h"
#include "qmi8658.h"

static const char *TAG = "DEBUG";

#define MIC_SPK_LOOPBACK_TEST_TIME  10000
#define QMI8658_TEST_TIME           10000
#define PCF85063A_TEST_TIME         10000




static void display_color_test()
{
    struct {
        lv_color_t color;
        const char *name;
    } test_colors[] = {
        {lv_color_make(255, 0, 0),     "红色 (Red)"},
        {lv_color_make(0, 255, 0),     "绿色 (Green)"},
        {lv_color_make(0, 0, 255),     "蓝色 (Blue)"},
        {lv_color_make(255, 255, 0),   "黄色 (Yellow)"},
        {lv_color_make(255, 0, 255),   "洋红色 (Magenta)"},
        {lv_color_make(0, 255, 255),   "青色 (Cyan)"},
        {lv_color_make(0, 0, 0),       "黑色 (Black)"},
        {lv_color_make(255, 255, 255), "白色 (White)"},
    };

    int num_colors = sizeof(test_colors) / sizeof(test_colors[0]);

    for (int i = 0; i < num_colors; i++) {

        lvgl_port_lock(0);
        lv_obj_t *scr = lv_scr_act();
        lv_obj_set_style_bg_color(scr, test_colors[i].color, 0);
        lvgl_port_unlock();

        ESP_LOGI(TAG, "显示颜色: %s", test_colors[i].name);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    ESP_LOGI(TAG, "屏幕颜色显示测试完成!");
}


static void qmi8658_test()
{
    ESP_LOGI(TAG, "QMI8658 测试开始!");
    lvgl_port_lock(0);

    // 创建一个label，放在顶部中央
    lv_obj_t *scr = lv_scr_act();
    lv_obj_t *label = lv_label_create(scr);
    lv_label_set_text(label, "QMI8658 DATA");
    lv_obj_set_style_text_font(label, &lv_font_montserrat_26, 0);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 150);

    // accel label
    lv_obj_t *label_accel = lv_label_create(scr);
    lv_label_set_text(label_accel, "Accel: ");
    lv_obj_align(label_accel, LV_ALIGN_TOP_LEFT, 40, 240);

    // accel x, y, z
    lv_obj_t *label_accel_x = lv_label_create(scr);
    lv_label_set_text(label_accel_x, "X: ");
    lv_obj_set_style_text_font(label_accel_x, &lv_font_montserrat_24, 0);
    lv_obj_align(label_accel_x, LV_ALIGN_TOP_LEFT, 80, 260);
    lv_obj_t *label_accel_x_value = lv_label_create(scr);
    lv_label_set_text(label_accel_x_value, "0");
    lv_obj_align(label_accel_x_value, LV_ALIGN_TOP_LEFT, 110, 260);
    lv_obj_set_style_text_font(label_accel_x_value, &lv_font_montserrat_24, 0);

    lv_obj_t *label_accel_y = lv_label_create(scr);
    lv_label_set_text(label_accel_y, "Y: ");
    lv_obj_set_style_text_font(label_accel_y, &lv_font_montserrat_24, 0);
    lv_obj_align(label_accel_y, LV_ALIGN_TOP_LEFT, 80, 300);
    lv_obj_t *label_accel_y_value = lv_label_create(scr);
    lv_label_set_text(label_accel_y_value, "0");
    lv_obj_set_style_text_font(label_accel_y_value, &lv_font_montserrat_24, 0);
    lv_obj_align(label_accel_y_value, LV_ALIGN_TOP_LEFT, 110, 300);

    lv_obj_t *label_accel_z = lv_label_create(scr);
    lv_label_set_text(label_accel_z, "Z: ");
    lv_obj_set_style_text_font(label_accel_z, &lv_font_montserrat_24, 0);
    lv_obj_align(label_accel_z, LV_ALIGN_TOP_LEFT, 80, 340);
    lv_obj_t *label_accel_z_value = lv_label_create(scr);
    lv_label_set_text(label_accel_z_value, "0");
    lv_obj_set_style_text_font(label_accel_z_value, &lv_font_montserrat_24, 0);
    lv_obj_align(label_accel_z_value, LV_ALIGN_TOP_LEFT, 110, 340);

    // gyro label
    lv_obj_t *label_gyro = lv_label_create(scr);
    lv_label_set_text(label_gyro, "Gyro: ");
    lv_obj_align(label_gyro, LV_ALIGN_TOP_LEFT, 40, 380);

    // gyro x, y, z
    lv_obj_t *label_gyro_x = lv_label_create(scr);
    lv_label_set_text(label_gyro_x, "X: ");
    lv_obj_set_style_text_font(label_gyro_x, &lv_font_montserrat_24, 0);
    lv_obj_align(label_gyro_x, LV_ALIGN_TOP_LEFT, 80, 400);
    lv_obj_t *label_gyro_x_value = lv_label_create(scr);
    lv_label_set_text(label_gyro_x_value, "0");
    lv_obj_set_style_text_font(label_gyro_x_value, &lv_font_montserrat_24, 0);
    lv_obj_align(label_gyro_x_value, LV_ALIGN_TOP_LEFT, 110, 400);

    lv_obj_t *label_gyro_y = lv_label_create(scr);
    lv_label_set_text(label_gyro_y, "Y: ");
    lv_obj_set_style_text_font(label_gyro_y, &lv_font_montserrat_24, 0);
    lv_obj_align(label_gyro_y, LV_ALIGN_TOP_LEFT, 80, 440);
    lv_obj_t *label_gyro_y_value = lv_label_create(scr);
    lv_label_set_text(label_gyro_y_value, "0");
    lv_obj_set_style_text_font(label_gyro_y_value, &lv_font_montserrat_24, 0);
    lv_obj_align(label_gyro_y_value, LV_ALIGN_TOP_LEFT, 110, 440);

    lv_obj_t *label_gyro_z = lv_label_create(scr);
    lv_label_set_text(label_gyro_z, "Z: ");
    lv_obj_set_style_text_font(label_gyro_z, &lv_font_montserrat_24, 0);
    lv_obj_align(label_gyro_z, LV_ALIGN_TOP_LEFT, 80, 480);
    lv_obj_t *label_gyro_z_value = lv_label_create(scr);
    lv_label_set_text(label_gyro_z_value, "0");
    lv_obj_set_style_text_font(label_gyro_z_value, &lv_font_montserrat_24, 0);
    lv_obj_align(label_gyro_z_value, LV_ALIGN_TOP_LEFT, 110, 480);

    lvgl_port_unlock();

    TickType_t start_time = xTaskGetTickCount();

    qmi8658_data_t data;
    while(xTaskGetTickCount() - start_time < pdMS_TO_TICKS(QMI8658_TEST_TIME)) {
        bool ready;
        esp_err_t ret = qmi8658_is_data_ready(&board->qmi8658_handle->dev, &ready);
        if (ret == ESP_OK && ready) {
            ret = qmi8658_read_sensor_data(&board->qmi8658_handle->dev, &data);
            if (ret == ESP_OK) {

                lvgl_port_lock(0);
                lv_label_set_text_fmt(label_accel_x_value, "%.4f", data.accelX);
                lv_label_set_text_fmt(label_accel_y_value, "%.4f", data.accelY);
                lv_label_set_text_fmt(label_accel_z_value, "%.4f", data.accelZ);
                lv_label_set_text_fmt(label_gyro_x_value, "%.4f", data.gyroX);
                lv_label_set_text_fmt(label_gyro_y_value, "%.4f", data.gyroY);
                lv_label_set_text_fmt(label_gyro_z_value, "%.4f", data.gyroZ);
                lvgl_port_unlock();
            } else {
                ESP_LOGE(TAG, "Failed to read sensor data (error: %d)", ret);
            }
        } else {
            ESP_LOGE(TAG, "Data not ready or error reading status (error: %d)", ret);
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

static void pcf85063a_test()
{
    ESP_LOGI(TAG, "PCF85063A 测试开始!");

    lvgl_port_lock(0);

    // 创建一个label，放在顶部中央
    lv_obj_t *scr = lv_scr_act();
    lv_obj_clean(scr);
    lv_obj_t *label = lv_label_create(scr);
    lv_label_set_text(label, "PCF85063A DATA");
    lv_obj_set_style_text_font(label, &lv_font_montserrat_26, 0);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 150);

    // 创建年月日时分秒标签
    lv_obj_t *label_time_year = lv_label_create(scr);
    lv_label_set_text(label_time_year, "Year: ");
    lv_obj_set_style_text_font(label_time_year, &lv_font_montserrat_24, 0);
    lv_obj_align(label_time_year, LV_ALIGN_TOP_LEFT, 40, 240);

    lv_obj_t *label_time_month = lv_label_create(scr);
    lv_label_set_text(label_time_month, "Month: ");
    lv_obj_set_style_text_font(label_time_month, &lv_font_montserrat_24, 0);
    lv_obj_align(label_time_month, LV_ALIGN_TOP_LEFT, 40, 280);

    lv_obj_t *label_time_day = lv_label_create(scr);
    lv_label_set_text(label_time_day, "Day: ");
    lv_obj_set_style_text_font(label_time_day, &lv_font_montserrat_24, 0);
    lv_obj_align(label_time_day, LV_ALIGN_TOP_LEFT, 40, 320);

    lv_obj_t *label_time_hour = lv_label_create(scr);
    lv_label_set_text(label_time_hour, "Hour: ");
    lv_obj_set_style_text_font(label_time_hour, &lv_font_montserrat_24, 0);
    lv_obj_align(label_time_hour, LV_ALIGN_TOP_LEFT, 40, 360);

    lv_obj_t *label_time_minute = lv_label_create(scr);
    lv_label_set_text(label_time_minute, "Minute: ");
    lv_obj_set_style_text_font(label_time_minute, &lv_font_montserrat_24, 0);
    lv_obj_align(label_time_minute, LV_ALIGN_TOP_LEFT, 40, 400);

    lv_obj_t *label_time_second = lv_label_create(scr);
    lv_label_set_text(label_time_second, "Second: ");
    lv_obj_set_style_text_font(label_time_second, &lv_font_montserrat_24, 0);
    lv_obj_align(label_time_second, LV_ALIGN_TOP_LEFT, 40, 440);

    lvgl_port_unlock();

    TickType_t start_time = xTaskGetTickCount();
    pcf85063a_datetime_t datetime;

    time_t now;
    struct tm timeinfo;

    time(&now);
    localtime_r(&now, &timeinfo);
    datetime.year = timeinfo.tm_year + 1900;
    datetime.month = timeinfo.tm_mon + 1;
    datetime.day = timeinfo.tm_mday;
    datetime.hour = timeinfo.tm_hour;
    datetime.min = timeinfo.tm_min;
    datetime.sec = timeinfo.tm_sec;
    ESP_ERROR_CHECK(board->pcf85063a_handle->set_time_data(&datetime));

    while(xTaskGetTickCount() - start_time < pdMS_TO_TICKS(PCF85063A_TEST_TIME)) {
        esp_err_t ret = board->pcf85063a_handle->get_time_data(&datetime);
        if (ret == ESP_OK) {
            lvgl_port_lock(0);
            lv_label_set_text_fmt(label_time_year, "Year: %04d", datetime.year);
            lv_label_set_text_fmt(label_time_month, "Month: %02d", datetime.month);
            lv_label_set_text_fmt(label_time_day, "Day: %02d", datetime.day);
            lv_label_set_text_fmt(label_time_hour, "Hour: %02d", datetime.hour);
            lv_label_set_text_fmt(label_time_minute, "Minute: %02d", datetime.min);
            lv_label_set_text_fmt(label_time_second, "Second: %02d", datetime.sec);
            lvgl_port_unlock();
        } else {
            ESP_LOGE(TAG, "Failed to get datetime data (error: %d)", ret);
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

static void mic_spk_loopback_test()
{
    lvgl_port_lock(0);

    // 创建一个label，放在中央
    lv_obj_t *scr = lv_scr_act();
    lv_obj_clean(scr);
    lv_obj_t *label = lv_label_create(scr);
    lv_label_set_text(label, "MIC -> SPK Loopback Test");
    lv_obj_set_style_text_font(label, &lv_font_montserrat_24, 0);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

    lvgl_port_unlock();

    TickType_t start_time = xTaskGetTickCount();

    int16_t buffer[1024];
    while(xTaskGetTickCount() - start_time < pdMS_TO_TICKS(MIC_SPK_LOOPBACK_TEST_TIME)) {
        int samples = board->audio_handle->read(buffer, 1024);
        if(samples) board->audio_handle->write(buffer, samples * sizeof(int16_t), NULL, portMAX_DELAY);

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}



static void play_wav_from_sdcard()
{
    lvgl_port_lock(0);

    // 创建一个label，放在中央
    lv_obj_t *scr = lv_scr_act();
    lv_obj_clean(scr);
    lv_obj_t *label = lv_label_create(scr);
    lv_label_set_text(label, "Playing WAV from SD Card");
    lv_obj_set_style_text_font(label, &lv_font_montserrat_24, 0);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

    lvgl_port_unlock();

    board->audio_handle->play("/sdcard/cn.wav");
}


static void debug_task(void *arg)
{
    display_color_test();
    qmi8658_test();
    pcf85063a_test();
    mic_spk_loopback_test();
    play_wav_from_sdcard();
    vTaskDelete(NULL);
}

void debug_demo()
{
    xTaskCreate(debug_task,
                "test_task",
                1024 * 10,
                NULL,
                5,
                NULL);
    
}