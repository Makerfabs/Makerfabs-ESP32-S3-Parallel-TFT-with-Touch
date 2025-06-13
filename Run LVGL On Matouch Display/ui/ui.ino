/*
MaTouch 3.5Parallel v2.0

Arduino IDE V2.3.4
esp32 v2.0.16
lvgl v8.3.11
LovyanGFX v1.1.9

Tools:
Flash size: 16MB(128Mb)
Partition Schrme: 16M Flash(3MB APP/9.9MB FATFS)
PSRAM: OPI PSRAM
*/
#include <lvgl.h>
#include <ui.h>
#include "S3_Parallel16_ili9488.h"
#include "FT6236.h"

#define I2C_SCL 39
#define I2C_SDA 38

int h[5]={16,89,123,157,170};
int y[5]={23,75,178,256,333};
/*Don't forget to set Sketchbook location in File/Preferencesto the path of your UI project (the parent foder of this INO file)*/

/*Change to your screen resolution*/
static const uint16_t screenWidth = 480;
static const uint16_t screenHeight = 320;

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[screenWidth * screenHeight / 10];

LGFX lcd;
#define LCD_CS 46
#define LCD_BLK 45

void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
    if (lcd.getStartCount() == 0)
    { // Processing if not yet started
        lcd.startWrite();
    }
    lcd.pushImageDMA(area->x1, area->y1, area->x2 - area->x1 + 1, area->y2 - area->y1 + 1, (lgfx::rgb565_t *)&color_p->full);
    lv_disp_flush_ready(disp);
}

/*Read the touchpad*/
void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data)
{
    int touchX = 0, touchY = 0;

    bool touched = false; //

    if (!get_pos(&touchX, &touchY))
    {
        data->state = LV_INDEV_STATE_REL;
    }
    else
    {
        data->state = LV_INDEV_STATE_PR;

        /*Set the coordinates*/
        data->point.x = 480 - touchY;
        data->point.y = touchX;

        Serial.print("Data x ");
        Serial.println(data->point.x);

        Serial.print("Data y ");
        Serial.println(data->point.y);
    }
}

void setup()
{
    // Pin init
    pinMode(LCD_CS, OUTPUT);
    pinMode(LCD_BLK, OUTPUT);

    digitalWrite(LCD_CS, LOW);
    digitalWrite(LCD_BLK, HIGH);

    Wire.begin(I2C_SDA, I2C_SCL);

    Serial.begin(115200); /* prepare for possible serial debug */

    String LVGL_Arduino = "Hello Arduino! ";
    LVGL_Arduino += String('V') + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();

    Serial.println(LVGL_Arduino);
    Serial.println("I am LVGL_Arduino");

    lv_init();

#if LV_USE_LOG != 0
    lv_log_register_print_cb(my_print); /* register print function for debugging */
#endif

    lcd.init();
    lcd.setRotation(3);

    lv_disp_draw_buf_init(&draw_buf, buf, NULL, screenWidth * screenHeight / 10);

    /*Initialize the display*/
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    /*Change the following line to your display resolution*/
    disp_drv.hor_res = screenWidth;
    disp_drv.ver_res = screenHeight;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);

    /*Initialize the (dummy) input device driver*/
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = my_touchpad_read;
    lv_indev_drv_register(&indev_drv);

    ui_init();

    Serial.println("Setup done");
    
    xTaskCreatePinnedToCore(Task_TFT, "Task_TFT", 40960, NULL, 3, NULL, 1);
    xTaskCreatePinnedToCore(arc_change, "arc_change", 20000, NULL, 1, NULL, 0);
}

void loop()
{
}

void Task_TFT(void *pvParameters)
{
    while (1)
    {
        lv_timer_handler();
        vTaskDelay(10);
    }
}

void arc_change(void *pvParameters)
{
  while (1)
  {
    for(int i=0;i<3;i++)
    {
      lv_arc_set_value(ui_Arc1, h[i]);
      _ui_arc_set_text_value(ui_Label5, ui_Arc1,"","");
      vTaskDelay(800);
       lv_arc_set_value(ui_Arc2, y[i]);
      _ui_arc_set_text_value(ui_Label8, ui_Arc2,"","");
      vTaskDelay(800);
    }
    vTaskDelay(500);
  }
}
