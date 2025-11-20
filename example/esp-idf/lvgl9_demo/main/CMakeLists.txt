set(SOURCES 
            "main.c"


            )

set(INCLUDE_DIRS "." )

if(DEFINED CONFIG_LVGL_VERSION_MAJOR)
    message("add v9 display files")
    file(GLOB DISPLAY_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/display/v9/*.c
    )
    list(APPEND SOURCES ${DISPLAY_SOURCES})
    list (APPEND INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/display/v9)
else()
    message("add v8 display files")
    file(GLOB DISPLAY_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/display/v8/*.c
    )
    list(APPEND SOURCES ${DISPLAY_SOURCES})
    list (APPEND INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/display/v8)
endif()


# Add board files according to BOARD_TYPE
# Set default assets if the board uses partition table V2
if(CONFIG_BOARD_TYPE_MATOUCH_2_8_S3)
    set(BOARD_TYPE "matouch-2-8-s3")
elseif(CONFIG_BOARD_TYPE_MATOUCH_3_16_S3)
    set(BOARD_TYPE "matouch-3-16-s3")
elseif(CONFIG_BOARD_TYPE_MATOUCH_1_8_S3)
    set(BOARD_TYPE "matouch-1-8-s3")
elseif(CONFIG_BOARD_TYPE_MATOUCH_3_5_SPI_S3)
    set(BOARD_TYPE "matouch-3-5-spi-s3")
elseif(CONFIG_BOARD_TYPE_MATOUCH_3_5_8080_S3)
    set(BOARD_TYPE "matouch-3-5-8080-s3")
endif()

message("current board type: ${BOARD_TYPE}")


file(GLOB BOARD_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/boards/${BOARD_TYPE}/*.c
)
list(APPEND SOURCES ${BOARD_SOURCES})


if(CONFIG_DEVELOPMENT_MODE_DEBUG)
    file(GLOB DEV_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/debug/${BOARD_TYPE}/*.c
    )
    list(APPEND SOURCES ${DEV_SOURCES})
elseif(CONFIG_DEVELOPMENT_MODE_RELEASE)
    file(GLOB DEV_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/release/${BOARD_TYPE}/*.c
    )
    file (GLOB ASSETS_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/release/${BOARD_TYPE}/assets/*.c
    )
    list(APPEND SOURCES ${ASSETS_SOURCES})
    list(APPEND SOURCES ${DEV_SOURCES})
endif()


idf_component_register(SRCS ${SOURCES}
                    # EMBED_FILES ${LANG_SOUNDS} ${COMMON_SOUNDS}
                    INCLUDE_DIRS ${INCLUDE_DIRS}
                    # WHOLE_ARCHIVE
                    )

# spiffs_create_partition_image(storage ../spiffs FLASH_IN_PROJECT)