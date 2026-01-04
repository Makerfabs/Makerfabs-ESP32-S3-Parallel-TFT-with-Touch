#include "bsp_pcf8563.h"
#include <esp_log.h>
#include <string.h>

static const char *TAG = "bsp_pcf8563";

#define PCF8563_REG_VL_SECONDS 0x02
#define PCF8563_CMD_START      0x02
#define PCF8563_I2C_ADDR       0x51
#define PCF8563_I2C_FREQ_HZ    100000

// 私有数据结构（封装实现细节）
typedef struct {
    i2c_master_dev_handle_t i2c_dev;
    SemaphoreHandle_t lock;
} bsp_pcf8563_priv_t;

// ============================================================================
// 辅助函数
// ============================================================================

static inline uint8_t bcd2dec(uint8_t val) {
    return ((val >> 4) * 10) + (val & 0x0f);
}

static inline uint8_t dec2bcd(uint8_t val) {
    return ((val / 10) << 4) + (val % 10);
}

// ============================================================================
// 私有方法实现
// ============================================================================

static esp_err_t _get_time_impl(bsp_pcf8563_handle_t self, struct tm *timeinfo) 
{
    if (!self || !self->_priv || !timeinfo) {
        return ESP_ERR_INVALID_ARG;
    }
    
    bsp_pcf8563_priv_t *priv = (bsp_pcf8563_priv_t *)self->_priv;
    
    if (xSemaphoreTake(priv->lock, pdMS_TO_TICKS(PCF8563_LOCK_TIMEOUT_MS)) != pdTRUE) {
        return ESP_ERR_TIMEOUT;
    }
    
    uint8_t reg_addr = PCF8563_CMD_START;
    uint8_t data[7];
    
    esp_err_t ret = i2c_master_transmit_receive(
        priv->i2c_dev, 
        &reg_addr, 1, 
        data, 7, 
        -1
    );
    
    if (ret == ESP_OK) {
        // 解析数据（屏蔽无效位）
        timeinfo->tm_sec  = bcd2dec(data[0] & 0x7F);
        timeinfo->tm_min  = bcd2dec(data[1] & 0x7F);
        timeinfo->tm_hour = bcd2dec(data[2] & 0x3F);
        timeinfo->tm_mday = bcd2dec(data[3] & 0x3F);
        timeinfo->tm_wday = data[4] & 0x07;
        timeinfo->tm_mon  = bcd2dec(data[5] & 0x1F) - 1;
        
        // 年份：假设 2000 年后
        int year = bcd2dec(data[6]);
        timeinfo->tm_year = year + 100; // 2000-2099
        
        // 设置其他字段
        timeinfo->tm_isdst = -1; // 未知夏令时
    }
    
    xSemaphoreGive(priv->lock);
    return ret;
}

static esp_err_t _set_time_impl(bsp_pcf8563_handle_t self, const struct tm *timeinfo) 
{
    if (!self || !self->_priv || !timeinfo) {
        return ESP_ERR_INVALID_ARG;
    }
    
    bsp_pcf8563_priv_t *priv = (bsp_pcf8563_priv_t *)self->_priv;
    
    if (xSemaphoreTake(priv->lock, pdMS_TO_TICKS(PCF8563_LOCK_TIMEOUT_MS)) != pdTRUE) {
        return ESP_ERR_TIMEOUT;
    }
    
    uint8_t data[8];
    data[0] = PCF8563_CMD_START;
    data[1] = dec2bcd(timeinfo->tm_sec);
    data[2] = dec2bcd(timeinfo->tm_min);
    data[3] = dec2bcd(timeinfo->tm_hour);
    data[4] = dec2bcd(timeinfo->tm_mday);
    data[5] = timeinfo->tm_wday;
    data[6] = dec2bcd(timeinfo->tm_mon + 1);
    
    // 年份处理
    int year = timeinfo->tm_year - 100;
    if (year < 0) year = 0;
    if (year > 99) year = 99;
    data[7] = dec2bcd(year);
    
    esp_err_t ret = i2c_master_transmit(priv->i2c_dev, data, 8, -1);
    
    xSemaphoreGive(priv->lock);
    return ret;
}

static esp_err_t _destroy_impl(bsp_pcf8563_handle_t self)
{
    if (!self) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (self->_priv) {
        bsp_pcf8563_priv_t *priv = (bsp_pcf8563_priv_t *)self->_priv;
        
        // 释放锁
        if (priv->lock) {
            vSemaphoreDelete(priv->lock);
        }
        
        // 释放 I2C 设备（如果需要的话）
        // i2c_master_bus_rm_device(priv->i2c_dev);
        
        free(priv);
    }
    
    free(self);
    return ESP_OK;
}

// ============================================================================
// 公共接口实现
// ============================================================================

esp_err_t bsp_pcf8563_init(bsp_pcf8563_handle_t *handle, i2c_master_bus_handle_t bus_handle)
{
    if (!handle || !bus_handle) {
        ESP_LOGE(TAG, "Invalid arguments");
        return ESP_ERR_INVALID_ARG;
    }
    
    // 分配公共对象
    bsp_pcf8563_handle_t dev = (bsp_pcf8563_handle_t)calloc(1, sizeof(struct bsp_pcf8563_t));
    if (!dev) {
        ESP_LOGE(TAG, "Failed to allocate memory for handle");
        return ESP_ERR_NO_MEM;
    }
    
    // 分配私有数据
    bsp_pcf8563_priv_t *priv = (bsp_pcf8563_priv_t *)calloc(1, sizeof(bsp_pcf8563_priv_t));
    if (!priv) {
        ESP_LOGE(TAG, "Failed to allocate memory for private data");
        free(dev);
        return ESP_ERR_NO_MEM;
    }
    
    // 创建互斥锁
    priv->lock = xSemaphoreCreateMutex();
    if (!priv->lock) {
        ESP_LOGE(TAG, "Failed to create mutex");
        free(priv);
        free(dev);
        return ESP_ERR_NO_MEM;
    }
    
    // 配置 I2C 设备
    i2c_device_config_t dev_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = PCF8563_I2C_ADDR,
        .scl_speed_hz = PCF8563_I2C_FREQ_HZ,
    };
    
    esp_err_t ret = i2c_master_bus_add_device(bus_handle, &dev_config, &priv->i2c_dev);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to add I2C device: %s", esp_err_to_name(ret));
        vSemaphoreDelete(priv->lock);
        free(priv);
        free(dev);
        return ret;
    }
    
    // 绑定方法和私有数据
    dev->_priv = priv;
    dev->get_time = _get_time_impl;
    dev->set_time = _set_time_impl;
    dev->destroy = _destroy_impl;
    
    *handle = dev;
    
    ESP_LOGI(TAG, "PCF8563 RTC initialized successfully");
    return ESP_OK;
}

esp_err_t bsp_pcf8563_destroy(bsp_pcf8563_handle_t handle)
{
    if (!handle || !handle->destroy) {
        return ESP_ERR_INVALID_ARG;
    }
    
    return handle->destroy(handle);
}