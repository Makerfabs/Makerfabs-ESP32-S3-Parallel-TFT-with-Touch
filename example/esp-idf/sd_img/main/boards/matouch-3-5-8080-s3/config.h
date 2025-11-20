#ifndef _BOARD_CONFIG_H_
#define _BOARD_CONFIG_H_

#include <driver/gpio.h>

#define AUDIO_INPUT_SAMPLE_RATE  16000
#define AUDIO_OUTPUT_SAMPLE_RATE 16000


#define SPI2_MOSI_PIN   GPIO_NUM_2
#define SPI2_MISO_PIN   GPIO_NUM_41
#define SPI2_CLK_PIN    GPIO_NUM_42

#define SPI3_MOSI_PIN   GPIO_NUM_13
#define SPI3_MISO_PIN   GPIO_NUM_12
#define SPI3_CLK_PIN    GPIO_NUM_14

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


#define DISPLAY_BACKLIGHT_PIN       GPIO_NUM_45
#define DISPLAY_MOSI_PIN            GPIO_NUM_13
#define DISPLAY_MISO_PIN            GPIO_NUM_12
#define DISPLAY_CLK_PIN             GPIO_NUM_18
#define DISPLAY_DC_PIN              GPIO_NUM_17
#define DISPLAY_RST_PIN             GPIO_NUM_NC
#define DISPLAY_CS_PIN              GPIO_NUM_46
#define DISPLAY_RD_PIN              GPIO_NUM_48

#define EXAMPLE_LCD_IO_SPI_CS       GPIO_NUM_45
#define EXAMPLE_LCD_IO_SPI_SCK      GPIO_NUM_39
#define EXAMPLE_LCD_IO_SPI_SDO      GPIO_NUM_40
#define EXAMPLE_LCD_IO_RGB_DE       GPIO_NUM_7
#define EXAMPLE_LCD_IO_RGB_PCLK     GPIO_NUM_6
#define EXAMPLE_LCD_IO_RGB_VSYNC    GPIO_NUM_4
#define EXAMPLE_LCD_IO_RGB_HSYNC    GPIO_NUM_5
#define EXAMPLE_LCD_IO_RGB_DISP 
#define EXAMPLE_LCD_IO_RGB_RESET    GPIO_NUM_NC

#define EXAMPLE_LCD_IO_DATA0        GPIO_NUM_47
#define EXAMPLE_LCD_IO_DATA1        GPIO_NUM_21
#define EXAMPLE_LCD_IO_DATA2        GPIO_NUM_14
#define EXAMPLE_LCD_IO_DATA3        GPIO_NUM_13
#define EXAMPLE_LCD_IO_DATA4        GPIO_NUM_12
#define EXAMPLE_LCD_IO_DATA5        GPIO_NUM_11
#define EXAMPLE_LCD_IO_DATA6        GPIO_NUM_10
#define EXAMPLE_LCD_IO_DATA7        GPIO_NUM_9
#define EXAMPLE_LCD_IO_DATA8        GPIO_NUM_3
#define EXAMPLE_LCD_IO_DATA9        GPIO_NUM_8
#define EXAMPLE_LCD_IO_DATA10       GPIO_NUM_16
#define EXAMPLE_LCD_IO_DATA11       GPIO_NUM_15
#define EXAMPLE_LCD_IO_DATA12       GPIO_NUM_7
#define EXAMPLE_LCD_IO_DATA13       GPIO_NUM_6
#define EXAMPLE_LCD_IO_DATA14       GPIO_NUM_5
#define EXAMPLE_LCD_IO_DATA15       GPIO_NUM_4




#define TOUCH_SDA_PIN               GPIO_NUM_38
#define TOUCH_SCL_PIN               GPIO_NUM_39

#define SDCARD_MOUNT_POINT          "/sdcard"
#define SDCARD_CS_PIN               GPIO_NUM_1
#define SDCARD_MISO_PIN             SPI2_MISO_PIN
#define SDCARD_MOSI_PIN             SPI2_MOSI_PIN
#define SDCARD_CLK_PIN              SPI2_CLK_PIN




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
