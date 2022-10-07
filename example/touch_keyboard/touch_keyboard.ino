
#include "USB.h"
#include "USBHIDKeyboard.h"
#include "SPI.h"
#include "FT6236.h"
#include "Parallel16_9488.h"
#include "Button.h"
#include <WiFi.h>
#include <time.h>

#define SSID "Makerfabs"
#define PWD "20160704"

#define I2C_SCL 39
#define I2C_SDA 38

#define COLOR_BACKGROUND 0xEF9E
#define COLOR_BUTTON TFT_WHITE
#define COLOR_BUTTON_P TFT_YELLOW
#define COLOR_TEXT 0x322B
#define COLOR_LINE TFT_BLACK
#define COLOR_SHADOW 0x4BAF

#define BUTTON_COUNT 5

const char *ntpServer = "120.25.108.11";
const long gmtOffset_sec = (-5) * 60 * 60; // China+8
const int daylightOffset_sec = 0;
struct tm timeinfo;
String global_time = "No time";

LGFX lcd;
USBHIDKeyboard Keyboard;

int pos[2] = {0, 0};
Button b[BUTTON_COUNT];

String b_list[BUTTON_COUNT] = {
    "Makerfabs",
    "Passward",
    "Now Time",
    "Ctrl + V",
    "Enter"};

void setup()
{
    Serial.begin(115200);
    Serial.println("Keyboard begin");

    lcd_init();

    WiFi.begin(SSID, PWD);
    int connect_count = 0;
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
        connect_count++;
        if (connect_count > 20)
        {
            Serial.println("Wifi time out");
            break;
        }
    }
    configTime(28800, daylightOffset_sec, ntpServer);

    Keyboard.begin();
    USB.begin();

    delay(2000);

    while (1)
    {
        fresh_time();
        if (global_time.indexOf("N") == -1)
            break;
    }
    lcd.setCursor(220, 58);
    lcd.print("Over");

    // Set lora node and draw buttons
    for (int i = 0; i < BUTTON_COUNT; i++)
    {

        b[i].set(10, 90 + 80 * i, 200, 60, "NULL", ENABLE);

        // String show_txt = "Temp:";
        // show_txt = show_txt + i;

        b[i].setText(b_list[i]);
        b[i].setValue(i);

        drawButton(b[i]);
    }
}

void loop()
{
    ft6236_pos(pos);

    for (int i = 0; i < BUTTON_COUNT; i++)
    {
        int button_value = UNABLE;
        if ((button_value = b[i].checkTouch(pos[0], pos[1])) != UNABLE)
        {

            Serial.printf("Pos is :%d,%d\n", pos[0], pos[1]);
            Serial.printf("Value is :%d\n", button_value);
            Serial.printf("Text is :");
            Serial.println(b[i].getText());

            drawButton_p(b[i]);
            key_input(button_value);
            delay(500);

            drawButton(b[i]);
            delay(200);
        }
    }
}

void lcd_init()
{
    // Pin init
    pinMode(LCD_CS, OUTPUT);
    pinMode(LCD_BLK, OUTPUT);

    digitalWrite(LCD_CS, LOW);
    digitalWrite(LCD_BLK, HIGH);

    // lcd init
    lcd.init();
    lcd.fillScreen(COLOR_BACKGROUND);
    lcd.setTextColor(COLOR_TEXT);
    lcd.setTextSize(2);
    lcd.setCursor(10, 10);
    lcd.print("Makerfabs ESP32-S3");
    lcd.setCursor(10, 26);
    lcd.print("Parallel lcd with Touch");
    lcd.setCursor(10, 42);
    lcd.print("Touch Keyboard Demo");

    // I2C init
    Wire.begin(I2C_SDA, I2C_SCL);
    byte error, address;

    Wire.beginTransmission(TOUCH_I2C_ADD);
    error = Wire.endTransmission();

    if (error == 0)
    {
        Serial.print("I2C device found at address 0x");
        Serial.print(TOUCH_I2C_ADD, HEX);
        Serial.println("  !");
    }
    else
    {
        Serial.print("Unknown error at address 0x");
        Serial.println(TOUCH_I2C_ADD, HEX);
    }

    lcd.setCursor(10, 58);
    lcd.print("Connect to NTP...");
}

void drawButton(Button b)
{
    int b_x;
    int b_y;
    int b_w;
    int b_h;
    int shadow_len = 4;
    String text;
    int textSize;

    b.getFoDraw(&b_x, &b_y, &b_w, &b_h, &text, &textSize);

    lcd.fillRect(b_x, b_y, b_w, b_h, COLOR_BUTTON);
    lcd.drawRect(b_x, b_y, b_w, b_h, COLOR_LINE);
    lcd.setCursor(b_x + 20, b_y + 20);
    lcd.setTextColor(COLOR_TEXT);
    lcd.setTextSize(textSize);
    lcd.println(text);

    // Add button shadow
    if (b.getValue() != UNABLE)
    {
        lcd.fillRect(b_x + shadow_len, b_y + b_h, b_w, shadow_len, COLOR_SHADOW);
        lcd.fillRect(b_x + b_w, b_y + shadow_len, shadow_len, b_h, COLOR_SHADOW);
    }
}

void drawButton_p(Button b)
{
    int b_x;
    int b_y;
    int b_w;
    int b_h;
    int shadow_len = 4;
    String text;
    int textSize;

    b.getFoDraw(&b_x, &b_y, &b_w, &b_h, &text, &textSize);

    lcd.fillRect(b_x, b_y, b_w + shadow_len, b_h + shadow_len, COLOR_BACKGROUND);

    lcd.fillRect(b_x + shadow_len, b_y + shadow_len, b_w, b_h, COLOR_BUTTON_P);
    lcd.drawRect(b_x + shadow_len, b_y + shadow_len, b_w, b_h, COLOR_LINE);
    lcd.setCursor(b_x + 20, b_y + 20);
    lcd.setTextColor(COLOR_TEXT);
    lcd.setTextSize(textSize);
    lcd.println(text);
}

void key_input(int value)
{
    switch (value)
    {
    case 0:
        Keyboard.print("Makerfabs");
        break;
    case 1:

        Keyboard.print("Password");
        break;
    case 2:
        fresh_time();
        Keyboard.print(global_time);
        break;
    case 3:
        Keyboard.press(KEY_LEFT_CTRL);
        Keyboard.press('V');
        break;
    case 4:
        Keyboard.press(KEY_RETURN);
        break;
    defualt:
        break;
    }
    delay(100);
    Keyboard.releaseAll();
}

void fresh_time()
{
    char time_str[40] = "";
    if (!getLocalTime(&timeinfo))
    {
        Serial.println("Failed to obtain time");
    }
    else
    {
        sprintf(time_str, "%02d/%02d/%02d %02d:%02d:%02d", timeinfo.tm_year + 1900, timeinfo.tm_mon, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
        Serial.println(time_str);
        global_time = (String)time_str;
    }
}