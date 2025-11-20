#include "bsp_pcf85063a.h"

#include <esp_log.h>

static const char *TAG = "bsp_pcf85063a";

static bsp_pcf85063a_t _g_handle = NULL;




static esp_err_t _get_time_data(pcf85063a_datetime_t *time_data)
{
    esp_err_t ret = ESP_OK;
    if (!time_data) return ESP_ERR_INVALID_ARG;
    if (xSemaphoreTake(_g_handle->lock, pdMS_TO_TICKS(PCF85063A_LOCK_TIMEOUT_MS)) != pdTRUE) {
        return ESP_ERR_TIMEOUT;
    }
    ret = pcf85063a_get_time_date(&_g_handle->dev, time_data);
    xSemaphoreGive(_g_handle->lock);
    return ret;
}

static esp_err_t _set_time_data(pcf85063a_datetime_t *time_data)
{
    esp_err_t ret = ESP_OK;

    if (!time_data) return ESP_ERR_INVALID_ARG;
    if (xSemaphoreTake(_g_handle->lock, pdMS_TO_TICKS(PCF85063A_LOCK_TIMEOUT_MS)) != pdTRUE) {
        return ESP_ERR_TIMEOUT;
    }
    ret = pcf85063a_set_time_date(&_g_handle->dev, *time_data);
    xSemaphoreGive(_g_handle->lock);
    return ret;
}



esp_err_t bsp_pcf85063a_init(bsp_pcf85063a_t *handle, i2c_master_bus_handle_t bus_handle)
{
    esp_err_t ret = ESP_OK;

    if(bus_handle == NULL) {
        ESP_LOGE(TAG, "Invalid I2C bus handle");
        return ESP_ERR_INVALID_ARG;
    }

    bsp_pcf85063a_t _handle = (bsp_pcf85063a_t)malloc(sizeof(struct bsp_pcf85063a_handle_t));
    if(!_handle) return ESP_ERR_NO_MEM;

    ret = pcf85063a_init(&_handle->dev, bus_handle, PCF85063A_ADDRESS);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize PCF85063A RTC");
        free(_handle);
        return ret;
    }

    _handle->get_time_data     = _get_time_data; // To be implemented
    _handle->set_time_data     = _set_time_data; // To be implemented

    _handle->lock = xSemaphoreCreateMutex();
    if (!_handle->lock) {
        ESP_LOGE(TAG, "Failed to create semaphore");
        free(_handle);
        return ESP_ERR_NO_MEM;
    }

    *handle = _handle;
    _g_handle = _handle;

    ESP_LOGI(TAG, "PCF85063A RTC initialized successfully");

    return ret;
}