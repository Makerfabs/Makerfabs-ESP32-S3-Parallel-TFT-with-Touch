#include "bsp_sdcard.h"

#include <esp_vfs_fat.h>
#include <sdmmc_cmd.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>


static const char *TAG = "bsp_sdcard";


// @brief
static sdmmc_card_t *_card = NULL;

static char _mount_point[64] = {0};


static esp_err_t _print_sd_files()
{
    DIR *dir = NULL;
    struct dirent *entry;
    struct stat st;

    if (_card == NULL) {
        ESP_LOGE(TAG, "handle is NULL");
        return ESP_ERR_INVALID_ARG;
    }
    
    dir = opendir(_mount_point);
    if (dir == NULL) {
        ESP_LOGE(TAG, "Failed to open mount point '%s'", _mount_point);
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Listing files in %s:", _mount_point);
    while ((entry = readdir(dir)) != NULL) {
        char path[512];
        snprintf(path, sizeof(path), "%s/%s", _mount_point, entry->d_name);
        if (stat(path, &st) == 0) {
            if (S_ISDIR(st.st_mode)) {
                ESP_LOGI(TAG, "  [DIR]  %s", entry->d_name);
            } else {
                ESP_LOGI(TAG, "  [FILE] %s  (%lld bytes)", entry->d_name, (long long)st.st_size);
            }
        } else {
            ESP_LOGW(TAG, "  [?] %s (stat failed)", entry->d_name);
        }
    }

    closedir(dir);
    return ESP_OK;
}

esp_err_t bsp_sdcard_mount(const char *mount_point, gpio_num_t sd_cs)
{
    esp_err_t ret = ESP_OK;

    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = true, /* If mount failed, format the filesystem */
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();

    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = sd_cs;
    slot_config.host_id = host.slot;

    ESP_LOGI(TAG, "Mounting filesystem");
    ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &_card);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount filesystem. "
                     "If you want the card to be formatted, set the CONFIG_EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.");
        } else {
            ESP_LOGE(TAG, "Failed to initialize the card (%s). "
                     "Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(ret));
        }
        return ret;
    }
    ESP_LOGI(TAG, "Filesystem mounted");

    /* 保存挂载点（确保以 '\0' 结尾） */
    if (mount_point) {
        strncpy(_mount_point, mount_point, sizeof(_mount_point) - 1);
        _mount_point[sizeof(_mount_point) - 1] = '\0';
    }

    sdmmc_card_print_info(stdout, _card);

    // _print_sd_files();

    return ret;
}