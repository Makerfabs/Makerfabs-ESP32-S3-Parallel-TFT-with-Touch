#include <lvgl.h>
#include <ui.h>
#include "S3_Parallel16_ili9488.h"
#include "FT6236.h"
#include <WiFi.h>

#define SSID "Makerfabs"
#define PWD "20160704"

#define I2C_SCL 39
#define I2C_SDA 38

/*Don't forget to set Sketchbook location in File/Preferencesto the path of your UI project (the parent foder of this INO file)*/

/*Change to your screen resolution*/
static const uint16_t screenWidth = 480;
static const uint16_t screenHeight = 320;

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[screenWidth * screenHeight / 10];

// NTP
const char *ntpServer = "120.25.108.11";
int net_flag = 0;

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

        //Serial.print("Data x ");
        //Serial.println(data->point.x);

        //Serial.print("Data y ");
        //Serial.println(data->point.y);
    }
}

void wifi_init()
{
    WiFi.begin(SSID, PWD);

    int connect_count = 0;
    while (WiFi.status() != WL_CONNECTED)
    {
        vTaskDelay(500);
        Serial.print(".");
        connect_count++;
    }

    Serial.println("Wifi connect");
    configTime((const long)(8 * 3600), 0, ntpServer);

    net_flag = 1;
}

long task_runtime_1 = 0;
void Task_my(void *pvParameters)
{
    while (1)
    {
        if (net_flag == 1)
            if ((millis() - task_runtime_1) > 1000)
            {
                display_time();

                task_runtime_1 = millis();
            }

        vTaskDelay(100);
    }
}

void display_time()
{
    struct tm timeinfo;

    if (!getLocalTime(&timeinfo))
    {
        Serial.println("Failed to obtain time");
        return;
    }
    else
    {
        int year = timeinfo.tm_year + 1900;
        int month = timeinfo.tm_mon + 1;
        int day = timeinfo.tm_mday;
        int hour = timeinfo.tm_hour;
        int min = timeinfo.tm_min;
        int sec = timeinfo.tm_sec;

        lv_label_set_text_fmt(ui_Label5, "%d.", year); //year

        if(day<10){lv_label_set_text_fmt(ui_Label4, "December 0%d", day);} //day
        else {lv_label_set_text_fmt(ui_Label4, "December %d", day);}

        if(min<10){lv_label_set_text_fmt(ui_Label2, "%d:0%d", hour,min);} //time
        else {lv_label_set_text_fmt(ui_Label2, "%d:%d", hour,min);}

        if(24-day<10){lv_label_set_text_fmt(ui_Label9, "0%d", 24-day);}//counter days
        else {lv_label_set_text_fmt(ui_Label9, "%d", 24-day);}

        if(23-hour<10){lv_label_set_text_fmt(ui_Label10, "0%d", 23-hour);}//counter hour
        else {lv_label_set_text_fmt(ui_Label10, "%d", 23-hour);}

        if(59-min<10){lv_label_set_text_fmt(ui_Label11, "0%d", 59-min);}//counter mins
        else {lv_label_set_text_fmt(ui_Label11, "%d", 59-min);}

        if(59-sec<10){lv_label_set_text_fmt(ui_Label13, "0%d", 59-sec);}//counter sec
        else {lv_label_set_text_fmt(ui_Label13, "%d", 59-sec);}
    }
}

void Task_TFT(void *pvParameters)
{
    while (1)
    {
        lv_timer_handler(); /* let the GUI do its work */
        delay(5);
        //lv_label_set_text_fmt(ui_Label29, "%d", b);
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

    wifi_init();

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
    disp_drv.sw_rotate = 1;   // add for rotation
    disp_drv.rotated = LV_DISP_ROT_90;   // add for rotation
    lv_disp_drv_register(&disp_drv);

    /*Initialize the (dummy) input device driver*/
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = my_touchpad_read;
    lv_indev_drv_register(&indev_drv);

    ui_init();

    xTaskCreatePinnedToCore(Task_TFT, "Task_TFT", 40960, NULL, 3, NULL, 0);
    xTaskCreatePinnedToCore(Task_my, "Task_my", 20000, NULL, 1, NULL, 1);

    Serial.println("Setup done");
}

void loop()
{
}
