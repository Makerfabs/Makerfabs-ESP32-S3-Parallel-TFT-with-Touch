
set(SOURCES)
set(INCLUDE_DIRS ".")
set(REQUIRES_LIST)

# Check if CONFIG_SPIFFS_ENABLE is enabled
if(CONFIG_SPIFFS_ENABLE)
    list(APPEND SOURCES "bsp_spiffs/bsp_spiffs.c")
    list(APPEND INCLUDE_DIRS "bsp_spiffs")
endif()

if(CONFIG_SDCARD_ENABLE)
    list(APPEND SOURCES "sdcard/bsp_sdcard.c")
    list(APPEND INCLUDE_DIRS "sdcard")
endif()

if(CONFIG_AUDIO_ENABLE)
    list(APPEND SOURCES "audio/no_audio_codec.c")
    list(APPEND INCLUDE_DIRS "audio")
endif()

if(CONFIG_QMI8658_ENABLE)
    list(APPEND SOURCES "qmi8658/bsp_qmi8658.c")
    list(APPEND INCLUDE_DIRS "qmi8658")
endif()

if(CONFIG_PCF85063A_ENABLE)
    list(APPEND SOURCES "pcf85063a/bsp_pcf85063a.c")
    list(APPEND INCLUDE_DIRS "pcf85063a")
endif()

message(STATUS "Component SOURCES: ${SOURCES}")
message(STATUS "Component INCLUDE_DIRS: ${INCLUDE_DIRS}")
message(STATUS "Component REQUIRES_LIST: ${REQUIRES_LIST}")

idf_component_register(SRCS ${SOURCES}
                    INCLUDE_DIRS ${INCLUDE_DIRS}
                    REQUIRES spiffs esp_driver_i2s esp_driver_gpio qmi8658 pcf85063a sdmmc fatfs esp-audio-player
                    )
