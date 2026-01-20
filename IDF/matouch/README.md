# Example Description
This project includes example code for some MaTouch products.
- [**MaTouch 1.28" ToolSet_Controller**](#matouch-128-toolset_controller)
- [**MaTouch_ESP32-S3 Parallel TFT with Touch 4.3"**](#matouch_esp32-s3-parallel-tft-with-touch-43)
- [**Makerfabs-ESP32-S3-SPI-TFT-with-Touch**](#makerfabs-esp32-s3-spi-tft-with-touch)
- [**Makerfabs-ESP32-S3-Parallel-TFT-with-Touch**](#makerfabs-esp32-s3-parallel-tft-with-touch)

This repository is suitable for the [**Makerfabs-ESP32-S3-Parallel-TFT-with-Touch**](#makerfabs-esp32-s3-parallel-tft-with-touch).

## How to use example
#### MaTouch 1.28" ToolSet_Controller
Select the development board as **1-28-gc9a01-cst816s** .
![image-20251222183148326](https://easyimage.linwanrong.com/i/2025/12/22/uagknl-0.webp)
This example includes four routines：
> `main/firmware/1-28-gc9a01-cst816s/release.c`

![image-20251222183606880](https://easyimage.linwanrong.com/i/2025/12/22/ud6c66-0.webp)
- xuanniu_test：Example of a rotary encoder.
![aadb7e029fc8b3403fd312ede316a084-ezgif.com-optimize](https://easyimage.linwanrong.com/i/2025/12/23/fdclmu-0.gif)
- template_image_switch_lvgl: Use LVGL's built-in libturbo-jpeg decoder to load JPG images from the SD card.
![image_switch](https://easyimage.linwanrong.com/i/2025/12/22/vt59vi-0.gif)
- template_lvgl_demos_test: Examples included with LVGL.  
![qq_pic_merged_1766452938301](https://easyimage.linwanrong.com/i/2025/12/23/f9a5e0-0.webp)
- template_pcf8563_test: A simple PCF8563 RTC example.

#### MaTouch_ESP32-S3 Parallel TFT with Touch 4.3"
Select the development board as **4-3-unkonwn-gt911** .
![clipboard_2025-12-22_23-23](https://easyimage.linwanrong.com/i/2025/12/23/dzyu9t-0.webp)
This example includes two routines：

> `main/firmware/4-3-unkonwn-gt911/release.c`

![clipboard_2025-12-22_23-2](https://easyimage.linwanrong.com/i/2025/12/23/e10ojm-0.webp)
- template_image_switch_lvgl: Use LVGL's built-in libturbo-jpeg decoder to load JPG images from the SD card.
![ezgif.com-optimize](https://easyimage.linwanrong.com/i/2025/12/23/f641rz-0.gif)
- template_lvgl_demos_test: Examples included with LVGL.
![qq_pic_merged_1766453592919](https://easyimage.linwanrong.com/i/2025/12/23/ffq74n-0.webp)

#### Makerfabs-ESP32-S3-SPI-TFT-with-Touch

Select the development board as **3-5-ili9488-ft6236** .

![image-20251231115857035](https://easyimage.linwanrong.com/i/2025/12/31/j6249o-0.webp)

This example includes two routines：

> `main/firmware/3-5-ili9488-ft6236/release.c`

```c
static void release_task(void *arg)
{
    template_image_switch_lvgl("/sdcard/images");
    // template_lvgl_demos_test();
    vTaskDelete(NULL);
}
```

- template_image_switch_lvgl: Use LVGL's built-in libturbo-jpeg decoder to load JPG images from the SD card.

  ![VID20251231133347-ezgif.com-optimize](https://easyimage.linwanrong.com/i/2025/12/31/m5vgnp-0.gif)

- template_lvgl_demos_test: Examples included with LVGL.

  ![ezgif.com-optimize](https://easyimage.linwanrong.com/i/2026/01/04/hab40y-0.gif)

#### Makerfabs-ESP32-S3-Parallel-TFT-with-Touch

Select the development board as **3-5-ili9488-ft6236** .

![image-20260103170309468](https://easyimage.linwanrong.com/i/2026/01/03/s60qu5-0.webp)

This example includes two routines：

> `main/firmware/3-5-ili9488-ft6236/release.c`

```c
static void release_task(void *arg)
{
    template_image_switch_lvgl("/sdcard/images");
    // template_lvgl_demos_test();
    vTaskDelete(NULL);
}
```

- template_image_switch_lvgl: Use LVGL's built-in libturbo-jpeg decoder to load JPG images from the SD card.

  ![VID20251231133347-ezgif.com-optimize](https://easyimage.linwanrong.com/i/2025/12/31/m5vgnp-0.gif)

- template_lvgl_demos_test: Examples included with LVGL.

  ![ezgif.com-optimize](https://easyimage.linwanrong.com/i/2026/01/04/hab40y-0.gif)

## Example Note

If using `template_image_switch_lvgl` example, the following options must be enabled in menuconfig.

> Modify line 64 of `managed_components/lvgl__lvgl/env_support/cmake/esp.cmake`:
>
> ```cmake
> REQUIRES esp_timer espressif__libjpeg-turbo)
> ```

![image-20251223090228410](https://easyimage.linwanrong.com/i/2025/12/23/ex8p32-0.webp)
![image-20251223090352046](https://easyimage.linwanrong.com/i/2025/12/23/exypsg-0.webp)

If using the `template_lvgl_demos_test` example, enable different LVGL demos in menuconfig.
![image-20251223090701857](https://easyimage.linwanrong.com/i/2025/12/23/f027t7-0.webp)

























