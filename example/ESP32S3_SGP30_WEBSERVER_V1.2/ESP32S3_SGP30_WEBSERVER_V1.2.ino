
#include <Wire.h>
#include <SPI.h>
#include <WiFi.h>

#include "Adafruit_SGP30.h"
#include "Adafruit_SHT31.h"
#include "S3_Parallel16_ili9488.h"
#include "ESPAsyncWebServer.h"

LGFX lcd;
#define LCD_CS 37
#define LCD_BLK 45

// Replace with your network credentials
const char* ssid = "Makerfabs";
const char* password = "20160704";

#define I2C_SDA_PIN 38
#define I2C_SCL_PIN 39

Adafruit_SHT31 sht31 = Adafruit_SHT31();
Adafruit_SGP30 sgp;

float temperature;
float humidity;
float tvoc;  //tvoc
float eCO2;  //eCO2

int TVOC = 0;
//int eCO2 = 0;
int H2 = 0;
int Ethanol = 0;


AsyncWebServer server(80);
AsyncEventSource events("/events");

unsigned long lastTime = 0;  
unsigned long timerDelay = 2100;  // send readings timer


/* return absolute humidity [mg/m^3] with approximation formula
 * @param temperature [°C]
 * @param humidity [%RH]
 */
uint32_t getAbsoluteHumidity(float temperature, float humidity)
{
    // approximation formula from Sensirion SGP30 Driver Integration chapter 3.15
    const float absoluteHumidity = 216.7f * ((humidity / 100.0f) * 6.112f * exp((17.62f * temperature) / (243.12f + temperature)) / (273.15f + temperature)); // [g/m^3]
    const uint32_t absoluteHumidityScaled = static_cast<uint32_t>(1000.0f * absoluteHumidity);                                                                // [mg/m^3]
    return absoluteHumidityScaled;
}

void sensor_init()
{
    Serial.println("Check Sensor");

    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    Wire.setClock(100000UL);

    if (!sht31.begin(0x44))
    {
        Serial.println("SHT31 not found.");
    }
    delay(100);

    if (!sgp.begin())
    {
        Serial.println("SGP30 not found.");
    }
}

void sensor_display()
{
    int x_pos = 160;
    int y = 1;

    lcd.fillRect(160, 60, 120, 160, TFT_BLACK);
    lcd.setCursor(x_pos, 60 + 20 * y++);
    lcd.println(humidity);
    lcd.setCursor(x_pos, 60 + 20 * y++);
    lcd.println(temperature);
    lcd.setCursor(x_pos, 60 + 20 * y++);
    lcd.println(TVOC);
    lcd.setCursor(x_pos, 60 + 20 * y++);
    lcd.println(eCO2);
    lcd.setCursor(x_pos, 60 + 20 * y++);
    lcd.println(H2);
    lcd.setCursor(x_pos, 60 + 20 * y++);
    lcd.println(Ethanol);
    lcd.setCursor(x_pos, 60 + 20 * y++);
}

void getSGP30Readings(){
  // Tell SGP30 to begin measurement.
  unsigned long endTime = 1;//bme.beginReading();

    humidity = sht31.readHumidity();
    temperature = sht31.readTemperature();

    if (!isnan(temperature))
    { // check if 'is not a number'
        Serial.print("Temp *C = ");
        Serial.println(temperature);        
    }
    else
    {
        Serial.println("Failed to read temperature");
    }

    if (!isnan(humidity))
    { // check if 'is not a number'
        Serial.print("Hum. % = ");
        Serial.println(humidity);
    }
    else
    {
        Serial.println("Failed to read humidity");
    }

    // If you have a temperature / humidity sensor, you can set the absolute humidity to enable the humditiy compensation for the air quality signals
    // float temperature = 22.1; // [°C]
    // float humidity = 45.2; // [%RH]
    sgp.setHumidity(getAbsoluteHumidity(temperature, humidity));

    if (!sgp.IAQmeasure())
    {
        Serial.println("Measurement failed");
        return;
    }
    Serial.print("TVOC ");
    Serial.print(TVOC = sgp.TVOC);
    Serial.print(" ppb\t");
    Serial.print("eCO2 ");
    Serial.print(eCO2 = sgp.eCO2);
    Serial.println(" ppm");

    if (!sgp.IAQmeasureRaw())
    {
        Serial.println("Raw Measurement failed");
        return;
    }
    //Serial.print("Raw H2 ");
    //Serial.print(H2 = sgp.rawH2);
    //Serial.print(" \t");
    //Serial.print("Raw Ethanol ");
    //Serial.print(Ethanol = sgp.rawEthanol);
    //Serial.println("");
 
    tvoc = TVOC;
}

String processor(const String& var){
  getSGP30Readings();
  //Serial.println(var);
  if(var == "TEMPERATURE"){
    return String(temperature);
  }
  else if(var == "HUMIDITY"){
    return String(humidity);
  }
  else if(var == "TVOC"){
    return String(tvoc);
  }
 else if(var == "GAS"){
    return String(eCO2);
  }
}

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>SGP30 Web Server</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
  <link rel="icon" href="data:,">
  <style>
    html {font-family: Arial; display: inline-block; text-align: center;}
    p {  font-size: 1.2rem;}
    body {  margin: 0;}
    .topnav { overflow: hidden; background-color: #4B1D3F; color: white; font-size: 1.7rem; }
    .content { padding: 20px; }
    .card { background-color: white; box-shadow: 2px 2px 12px 1px rgba(140,140,140,.5); }
    .cards { max-width: 700px; margin: 0 auto; display: grid; grid-gap: 2rem; grid-template-columns: repeat(auto-fit, minmax(300px, 1fr)); }
    .reading { font-size: 2.8rem; }
    .card.temperature { color: #0e7c7b; }
    .card.humidity { color: #17bebb; }
    .card.tvoc { color: #d62246; }
    .card.gas { color: #3fca6b; }
  </style>
</head>
<body>
  <div class="topnav">
    <h3>R&D Office Environment Monitor</h3>
  </div>
  <div class="content">
    <div class="cards">
      <div class="card temperature">
        <h4><i class="fas fa-thermometer-half"></i> TEMPERATURE</h4><p><span class="reading"><span id="temp">%TEMPERATURE%</span> &deg;C</span></p>
      </div>
      <div class="card humidity">
        <h4><i class="fas fa-tint"></i> HUMIDITY</h4><p><span class="reading"><span id="hum">%HUMIDITY%</span> &percnt;</span></p>
      </div>
      <div class="card tvoc">
        <h4><i class="fas fa-angle-double-down"></i> TVOC</h4><p><span class="reading"><span id="tvoc">%TVOC%</span> ppb</span></p>
      </div>
      <div class="card gas">
        <h4><i class="fas fa-wind"></i> eCO2</h4><p><span class="reading"><span id="gas">%GAS%</span> ppm</span></p>
      </div>
    </div>
  </div>
<script>
if (!!window.EventSource) {
 var source = new EventSource('/events');
 
 source.addEventListener('open', function(e) {
  console.log("Events Connected");
 }, false);
 source.addEventListener('error', function(e) {
  if (e.target.readyState != EventSource.OPEN) {
    console.log("Events Disconnected");
  }
 }, false);
 
 source.addEventListener('message', function(e) {
  console.log("message", e.data);
 }, false);
 
 source.addEventListener('temperature', function(e) {
  console.log("temperature", e.data);
  document.getElementById("temp").innerHTML = e.data;
 }, false);
 
 source.addEventListener('humidity', function(e) {
  console.log("humidity", e.data);
  document.getElementById("hum").innerHTML = e.data;
 }, false);
 
 source.addEventListener('tvoc', function(e) {
  console.log("tvoc", e.data);
  document.getElementById("tvoc").innerHTML = e.data;
 }, false);
 
 source.addEventListener('gas', function(e) {
  console.log("gas", e.data);
  document.getElementById("gas").innerHTML = e.data;
 }, false);
}
</script>
</body>
</html>)rawliteral";

void setup() {
    Serial.begin(115200);

    Serial.println("Hello, Makerfabs!");

    pinMode(LCD_CS, OUTPUT);
    pinMode(LCD_BLK, OUTPUT);

    digitalWrite(LCD_CS, LOW);
    digitalWrite(LCD_BLK, HIGH);

    lcd.init();
    lcd.fillScreen(TFT_BLACK);

    lcd.setTextSize(4);
    lcd.setTextColor(TFT_WHITE);

    // lcd.fillRect(80, 160, 160, 160, TFT_BLACK);
    lcd.setCursor(10, 10);
    lcd.println("S3 Mabee Co2");
    
    //lcd.println("Hello World!");
    lcd.setTextSize(2);

    int x_pos = 10;
    int y = 1;

    lcd.setCursor(x_pos, 60 + 20 * y++);
    lcd.println("Humidity");
    lcd.setCursor(x_pos, 60 + 20 * y++);
    lcd.println("Temperature");
    lcd.setCursor(x_pos, 60 + 20 * y++);
    lcd.setTextColor(TFT_RED);
    lcd.println("TVOC");
    lcd.setCursor(x_pos, 60 + 20 * y++);
    lcd.setTextColor(TFT_GREEN);
    lcd.println("eCO2");
    lcd.setCursor(x_pos, 60 + 20 * y++);
    lcd.setTextColor(TFT_WHITE);
    lcd.println("Raw H2");
    lcd.setCursor(x_pos, 60 + 20 * y++);
    lcd.println("Raw Ethanol");

    x_pos = 280;
    y = 1;
    lcd.setCursor(x_pos, 60 + 20 * y++);
    lcd.println("%");
    lcd.setCursor(x_pos, 60 + 20 * y++);
    lcd.println("C");
    lcd.setCursor(x_pos, 60 + 20 * y++);
    lcd.println("ppb");
    lcd.setCursor(x_pos, 60 + 20 * y++);
    lcd.println("ppm");

    sensor_init();

  
    // Set the device as a Station and Soft Access Point simultaneously
    WiFi.mode(WIFI_AP_STA);
    
    // Set device as a Wi-Fi Station
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      Serial.println("Setting as a Wi-Fi Station..");
    }
    Serial.print("Station IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.println();

    // Init SGP30 sensor
    sensor_init();
  
    // Handle Web Server
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/html", index_html, processor);
    });
  
    // Handle Web Server Events
    events.onConnect([](AsyncEventSourceClient *client){
      if(client->lastId()){
        Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
      }
      // send event with message "hello!", id current millis
      // and set reconnect delay to 1 second
      client->send("hello!", NULL, millis(), 10000);
    });
    server.addHandler(&events);
    server.begin();
}

int counter = 0;

void loop() {
    counter++;
    if (counter == 30)
    {
        counter = 0;
        uint16_t TVOC_base, eCO2_base;

        if (!sgp.getIAQBaseline(&eCO2_base, &TVOC_base))
        {
            Serial.println("Failed to get baseline readings");
            return;
        }
        //Serial.print("****Baseline values: eCO2: 0x");
        //Serial.print(eCO2_base, HEX);
        //Serial.print(" & TVOC: 0x");
        //Serial.println(TVOC_base, HEX);
    }
  
    if ((millis() - lastTime) > timerDelay) {
      getSGP30Readings();
      Serial.printf("Temperature = %.2f ºC \n", temperature);
      Serial.printf("Humidity = %.2f % \n", humidity);
      Serial.printf("TVOC = %.2f ppb \n", tvoc);
      Serial.printf("eCO2 = %.2f ppm \n", eCO2);
      Serial.println();
  
      // Send Events to the Web Server with the Sensor Readings
      events.send("ping",NULL,millis());
      events.send(String(temperature).c_str(),"temperature",millis());
      events.send(String(humidity).c_str(),"humidity",millis());
      events.send(String(tvoc).c_str(),"tvoc",millis());
      events.send(String(eCO2).c_str(),"gas",millis());
      
      lastTime = millis();
    }
      //getSGP30Readings();
    sensor_display();
    delay(1000);
    
}
