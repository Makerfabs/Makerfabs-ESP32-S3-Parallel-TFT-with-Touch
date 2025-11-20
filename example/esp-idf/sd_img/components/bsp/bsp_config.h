#pragma once

#define AUDIO_DATA_BIT_WIDTH        (16)
#define AUDIO_SLOT_MODE             (1)
#define AUDIO_SAMPLES               (1024)

#define I2S_HW_VERSION_2

#if CONFIG_AUDIO_ENABLE
#include "audio/no_audio_codec.h"
#include "audio/bsp_wav.h"
#endif


#if CONFIG_SPIFFS_ENABLE
#include "bsp_spiffs/bsp_spiffs.h"
#endif

#if CONFIG_QMI8658_ENABLE
#include "qmi8658/bsp_qmi8658.h"
#endif

#if CONFIG_SDCARD_ENABLE
#include "sdcard/bsp_sdcard.h"
#endif

#if CONFIG_PCF85063A_ENABLE
#include "pcf85063a/bsp_pcf85063a.h"
#endif