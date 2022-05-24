#define LGFX_USE_V1

#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include <LovyanGFX.hpp>

#define I2C_SCL 39
#define I2C_SDA 38

//#define NS2009_TOUCH  //Resistive screen driver
#define FT6236_TOUCH //Capacitive screen driver

#ifdef NS2009_TOUCH
#include "NS2009.h"
const int i2c_touch_addr = NS2009_ADDR;
#define get_pos ns2009_pos
#endif

#ifdef FT6236_TOUCH
#include "FT6236.h"
const int i2c_touch_addr = TOUCH_I2C_ADD;
#define get_pos ft6236_pos
#endif

#define SPI_MOSI 2 //1
#define SPI_MISO 41
#define SPI_SCK 42
#define SD_CS 1 //2

#define LCD_CS 37
#define LCD_BLK 45


class LGFX : public lgfx::LGFX_Device
{
    // lgfx::Panel_ILI9341 _panel_instance;
    lgfx::Panel_ILI9488 _panel_instance;
    lgfx::Bus_Parallel16 _bus_instance; // 8ビットパラレルバスのインスタンス (ESP32のみ)

public:
    // コンストラクタを作成し、ここで各種設定を行います。
    // クラス名を変更した場合はコンストラクタも同じ名前を指定してください。
    LGFX(void)
    {
        {                                      // バス制御の設定を行います。
            auto cfg = _bus_instance.config(); // バス設定用の構造体を取得します。

            // 16位设置
            cfg.port = 0;              // 使用するI2Sポートを選択 (0 or 1) (ESP32のI2S LCDモードを使用します)
            cfg.freq_write = 20000000; // 送信クロック (最大20MHz, 80MHzを整数で割った値に丸められます)
            cfg.pin_wr = 35;           // WR を接続しているピン番号
            cfg.pin_rd = 48;           // RD を接続しているピン番号
            cfg.pin_rs = 36;           // RS(D/C)を接続しているピン番号

            cfg.pin_d0 = 47;
            cfg.pin_d1 = 21;
            cfg.pin_d2 = 14;
            cfg.pin_d3 = 13;
            cfg.pin_d4 = 12;
            cfg.pin_d5 = 11;
            cfg.pin_d6 = 10;
            cfg.pin_d7 = 9;
            cfg.pin_d8 = 3;
            cfg.pin_d9 = 8;
            cfg.pin_d10 = 16;
            cfg.pin_d11 = 15;
            cfg.pin_d12 = 7;
            cfg.pin_d13 = 6;
            cfg.pin_d14 = 5;
            cfg.pin_d15 = 4;

            _bus_instance.config(cfg);              // 設定値をバスに反映します。
            _panel_instance.setBus(&_bus_instance); // バスをパネルにセットします。
        }

        {                                        // 表示パネル制御の設定を行います。
            auto cfg = _panel_instance.config(); // 表示パネル設定用の構造体を取得します。

            cfg.pin_cs = -1;   // CS要拉低
            cfg.pin_rst = -1;  // RST和开发板RST相连
            cfg.pin_busy = -1; // BUSYが接続されているピン番号 (-1 = disable)

            // ※ 以下の設定値はパネル毎に一般的な初期値が設定されていますので、不明な項目はコメントアウトして試してみてください。

            cfg.memory_width = 320;   // ドライバICがサポートしている最大の幅
            cfg.memory_height = 480;  // ドライバICがサポートしている最大の高さ
            cfg.panel_width = 320;    // 実際に表示可能な幅
            cfg.panel_height = 480;   // 実際に表示可能な高さ
            cfg.offset_x = 0;         // パネルのX方向オフセット量
            cfg.offset_y = 0;         // パネルのY方向オフセット量
            cfg.offset_rotation = 0;  // 回転方向の値のオフセット 0~7 (4~7は上下反転)
            cfg.dummy_read_pixel = 8; // ピクセル読出し前のダミーリードのビット数
            cfg.dummy_read_bits = 1;  // ピクセル以外のデータ読出し前のダミーリードのビット数
            cfg.readable = true;      // データ読出しが可能な場合 trueに設定
            cfg.invert = false;       // パネルの明暗が反転してしまう場合 trueに設定
            cfg.rgb_order = false;    // パネルの赤と青が入れ替わってしまう場合 trueに設定
            cfg.dlen_16bit = true;    // データ長を16bit単位で送信するパネルの場合 trueに設定
            cfg.bus_shared = true;    // SDカードとバスを共有している場合 trueに設定(drawJpgFile等でバス制御を行います)

            _panel_instance.config(cfg);
        }

        setPanel(&_panel_instance); // 使用するパネルをセットします。
    }
};

LGFX lcd;
int pos[2] = {0, 0};
int err_code = 0;

void setup()
{
    pinMode(LCD_CS, OUTPUT);
    pinMode(LCD_BLK, OUTPUT);

    digitalWrite(LCD_CS, LOW);
    digitalWrite(LCD_BLK, HIGH);

    Serial.begin(115200);

    lcd.init();
    lcd.fillScreen(TFT_BLUE);
    lcd.setTextColor(TFT_YELLOW);
    lcd.setTextSize(2);
    lcd.setCursor(0, 0);
    lcd.print("Makerfabs ESP32-S3");
    lcd.setCursor(0, 16);
    lcd.print("Parallel TFT with Touch");

    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);

    lcd.setCursor(0, 32);
    if (SD_init())
    {
        lcd.print("ERROR:   SD");
        err_code += 1;
    }
    else
        lcd.print("SD INIT OVER");

    //I2C init
    Wire.begin(I2C_SDA, I2C_SCL);
    byte error, address;

    Wire.beginTransmission(i2c_touch_addr);
    error = Wire.endTransmission();

    lcd.setCursor(0, 48);
    if (error == 0)
    {
        Serial.print("I2C device found at address 0x");
        Serial.print(i2c_touch_addr, HEX);
        Serial.println("  !");
        lcd.print("TOUCH INIT OVER");
    }
    else
    {
        Serial.print("Unknown error at address 0x");
        Serial.println(i2c_touch_addr, HEX);
        lcd.print("ERROR:   TOUCH");
        err_code += 1;
    }

    lcd.setCursor(0, 64);
    if (err_code)
    {

        lcd.print("SOMETHING WRONG");
        while (1)
            ;
    }
    else
        lcd.print("ALL SUCCESS");

    lcd.setRotation(1);

    int rect_length = 40;

    lcd.fillRect(240 - rect_length, 160 - rect_length, rect_length * 2, rect_length * 2, TFT_RED);

    lcd.setCursor(100, 10);
    lcd.print(F("TOUCH RED RECT"));

    while (1)
    {
        if (get_pos(pos))
        {
            if (pos[0] < 160 + rect_length && pos[0] > 160 - rect_length && pos[1] < 240 + rect_length && pos[1] > 240 - rect_length)
                break;
        }
        delay(10);
        Serial.println("No touch");
    }

    lcd.setRotation(3);
    print_img(SD, "/logo.bmp", 480, 320);
}

void loop()
{
    if (get_pos(pos))
    {
        Serial.println((String)"x=" + pos[0] + ",y=" + pos[1]);
    }

    delay(100);
}

int SD_init()
{

    if (!SD.begin(SD_CS))
    {
        Serial.println("Card Mount Failed");
        return 1;
    }
    uint8_t cardType = SD.cardType();

    if (cardType == CARD_NONE)
    {
        Serial.println("No SD card attached");
        return 1;
    }

    Serial.print("SD Card Type: ");
    if (cardType == CARD_MMC)
    {
        Serial.println("MMC");
    }
    else if (cardType == CARD_SD)
    {
        Serial.println("SDSC");
    }
    else if (cardType == CARD_SDHC)
    {
        Serial.println("SDHC");
    }
    else
    {
        Serial.println("UNKNOWN");
    }

    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    Serial.printf("SD Card Size: %lluMB\n", cardSize);
    listDir(SD, "/", 2);

    Serial.println("SD init over.");
    return 0;
}

void listDir(fs::FS &fs, const char *dirname, uint8_t levels)
{
    Serial.printf("Listing directory: %s\n", dirname);

    File root = fs.open(dirname);
    if (!root)
    {
        Serial.println("Failed to open directory");
        return;
    }
    if (!root.isDirectory())
    {
        Serial.println("Not a directory");
        return;
    }

    File file = root.openNextFile();
    while (file)
    {
        if (file.isDirectory())
        {
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if (levels)
            {
                listDir(fs, file.name(), levels - 1);
            }
        }
        else
        {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("  SIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}

void readFile(fs::FS &fs, const char *path)
{
    Serial.printf("Reading file: %s\n", path);

    File file = fs.open(path);
    if (!file)
    {
        Serial.println("Failed to open file for reading");
        return;
    }

    Serial.print("Read from file: ");
    while (file.available())
    {
        Serial.write(file.read());
    }
    file.close();
}

void writeFile(fs::FS &fs, const char *path, const char *message)
{
    Serial.printf("Writing file: %s\n", path);

    File file = fs.open(path, FILE_WRITE);
    if (!file)
    {
        Serial.println("Failed to open file for writing");
        return;
    }
    if (file.print(message))
    {
        Serial.println("File written");
    }
    else
    {
        Serial.println("Write failed");
    }
    file.close();
}

void appendFile(fs::FS &fs, const char *path, const char *message)
{
    Serial.printf("Appending to file: %s\n", path);

    File file = fs.open(path, FILE_APPEND);
    if (!file)
    {
        Serial.println("Failed to open file for appending");
        return;
    }
    if (file.print(message))
    {
        Serial.println("Message appended");
    }
    else
    {
        Serial.println("Append failed");
    }
    file.close();
}

//Display image from file
int print_img(fs::FS &fs, String filename, int x, int y)
{
    File f = fs.open(filename, "r");
    if (!f)
    {
        Serial.println("Failed to open file for reading");
        f.close();
        return 0;
    }

    f.seek(54);
    int X = x;
    int Y = y;
    uint8_t RGB[3 * X];
    for (int row = 0; row < Y; row++)
    {
        f.seek(54 + 3 * X * row);
        f.read(RGB, 3 * X);
        lcd.pushImage(0, row, X, 1, (lgfx::rgb888_t *)RGB);
    }

    f.close();
    return 0;
}

//Display image from file
//BMP is 32 bit...
int print_img_small(fs::FS &fs, String filename, int x, int y, int w, int h)
{
    File f = fs.open(filename, "r");
    if (!f)
    {
        Serial.println("Failed to open file for reading");
        f.close();
        return 0;
    }

    f.seek(54);
    int X = w;
    int Y = h;
    uint8_t RGB[3 * X];
    uint8_t RGBA[4 * X];
    for (int row = 0; row < Y; row++)
    {
        f.seek(54 + 4 * X * row);
        f.read(RGBA, 4 * X);

        for (int i = 0; i < X; i++)
        {
            RGB[3 * i] = RGBA[4 * X - 4 * i - 4];
            RGB[3 * i + 1] = RGBA[4 * X - 4 * i - 3];
            RGB[3 * i + 2] = RGBA[4 * X - 4 * i - 2];
        }
        lcd.pushImage(x, y + row, X, 1, (lgfx::rgb888_t *)RGB);
    }

    f.close();
    return 0;
}

//transform touch screen pos
void pos_rotation(int pos[2], int rotation)
{
    if (pos[0] == -1)
        return;
    if (rotation == 0)
    {
        return;
    }
    if (rotation == 3)
    {
        int tempx = 480 - pos[1];
        int tempy = pos[0];
        pos[0] = tempx;
        pos[1] = tempy;
    }
}
