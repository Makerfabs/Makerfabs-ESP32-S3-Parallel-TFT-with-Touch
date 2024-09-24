
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

#define COLOR_BACKGROUND TFT_BLACK
#define COLOR_BUTTON TFT_BLACK
#define COLOR_BUTTON_P 0x4BAF
#define COLOR_TEXT TFT_WHITE
#define COLOR_LINE TFT_WHITE
#define COLOR_SHADOW 0x4BAF

#define BUTTON_POS_X 10
#define BUTTON_POS_Y 90

#define BUTTON_DELAY 150

#define BUTTON_COUNT_M 3
#define BUTTON_COUNT_P1 5
#define BUTTON_COUNT_P2 12
#define BUTTON_COUNT_P3 5

const char *ntpServer = "120.25.108.11";
const long gmtOffset_sec = (-5) * 60 * 60; // China+8
const int daylightOffset_sec = 0;
struct tm timeinfo;
String global_time = "No time";

LGFX lcd;
USBHIDKeyboard Keyboard;

int pos[2] = {0, 0};

void main_page();
void page1();
void page2();
void page3();
void lcd_init();
void drawButton(Button b);
void drawButton_p(Button b);
void clean_button();
void ntp_init();
void fresh_time();
void page_switch(int page);
void key_input_1(int value);
void key_input_2(int value);
void key_input_3(int value);


void setup()
{
    Serial.begin(115200);
    Serial.println("Keyboard begin");

    lcd_init();
    Keyboard.begin();
    USB.begin();

    main_page();
}

void loop()
{
}

// Pages

void main_page()
{
    Button b[BUTTON_COUNT_M];

    String b_list[BUTTON_COUNT_M] = {
        "Keyboard 1",
        "Keyboard 2",
        "Keyboard 3"};

    // Button set
    for (int i = 0; i < BUTTON_COUNT_M; i++)
    {

        b[i].set(BUTTON_POS_X, BUTTON_POS_Y + 80 * i, 200, 60, "NULL", ENABLE);
        b[i].setText(b_list[i]);
        b[i].setValue(i);

        drawButton(b[i]);
    }

    lcd.setTextSize(8);
    lcd.setCursor(10, 360);
    lcd.setTextColor(0xFE07);
    lcd.print("16Bit");

    while (1)
    {
        ft6236_pos(pos);

        for (int i = 0; i < BUTTON_COUNT_P1; i++)
        {
            int button_value = UNABLE;
            if ((button_value = b[i].checkTouch(pos[0], pos[1])) != UNABLE)
            {

                Serial.printf("Pos is :%d,%d\n", pos[0], pos[1]);
                Serial.printf("Value is :%d\n", button_value);
                Serial.printf("Text is :");
                Serial.println(b[i].getText());

                drawButton_p(b[i]);
                delay(BUTTON_DELAY);
                drawButton(b[i]);

                page_switch(button_value);

                delay(200);
            }
        }
    }
}

void page1()
{
    Button b[BUTTON_COUNT_P1];

    String b_list[BUTTON_COUNT_P1] = {
        "Makerfabs",
        "Passward",
        "Now Time",
        "Ctrl + V",
        "Enter"};

    clean_button();
    ntp_init();

    // Button set
    for (int i = 0; i < BUTTON_COUNT_P1; i++)
    {

        b[i].set(BUTTON_POS_X, BUTTON_POS_Y + 80 * i, 200, 60, "NULL", ENABLE);
        b[i].setText(b_list[i]);
        b[i].setValue(i);

        drawButton(b[i]);
    }

    while (1)
    {
        ft6236_pos(pos);

        for (int i = 0; i < BUTTON_COUNT_P1; i++)
        {
            int button_value = UNABLE;
            if ((button_value = b[i].checkTouch(pos[0], pos[1])) != UNABLE)
            {

                Serial.printf("Pos is :%d,%d\n", pos[0], pos[1]);
                Serial.printf("Value is :%d\n", button_value);
                Serial.printf("Text is :");
                Serial.println(b[i].getText());

                drawButton_p(b[i]);
                delay(BUTTON_DELAY);
                drawButton(b[i]);
                key_input_1(button_value);
                delay(200);
            }
        }
    }
}

void page2()
{
    Button b[BUTTON_COUNT_P2];

    String b_list[BUTTON_COUNT_P2] = {
        "1",
        "2",
        "3",
        "4",
        "5",
        "6",
        "7",
        "8",
        "9",
        "0",
        "DEL",
        "ENTER"};

    clean_button();

    // Button set
    for (int i = 0; i < BUTTON_COUNT_P2; i++)
    {

        b[i].set(BUTTON_POS_X + i % 3 * 100, BUTTON_POS_Y + i / 3 * 90, 80, 80, "NULL", ENABLE);
        b[i].setText(b_list[i]);
        b[i].setValue(i);

        drawButton(b[i]);
    }

    while (1)
    {
        ft6236_pos(pos);

        for (int i = 0; i < BUTTON_COUNT_P2; i++)
        {
            int button_value = UNABLE;
            if ((button_value = b[i].checkTouch(pos[0], pos[1])) != UNABLE)
            {

                Serial.printf("Pos is :%d,%d\n", pos[0], pos[1]);
                Serial.printf("Value is :%d\n", button_value);
                Serial.printf("Text is :");
                Serial.println(b[i].getText());

                drawButton_p(b[i]);
                delay(BUTTON_DELAY);
                drawButton(b[i]);
                key_input_2(button_value);
                delay(200);
            }
        }
    }
}

void page3()
{
    Button b[BUTTON_COUNT_P3];

    String b_list[BUTTON_COUNT_P3] = {
        "^",
        "v",
        "<",
        ">",
        "    SPACE"};

    clean_button();

    b[0].set(120, 90, 80, 80, "NULL", ENABLE);
    b[1].set(120, 180, 80, 80, "NULL", ENABLE);
    b[2].set(20, 180, 80, 80, "NULL", ENABLE);
    b[3].set(220, 180, 80, 80, "NULL", ENABLE);
    b[4].set(20, 360, 280, 80, "NULL", ENABLE);

    // Button set
    for (int i = 0; i < BUTTON_COUNT_P3; i++)
    {
        b[i].setText(b_list[i]);
        b[i].setValue(i);
        drawButton(b[i]);
    }

    while (1)
    {
        ft6236_pos(pos);

        for (int i = 0; i < BUTTON_COUNT_P3; i++)
        {
            int button_value = UNABLE;
            if ((button_value = b[i].checkTouch(pos[0], pos[1])) != UNABLE)
            {

                Serial.printf("Pos is :%d,%d\n", pos[0], pos[1]);
                Serial.printf("Value is :%d\n", button_value);
                Serial.printf("Text is :");
                Serial.println(b[i].getText());

                drawButton_p(b[i]);
                delay(BUTTON_DELAY);
                drawButton(b[i]);
                key_input_3(button_value);
                delay(200);
            }
        }
    }
}

// Hardware init
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
}

// Draw button and shadow

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
    lcd.print(text);

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
    lcd.print(text);
}

void clean_button()
{
    lcd.fillRect(BUTTON_POS_X, BUTTON_POS_Y, 319 - BUTTON_POS_X, 479 - BUTTON_POS_Y, COLOR_BACKGROUND);
}

// NTP time and wifi connect
void ntp_init()
{
    lcd.setCursor(10, 58);
    lcd.print("Connect to NTP...");
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
    while (1)
    {
        fresh_time();
        if (global_time.indexOf("N") == -1)
            break;
    }
    lcd.setCursor(220, 58);
    lcd.print("Over");
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

// Button Command
void page_switch(int page)
{
    switch (page)
    {
    case 0:
        page1();
        break;
    case 1:
        page2();
        break;
    case 2:
        page3();
        break;

    defualt:
        break;
    }
    delay(100);
}

void key_input_1(int value)
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

void key_input_2(int value)
{
    if (value < 9)
    {
        Keyboard.write('1' + value);
    }
    else if (value == 9)
        Keyboard.write('0');
    else if (value == 10)
        Keyboard.write(KEY_BACKSPACE);
    else if (value == 11)
        Keyboard.write(KEY_RETURN);

    delay(100);
    Keyboard.releaseAll();
}

void key_input_3(int value)
{
    switch (value)
    {
    case 0:
        Keyboard.press(KEY_UP_ARROW);
        break;
    case 1:

        Keyboard.press(KEY_DOWN_ARROW);
        break;
    case 2:
        Keyboard.press(KEY_LEFT_ARROW);
        break;
    case 3:
        Keyboard.press(KEY_RIGHT_ARROW);
        break;
    case 4:
        Keyboard.press(0x20);
        break;
    defualt:
        break;
    }
    delay(100);
    Keyboard.releaseAll();
}
