#include "bsp_qmi8658.h"

#include <esp_check.h>
#include <esp_log.h>
#include <math.h>

static const char *TAG = "bsp_qmi8658";

static bsp_qmi8658_t _g_handle = NULL;

static esp_err_t _set_arg()
{
    esp_err_t ret = ESP_OK;

    ret |= qmi8658_set_accel_range(&_g_handle->dev, QMI8658_ACCEL_RANGE_8G);
    ret |= qmi8658_set_accel_odr(&_g_handle->dev, QMI8658_ACCEL_ODR_1000HZ);
    ret |= qmi8658_set_gyro_range(&_g_handle->dev, QMI8658_GYRO_RANGE_512DPS);
    ret |= qmi8658_set_gyro_odr(&_g_handle->dev, QMI8658_GYRO_ODR_1000HZ);

    qmi8658_set_accel_unit_mps2(&_g_handle->dev, true);
    qmi8658_set_gyro_unit_rads(&_g_handle->dev, true);
    qmi8658_set_display_precision(&_g_handle->dev, 4);

    if(ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure QMI8658 sensor");
        return ret;
    }

    return ret;
}

static esp_err_t _enable_sensors(uint8_t flags)
{
    if (!_g_handle) return ESP_ERR_INVALID_ARG;
    if (xSemaphoreTake(_g_handle->lock, pdMS_TO_TICKS(QMI8658_LOCK_TIMEOUT_MS)) != pdTRUE) {
        return ESP_ERR_TIMEOUT;
    }
    esp_err_t err = qmi8658_enable_sensors(&_g_handle->dev, flags);
    xSemaphoreGive(_g_handle->lock);
    return err;
}

static esp_err_t _read_accel(float *x, float *y, float *z)
{
    if (!_g_handle || !x || !y || !z) return ESP_ERR_INVALID_ARG;
    if (xSemaphoreTake(_g_handle->lock, pdMS_TO_TICKS(QMI8658_LOCK_TIMEOUT_MS)) != pdTRUE) {
        return ESP_ERR_TIMEOUT;
    }
    esp_err_t err = qmi8658_read_accel(&_g_handle->dev, x, y, z);
    xSemaphoreGive(_g_handle->lock);
    return err;
}

static esp_err_t _read_gyro(float *x, float *y, float *z)
{
    if (!x || !y || !z) return ESP_ERR_INVALID_ARG;
    if (xSemaphoreTake(_g_handle->lock, pdMS_TO_TICKS(QMI8658_LOCK_TIMEOUT_MS)) != pdTRUE) {
        return ESP_ERR_TIMEOUT;
    }
    esp_err_t err = qmi8658_read_gyro(&_g_handle->dev, x, y, z);
    xSemaphoreGive(_g_handle->lock);
    return err;
}

static esp_err_t _read_sensor_data(qmi8658_data_t *data)
{
    if (!data) return ESP_ERR_INVALID_ARG;
    if (xSemaphoreTake(_g_handle->lock, pdMS_TO_TICKS(QMI8658_LOCK_TIMEOUT_MS)) != pdTRUE) {
        return ESP_ERR_TIMEOUT;
    }
    esp_err_t err = qmi8658_read_sensor_data(&_g_handle->dev, data);
    xSemaphoreGive(_g_handle->lock);
    return err;
}

static void _delete()
{
    if (!_g_handle) return;
    if (_g_handle->lock) {
        if (xSemaphoreTake(_g_handle->lock, pdMS_TO_TICKS(QMI8658_LOCK_TIMEOUT_MS)) == pdTRUE) {
            xSemaphoreGive(_g_handle->lock);
        }
        vSemaphoreDelete(_g_handle->lock);
        _g_handle->lock = NULL;
    }
    free(_g_handle);
    _g_handle = NULL;
}

esp_err_t bsp_qmi8658_init(bsp_qmi8658_t *handle, i2c_master_bus_handle_t bus_handle)
{
    esp_err_t ret = ESP_OK;

    if(bus_handle == NULL) {
        ESP_LOGE(TAG, "Invalid I2C bus handle");
        return ESP_ERR_INVALID_ARG;
    }

    bsp_qmi8658_t _handle = (bsp_qmi8658_t)malloc(sizeof(struct bsp_qmi8658_handle_t));
    if(!_handle) return ESP_ERR_NO_MEM;

    ret = qmi8658_init(&_handle->dev, bus_handle, QMI8658_ADDRESS_HIGH);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize QMI8658 sensor");
        free(_handle);
        return ret;
    }

    _handle->set_arg            = _set_arg;
    _handle->enable_sensors     = _enable_sensors;
    _handle->read_accel         = _read_accel;
    _handle->read_gyro          = _read_gyro;
    _handle->read_sensor_data   = _read_sensor_data;
    _handle->delete             = _delete;

    _handle->lock = xSemaphoreCreateMutex();
    if (!_handle->lock) {
        ESP_LOGE(TAG, "Failed to create semaphore");
        free(_handle);
        return ESP_ERR_NO_MEM;
    }

    
    /* publish global handle so instance wrappers can access it */
    _g_handle = _handle;
    *(handle) = _handle;

    _handle->set_arg();

    return ESP_OK;
}