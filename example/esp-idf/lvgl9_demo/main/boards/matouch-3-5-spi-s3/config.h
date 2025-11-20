#ifndef _BOARD_CONFIG_H_
#define _BOARD_CONFIG_H_

#include <driver/gpio.h>

#define SPI2_MOSI_PIN   GPIO_NUM_2
#define SPI2_MISO_PIN   GPIO_NUM_41
#define SPI2_CLK_PIN    GPIO_NUM_42

#define SPI3_MOSI_PIN   GPIO_NUM_13
#define SPI3_MISO_PIN   GPIO_NUM_12
#define SPI3_CLK_PIN    GPIO_NUM_14



#define AUDIO_INPUT_SAMPLE_RATE  16000
#define AUDIO_OUTPUT_SAMPLE_RATE 16000

// 如果使用 Duplex I2S 模式，请注释下面一行
// #define AUDIO_I2S_METHOD_SIMPLEX

#ifdef AUDIO_I2S_METHOD_SIMPLEX

#define AUDIO_I2S_MIC_GPIO_WS       GPIO_NUM_2
#define AUDIO_I2S_MIC_GPIO_SCK      GPIO_NUM_42
#define AUDIO_I2S_MIC_GPIO_DIN      GPIO_NUM_41
#define AUDIO_I2S_SPK_GPIO_DOUT     GPIO_NUM_19
#define AUDIO_I2S_SPK_GPIO_BCLK     GPIO_NUM_20
#define AUDIO_I2S_SPK_GPIO_LRCK     GPIO_NUM_1

#else

#define AUDIO_I2S_GPIO_WS           GPIO_NUM_43
#define AUDIO_I2S_GPIO_BCLK         GPIO_NUM_44
#define AUDIO_I2S_GPIO_DIN          GPIO_NUM_2
#define AUDIO_I2S_GPIO_DOUT         GPIO_NUM_19

#endif


#define BUILTIN_LED_GPIO            GPIO_NUM_NC
#define BOOT_BUTTON_GPIO            GPIO_NUM_0
#define TOUCH_BUTTON_GPIO           GPIO_NUM_NC
#define VOLUME_UP_BUTTON_GPIO       GPIO_NUM_NC
#define VOLUME_DOWN_BUTTON_GPIO     GPIO_NUM_NC


#define DISPLAY_BACKLIGHT_PIN       GPIO_NUM_48
#define DISPLAY_MOSI_PIN            SPI2_MOSI_PIN
#define DISPLAY_MISO_PIN            SPI2_MISO_PIN
#define DISPLAY_CLK_PIN             SPI2_CLK_PIN
#define DISPLAY_DC_PIN              GPIO_NUM_21
#define DISPLAY_RST_PIN             GPIO_NUM_NC
#define DISPLAY_CS_PIN              GPIO_NUM_15
#define DISPLAY_RD_PIN              GPIO_NUM_48



#define TOUCH_SDA_PIN               GPIO_NUM_38
#define TOUCH_SCL_PIN               GPIO_NUM_39

#define SDCARD_MOUNT_POINT          "/sdcard"
#define SDCARD_CS_PIN               GPIO_NUM_1
#define SDCARD_MISO_PIN             SPI1_MISO_PIN
#define SDCARD_MOSI_PIN             SPI1_MOSI_PIN
#define SDCARD_CLK_PIN              SPI1_CLK_PIN


#define DISPLAY_WIDTH                   320
#define DISPLAY_HEIGHT                  480
#define DISPLAY_MIRROR_X                false
#define DISPLAY_MIRROR_Y                false
#define DISPLAY_SWAP_XY                 false
#define DISPLAY_INVERT_COLOR            true
#define DISPLAY_RGB_ORDER               LCD_RGB_ELEMENT_ORDER_BGR
#define DISPLAY_OFFSET_X                0
#define DISPLAY_OFFSET_Y                0
#define DISPLAY_BACKLIGHT_OUTPUT_INVERT false
#define DISPLAY_SPI_MODE                0



// A MCP Test: Control a lamp
#define LAMP_GPIO GPIO_NUM_18

#endif // _BOARD_CONFIG_H_
