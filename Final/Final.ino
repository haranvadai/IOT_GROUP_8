#include <WiFi.h>
//#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <Arduino_GFX_Library.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <time.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <Arduino.h>
#include <driver/i2s.h>
#include <math.h>
#include <cstring>


//keyboard stuff
#include <Adafruit_GFX.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_ILI9341.h>
//#include <Adafruit_STMPE610.h>
//#include <avr/pgmspace.h>

// This is calibration data for the raw touch data to the screen coordinates

#define IsWithin(x, a, b) ((x>=a)&&(x<=b))
#define TS_MINX 142
#define TS_MINY 125
#define TS_MAXX 3871
#define TS_MAXY 3727
//#define STMPE_CS 33
//Adafruit_STMPE610 ts = Adafruit_STMPE610(STMPE_CS);
const char Mobile_KB[3][13] PROGMEM = {
  {0, 13, 10, 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P'},
  {1, 12, 9, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L'},
  {3, 10, 7, 'Z', 'X', 'C', 'V', 'B', 'N', 'M'},
};

const char Mobile_NumKeys[3][13] PROGMEM = {
  {0, 13, 10, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0'},
  {0, 13, 10, '-', '/', ':', ';', '(', ')', '$', '&', '@', '"'},
  {5, 8, 5, '.', '\,', '?', '!', '\''}
};

const char Mobile_SymKeys[3][13] PROGMEM = {
  {0, 13, 10, '[', ']', '{', '}', '#', '%', '^', '*', '+', '='},
  {4, 9, 6, '_', '\\', '|', '~', '<', '>'}, //4
  {5, 8, 5, '.', '\,', '?', '!', '\''}
};

const char textLimit = 25;
char MyBuffer[textLimit];
char keyboardInput[textLimit];
char ssid[textLimit];
char password[textLimit];
char area2[textLimit];

#define DHTPIN 4
#define DHTTYPE DHT11
DHT_Unified dht(DHTPIN, DHTTYPE);

#define w 320
#define h 240

// Replace with your Wi-Fi credentials
// const char* ssid = "";
// const char* password = "";

// API URLs
const char* time_api = "https://timeapi.io/api/time/current/zone?timeZone=Asia%2FJerusalem";
const char* weather_api = "https://goweather.herokuapp.com/weather/";

int weatherLocation = 0;

// Define ILI9341 connections and rotation
#define TFT_DC 2
#define TFT_CS 15
#define TFT_RST 0
#define TFT_SCK 14
#define TFT_MOSI 13
#define TFT_MISO 12
#define ROTATION 1

#define TS_CS 33  //7
#define SD_CS 5
#define ROTATION 1

// Initialize the display
//Arduino_ESP32SPI bus = Arduino_ESP32SPI(TFT_DC, TFT_CS, TFT_SCK, TFT_MOSI, TFT_MISO);
//Arduino_ILI9341 tft = Arduino_ILI9341(&bus, TFT_RST);
TFT_eSPI tft = TFT_eSPI();

#define XPT2046_IRQ 23   // T_IRQ
#define XPT2046_MOSI 32  // T_DIN
#define XPT2046_MISO 22  // T_OUT
#define XPT2046_CLK 25   // T_CLK
#define XPT2046_CS 33    // T_CS

SPIClass touchscreenSPI = SPIClass(VSPI);
XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);

int brightness = 1, previousBrightness = 1;
#define BRIGHT_LOW 0
#define BRIGHT_HIGH 1

int x = 0, y = 0, z = 0;
unsigned long lastTouchTime = 0; 
#define SLEEP_TIMEOUT 60000

unsigned long lastAlert = -30000; 
#define ALERT_TIMEOUT 30000

#define BACKLIGHT_PIN 21  // Change to the correct LED pin on your display
#define PWM_CHANNEL 0     // ESP32 has multiple PWM channels
#define PWM_FREQ 5000     // Frequency in Hz
#define PWM_RESOLUTION 8  // 8-bit resolution (0-255)

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 7200;
const int   daylightOffset_sec = 3600;

String outsideTemp = "0", insideTemp = "0", apiData = "";

int theme = 1;

#define THEME_1 1
#define THEME_2 2
#define THEME_3 3
#define THEME_4 4

int area = 1;

#define HAIFA 1
#define TELAVIV 2
#define EILAT 3

int screen = 0;

#define MAIN 0
#define SECOND 1
#define SETTINGS 2
#define SLEEP 3
#define RED_AREA_PICKER 4


int offlineScreen = 0;

#define MAIN 0
#define KEYBOARD 1


int keyboardDes = 0;

#define SSID 0
#define PASS 1
#define AREA 2

// I2S configuration constants
#define I2S_NUM         (0) // I2S port number
#define I2S_SAMPLE_RATE (44100)
#define I2S_BITS        (16)
#define I2S_CHANNELS    (2)
#define I2S_FREQUENCY   (500)
#define I2S_BEEP_DURATION (200)

int muted = 1;

#define OFF 0
#define ON 1

int retry = 0;


void setup() {
  Serial.begin(115200);

  pinMode(TFT_CS, OUTPUT);
  digitalWrite(TFT_CS, HIGH);
  pinMode(21, OUTPUT);  //was 7
  digitalWrite(21, LOW);  //was 7
  pinMode(TS_CS, OUTPUT);  //was 7
  digitalWrite(TS_CS, HIGH);  //was 7
  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);

  // Brightness
  //ledcSetup(PWM_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
  //ledcAttachPin(BACKLIGHT_PIN, PWM_CHANNEL);
  //setBrightness(255); 

  //2 Lines below can be ignored, they're part of a bigger project
  pinMode(17, OUTPUT);
  digitalWrite(17, LOW);


  // Connect to Wi-Fi
  Serial.print("Connecting to Wi-Fi");
  WiFi.begin(ssid, password);
  // while (WiFi.status() != WL_CONNECTED) {
  //   delay(500);
  //   Serial.print(".");
  // }
  //Serial.println("\nWi-Fi connected.");

  //touch screen setup
  touchscreenSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  touchscreen.begin(touchscreenSPI);
  touchscreen.setRotation(ROTATION);

  // Initialize the display
  tft.begin();
  tft.setRotation(ROTATION);
  fillScreen();

  keyboardInput[0] = '\0';
  ssid[0] = '\0';
  password[0] = '\0';

  // Initialize I2S configuration
  i2s_config_t i2s_config = {
      .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX), // Master transmitter
      .sample_rate = I2S_SAMPLE_RATE,
      .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
      .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
      .communication_format = I2S_COMM_FORMAT_I2S_MSB,
      .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1, // Interrupt level 1
      .dma_buf_count = 8,
      .dma_buf_len = 64,
      .use_apll = false  // Set to true if using APLL (ESP32-S2 specific)
  };

  i2s_pin_config_t pin_config = {
      .bck_io_num = 18,   // BCK pin
      .ws_io_num = 5,    // LRCK pin
      .data_out_num = 19, // DATA pin
      .data_in_num = I2S_PIN_NO_CHANGE // Not used
  };

  // Install and start I2S driver
  i2s_driver_install((i2s_port_t)I2S_NUM, &i2s_config, 0, NULL);
  i2s_set_pin((i2s_port_t)I2S_NUM, &pin_config);
  i2s_zero_dma_buffer((i2s_port_t)I2S_NUM);

  beep();

  lastTouchTime = millis();

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  xTaskCreate(mainLoop, "mainLoop", 8192, nullptr, 5, nullptr);
  xTaskCreate(updateTemp, "updateTemp", 8192, nullptr, 5, nullptr);
  xTaskCreate(updateAlert, "updateAlert", 8192, nullptr, 5, nullptr);

}

void loop() 
{

}

void mainLoop(void*)
{
  while(1)
  {
    if (WiFi.status() == WL_CONNECTED) 
    {
      offlineScreen = MAIN;
      if (touchscreen.tirqTouched() && touchscreen.touched()) 
      {
        lastTouchTime = millis();
        bool wakeFlag = false;
        if(screen == SLEEP)
        {
          screen = MAIN;
          wakeFlag = true;
          brightness = previousBrightness;
        }
    
        if(!wakeFlag)
        {
        // Get Touchscreen points
          TS_Point p = touchscreen.getPoint();
        // Calibrate Touchscreen points with map function to the correct width and height
          x = map(p.x, 200, 3700, 1, w);
          y = map(p.y, 240, 3800, 1, h);
          z = p.z;
          Serial.println("x:"+ String(x) + " y:" + String(y)+ " z =" +String(z));
          if(screen == SETTINGS)
          {
            if(x <= w-65 && x >= w-150 && y <= h-50 && y >= h-90)
              theme = THEME_1;
            else if(x <= w-165 && x >= w-250 && y <= h-50 && y >= h-90)
              theme = THEME_2;
            else if(x <= w-65 && x >= w-150 && y <= h-110 && y >= h-150)
              theme = THEME_3;
            else if(x <= w-165 && x >= w-250 && y <= h-110 && y >= h-150)
              theme = THEME_4;
            else if(x <= w-65 && x >= w-150 && y <= h-205 && y >= h-230)
              brightness = BRIGHT_LOW;
            else if(x <= w-165 && x >= w-250 && y <= h-205 && y >= h-230)
              brightness = BRIGHT_HIGH;
          }
          else if(screen == RED_AREA_PICKER)
          {
            // if(x <= w-70 && x >= w-150 && y <= h-50 && y >= h-70)
            // {
            //   area = HAIFA;
            //   outsideTemp = "Fetching...";
            // }
            // else if(x <= w-70 && x >= w-150 && y <= h-80 && y >= h-100)
            // {
            //   area = TELAVIV;
            //   outsideTemp = "Fetching...";
            // }
            // else if(x <= w-70 && x >= w-150 && y <= h-110 && y >= h-130)
            // {
            //   area = EILAT;
            //   outsideTemp = "Fetching...";
            // }
            if(x <= w-65 && x >= w-150 && y <= h-80 && y >= h-105)
              muted = OFF;
            else if(x <= w-165 && x >= w-250 && y <= h-80 && y >= h-105)
              muted = ON;
            
            else if(x <= w-65 && x >= w-250 && y <= h-105 && y >= h-145)
              keyboardDes = AREA;
              outsideTemp = "Fetching...";
          }

          if(x >= w - 35)
          {
            if(screen == MAIN)
            {
              beep();
              screen = SLEEP;
              previousBrightness = brightness;
              Serial.println("updated to sleep");
            }
            if(screen == SECOND)
            {
              beep();
              screen = MAIN;
              Serial.println("updated to main");
            }
            else if(screen == SETTINGS)
            {
              beep();
              //screen = SECOND;
              screen = MAIN;
              Serial.println("updated to second");
            }
            else if(screen == RED_AREA_PICKER && keyboardDes != AREA)
            {
              beep();
              screen = SETTINGS;
              Serial.println("updated to settings");
            }
          }
          else if(x <= 50)
          {
            if(screen == MAIN)
            {
              beep();
              //screen = SECOND;
              screen = SETTINGS;
              Serial.println("updated to second");
            }
            else if(screen == SECOND)
            {
              beep();
              screen = SETTINGS;
              Serial.println("updated to settings");
            }
            else if(screen == SETTINGS)
            {
              beep();
              screen = RED_AREA_PICKER;
              Serial.println("updated to read alert picker");
            }
          }
        }
      } 
    
      checkAndUpdateSleepMode();
      // Format screen for display
      if(screen == MAIN)
      {
        fillScreen(); // Black background
        printHeader();
        printComsScreen();
        //setBrightness(255); 
      }
      else if(screen == SECOND)
      {
        fillScreen();
        printHeader();
      }
      else if(screen == SETTINGS)
      {
        fillScreen();
        printHeader();
        printSettingsScreen();
      }
      else if(screen == SLEEP)
      {
        brightness = BRIGHT_LOW;
        //fillScreen();
        printSleepScreen();
      }
      else if(screen == RED_AREA_PICKER)
      {
        fillScreen();
        printHeader();
        if(keyboardDes == AREA)
        {
          if(keyboardInput[0] == '\0')
          {
            tft.fillScreen(ILI9341_BLUE);

            tft.setTextColor(0xffff, 0xf000);
            tft.setTextSize(2);
            tft.drawCentreString("Area", w/2, 35, 1);

            MakeKB_Button(Mobile_KB);
            //if (!touchscreen.bufferEmpty())
            //{
              GetKeyPress(MyBuffer);
            //}
          }
          else
          {
            if(keyboardDes == AREA)
            {
              strcpy(area2, keyboardInput);
              keyboardInput[0] = '\0';
              keyboardDes = SSID;
            }
          }
        }
        else
        {
          printRedAreaPickerScreen();
        }
      }
    } 
    else 
    {
      Serial.println("Wi-Fi disconnected!");
      printOfflineScreen();
    }

    vTaskDelay(500/ portTICK_PERIOD_MS); // Update every second
  }
}

void updateTemp(void *)
{
  while(1)
  {
    outsideTemp = readOutsideTemp();
    insideTemp = readInsideTemp();
    vTaskDelay(10000 / portTICK_PERIOD_MS);
  }
}

// void updateTouch(void *)
// {
//   while(1)
//   {
//     if (touchscreen.tirqTouched() && touchscreen.touched()) 
//     {
//     // Get Touchscreen points
//       TS_Point p = touchscreen.getPoint();
//     // Calibrate Touchscreen points with map function to the correct width and height
//       x = map(p.x, 200, 3700, 1, w);
//       y = map(p.y, 240, 3800, 1, h);
//       z = p.z;
//       if(x >= w - 70)
//         screen = MAIN;
//       else if(x <= 70)
//         screen = SECOND;
//     } 
//     vTaskDelay(500 / portTICK_PERIOD_MS);
//   }
// }

float readInsideTemp()
{
  float t;
  // Get temperature event and print its value.
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature) || event.temperature >= 1000 || event.temperature <= -1000) {
    return t;
  }
  else 
  {
    t = event.temperature;
  }

  return t;
}

String readAPI() {

  HTTPClient http;
  //String time = "Error";

  
  http.begin("https://haranvadai.github.io/IOT_GR_08/");
  int httpResponseCode = http.GET();
  String payload = "";
  if (httpResponseCode == 200) 
  {
    payload = http.getString();
  } else {
    return apiData;
  }

  http.end();
  return payload;
 }


// Draw header bar
void printHeader() {
  if(brightness == BRIGHT_HIGH)
    tft.fillRect(0, 0, w, 40, 0x07E0); // Green bar
  else
    tft.fillRect(0, 0, w, 40, 0x03a0); // Green bar
  tft.setTextSize(2);
  if(brightness == BRIGHT_HIGH)
    tft.setTextColor(0xFFFF); // White text
  else
    tft.setTextColor(0x9cf3);
  tft.setCursor(10, 10);
  if(screen == MAIN)
    tft.drawCentreString("Main Screen", w/2, 5, 2);
    //tft.print("Main Screen");
  else if (screen == SECOND)
    //tft.print("Second Screen");
    tft.drawCentreString("Second Screen", w/2, 5, 2);
  else if (screen == SETTINGS || screen == RED_AREA_PICKER)
    tft.drawCentreString("Settings", w/2, 5, 2);
    //tft.print("SETTINGS");
  else if (screen == SETTINGS || screen == RED_AREA_PICKER)
    tft.drawCentreString("Settings", w/2, 5, 2);

  //Red alert
  if(millis() - lastAlert <= ALERT_TIMEOUT)
  {
    tft.setTextSize(3);
    if(brightness == BRIGHT_HIGH)
      tft.setTextColor(0xF800);
    else
      tft.setTextColor(0x9000); 
    tft.setCursor(w - 20, 10);
    tft.print("!");
  }
}

String readOutsideTemp(){

  HTTPClient http;
  String temperature = "Fetching...";
  String temp_api = weather_api;
  // if(area == HAIFA)
  //   temp_api += "HAIFA";
  // else if(area == TELAVIV)
  //   temp_api += "TEL_AVIV";
  // else if(area == EILAT)
  //   temp_api += "eilat";
  temp_api += area2;

  http.begin(temp_api);
  int httpResponseCode = http.GET();

  if (httpResponseCode == 200) {
    String payload = http.getString();
    int tempStart = payload.indexOf("temperature") + 14;
    int tempEnd = payload.indexOf("°");
    temperature = payload.substring(tempStart, tempEnd) + "C";
    //Serial.println(payload);
  } 
  else 
  {
    return temperature;
  }

  http.end();
  return temperature;
}


void printLocalTime(){
  tft.setTextSize(2);
  updateTextColor();
  struct tm timeinfo;

  if(!getLocalTime(&timeinfo))
  {
    // add local time stamp for clock maybe for no wifi.
    Serial.println("Failed to obtain time");
    return;
  }
  if(theme == THEME_1)
  {
    if(brightness == HIGH)
      tft.fillRect(10, 60, w, 80, 0x1866);
    else
      tft.fillRect(10, 60, w, 80, 0x1025);
  }
  else if(theme == THEME_2)
  {
    if(brightness == HIGH)
      tft.fillRect(10, 60, w, 80, 0xFFFF);
    else
      tft.fillRect(10, 60, w, 80, 0x9cf3);
  }
  else if(theme == THEME_3)
  {
    if(brightness == HIGH)
      tft.fillRect(10, 60, w, 80, 0x2862);
    else
      tft.fillRect(10, 60, w, 80, 0x1821);
  }
  else if(theme == THEME_4)
  {
    tft.fillRect(10, 60, w, 80, 0x0000);
  }

  tft.setCursor(10, 60);
  tft.print("Date: ");
  tft.print(&timeinfo, "%d"); //day
  tft.print(".");
  if(timeinfo.tm_mon+1 <= 9)
  {
    tft.print("0");
    tft.print(timeinfo.tm_mon+1); //month
  }
  else
    tft.print((timeinfo.tm_mon+1)); //month
  tft.print(".");
  tft.println(&timeinfo, "%Y"); //year

  tft.setCursor(10, 100);
  tft.print("Time: ");
  tft.print(&timeinfo, "%H"); //hour
  tft.print(":");
  tft.print(&timeinfo, "%M"); //mins
  tft.print(":");
  tft.println(&timeinfo, "%S"); //secs
}

void printTemp()
{
  tft.setTextSize(2);
  updateTextColor();

  // Display temperature
  tft.setCursor(10, 140);
  tft.print("Temp Outside: ");
  tft.print(outsideTemp);

  tft.setCursor(10, 180);
  tft.print("Temp inside: ");
  tft.print(insideTemp);
  tft.print(" C");
}

void printAPI()
{
  tft.setTextSize(2);
  updateTextColor();

  tft.setCursor(10, 220);
  tft.print("API: ");
  tft.print(apiData);
}

// void setBrightness(int brightness) 
// {
//   //tft.writecommand(0x51); // Write Brightness Control
//   //tft.writedata(brightness);
//   ledcWrite(PWM_CHANNEL, brightness);
// }

void printComsScreen()
{
  printLocalTime();
  printTemp();
  printAPI();
}

void printSettingsScreen()
{
  tft.setTextSize(2);
  // Draw color buttons
  if(brightness == BRIGHT_HIGH)
  {
    tft.fillRect(65, 50, 85, 40, 0x07FF); 
    tft.fillRect(66, 51, 83, 38, 0x1866); 
    tft.setTextColor(0x07FF);
  }
  else
  {
    tft.fillRect(65, 50, 85, 40, 0x04D3);
    tft.fillRect(66, 51, 83, 38, 0x1025);
    tft.setTextColor(0x04D3); 
  }
  tft.drawCentreString("Theme 1", 110, 65, 1);

  if(brightness == BRIGHT_HIGH)
  {
    tft.fillRect(65, 110, 85, 40, 0xF800); 
    tft.fillRect(66, 111, 83, 38, 0x2862); 
    tft.setTextColor(0xF800);
  }
  else
  {
    tft.fillRect(65, 110, 85, 40, 0x9000);
    tft.fillRect(66, 111, 83, 38, 0x1821);
    tft.setTextColor(0x9000);
  }
  tft.drawCentreString("Theme 3", 110, 125, 1);

  if(brightness == BRIGHT_HIGH)
  {
    tft.fillRect(165, 50, 85, 40, 0x0000); 
    tft.fillRect(166, 51, 83, 38, 0xFFFF); 
    tft.setTextColor(0x0000);
  }
  else
  {
    tft.fillRect(165, 50, 85, 40, 0x0000); 
    tft.fillRect(166, 51, 83, 38, 0x9cf3); 
    tft.setTextColor(0x0000);
  }
  tft.drawCentreString("Theme 2", 210, 65, 1);

  if(brightness == BRIGHT_HIGH)
  {
    tft.fillRect(165, 110, 85, 40, 0xD81B); 
    tft.fillRect(166, 111, 83, 38, 0x0000);
    tft.setTextColor(0xD81B);
  }
  else
  {
    tft.fillRect(165, 110, 85, 40, 0x580b); 
    tft.fillRect(166, 111, 83, 38, 0x0000);
    tft.setTextColor(0x580b);
  }
  tft.drawCentreString("Theme 4", 210, 125, 1);

  tft.setTextSize(3);
  updateTextColor();
  tft.setCursor(65, 175);
  tft.println("Brightness");
  tft.setTextSize(2);
  tft.setCursor(65, 205);
  if(brightness == BRIGHT_LOW)
    tft.println("Low  X");
  else
    tft.println("Low");
  tft.setCursor(165, 205);
  if(brightness == BRIGHT_HIGH)
    tft.println("High  X");
  else
    tft.println("High");

}

void printSleepScreen()
{
  struct tm timeinfo;
  String time = "Fetching time...";
  if(!getLocalTime(&timeinfo))
  {
    // add local time stamp for clock maybe for no wifi.
    Serial.println("Failed to obtain time");
  }
  else
  {
    time = "";
    if(timeinfo.tm_hour <= 9)
      time +="0";
    time += String(timeinfo.tm_hour) + ":";
    if(timeinfo.tm_min <= 9)
      time +="0";
    time += String(timeinfo.tm_min) + ":";
    if(timeinfo.tm_sec <= 9)
      time +="0";
    time += String(timeinfo.tm_sec);
  }
  fillScreen();

  //Red alert
  if(millis() - lastAlert <= ALERT_TIMEOUT)
  {
    tft.setTextSize(50);
    if(brightness == BRIGHT_HIGH)
      tft.setTextColor(0xF800);
    else
      tft.setTextColor(0x9000); 
    tft.setCursor(w/2 - 10, h/2 - 80);
    tft.print("!");
    tft.setCursor(w/2 - 10, h/2 + 40);
    tft.print("!");
  }

  tft.setTextSize(2);
  updateTextColor();
  tft.drawCentreString(time, w/2, h/2-15, 4);


}

void checkAndUpdateSleepMode() {
  if (screen != SLEEP && millis() - lastTouchTime >= SLEEP_TIMEOUT) 
  {
    previousBrightness = brightness;
    screen = SLEEP;
  }
}

void fillScreen()
{
  if(theme == THEME_1)
  {
    if(brightness == BRIGHT_LOW)
      tft.fillScreen(0x1025); 
    else
      tft.fillScreen(0x1866); 
  }
  else if(theme == THEME_2)
  {
    if(brightness == BRIGHT_LOW)
      tft.fillScreen(0x9cf3);
    else
      tft.fillScreen(0xFFFF);
  }
  else if(theme == THEME_3)
  {
    if(brightness == BRIGHT_LOW)
      tft.fillScreen(0x1821); 
    else
      tft.fillScreen(0x2862); 
  }
  else if(theme == THEME_4)
  {
    tft.fillScreen(0x0000); 
  }
}

void updateTextColor()
{
  if(theme == THEME_1) // cyan
  {
    if(brightness == BRIGHT_LOW)
      tft.setTextColor(0x04D3);
    else
      tft.setTextColor(0x07FF);
  }
  else if(theme == THEME_2)
  {
      tft.setTextColor(0x0000);
  }
  else if(theme == THEME_3)
  {
    if(brightness == BRIGHT_LOW)
      tft.setTextColor(0x9000);
    else
      tft.setTextColor(0xF800);
  }
  else if(theme == THEME_4)
  {
    if(brightness == BRIGHT_LOW)
      tft.setTextColor(0x580b);
    else
      tft.setTextColor(0xD81B);
  }
}

void updateAlert(void*)
{
  while(1)
  {
    apiData = readAPI();
    if(area2[0] != '\0' && apiData.substring(0, strlen(area2)) == area2)
       lastAlert = millis();
    // if(apiData.substring(0, 5) == "haifa" && area == HAIFA)
    //   lastAlert = millis();
    // else if(apiData.substring(0, 8) == "tel aviv" && area == TELAVIV)
    //   lastAlert = millis();
    // else if(apiData.substring(0, 5) == "eilat" && area == EILAT)
    //   lastAlert = millis();
    vTaskDelay(1000/ portTICK_PERIOD_MS); // Update every second
  }
}

void printRedAreaPickerScreen()
{
  tft.setTextSize(2);
  updateTextColor();

  tft.setTextSize(3);
  tft.setCursor(65, 50);
  tft.println("Sound");
  tft.setTextSize(2);
  tft.setCursor(65, 80);
  if(muted == OFF)
    tft.println("Off  X");
  else
    tft.println("Off");
  tft.setCursor(165, 80);
  if(muted == ON)
    tft.println("On  X");
  else
    tft.println("On");

  tft.setTextSize(2);
  updateTextColor();
  if(brightness == BRIGHT_HIGH)
  {
    if(theme == THEME_1)
    {
      tft.fillRect(65, 105, 180, 40, 0x04d3); 
      tft.fillRect(66, 106, 178, 38, 0x1866); 
    }
    else if(theme == THEME_2)
    {
      tft.fillRect(65, 105, 180, 40, 0x0000); 
      tft.fillRect(66, 106, 178, 38, 0xFFFF); 
    }
    else if(theme == THEME_3)
    {
      tft.fillRect(65, 105, 180, 40, 0x9000); 
      tft.fillRect(66, 106, 178, 38, 0x2862); 
    }
    else if(theme == THEME_4)
    {
      tft.fillRect(65, 105, 180, 40, 0x580b); 
      tft.fillRect(66, 106, 178, 38, 0x0000);
    }
  }
  else
  {
    if(theme == THEME_1)
    {
      tft.fillRect(65, 105, 180, 40, 0x07ff);
      tft.fillRect(66, 106, 178, 38, 0x1025);
    }
    else if(theme == THEME_3)
    {
      tft.fillRect(65, 105, 180, 40, 0xf800);
      tft.fillRect(66, 106, 178, 38, 0x1821);
    }
    else if(theme == THEME_2)
    {
      tft.fillRect(65, 105, 180, 40, 0x0000); 
      tft.fillRect(66, 106, 178, 38, 0x9cf3); 
    }
    else if(theme == THEME_4)
    {
      tft.fillRect(65, 105, 180, 40, 0xd81b); 
      tft.fillRect(66, 106, 178, 38, 0x0000);
    }
  }
  tft.drawCentreString("Area", 155, 117, 1);

  tft.setTextSize(3);
  tft.setCursor(65, 155);
  tft.println("Area:");
  tft.setTextSize(2);
  tft.setCursor(65, 185);
  tft.println(area2);

  // tft.setCursor(70, 50);
  // if(area == HAIFA)
  //   tft.println("Haifa  X");
  // else
  //   tft.println("Haifa");

  // tft.setCursor(70, 80);
  // if(area == TELAVIV)
  //   tft.println("Tel Aviv  X");
  // else
  //   tft.println("Tel Aviv");

  // tft.setCursor(70, 110);
  // if(area == EILAT)
  //   tft.println("Eilat  X");
  // else
  //   tft.println("Eilat");
}


void beep() {
  const int samplesPerCycle = I2S_SAMPLE_RATE / I2S_FREQUENCY; // Samples per waveform cycle
  int amplitude = 1000; // Amplitude of the waveform
  if(muted == OFF)
    amplitude = 0;

  // Create a square wave buffer
  int16_t wave[samplesPerCycle];
  for (int i = 0; i < samplesPerCycle; i++) {
    wave[i] = (i < samplesPerCycle / 2) ? amplitude : -amplitude; // High for half, low for half
  }

  unsigned long startTime = millis();
  while (millis() - startTime < I2S_BEEP_DURATION) {
    size_t bytes_written;
    i2s_write((i2s_port_t)I2S_NUM, wave, sizeof(wave), &bytes_written, portMAX_DELAY); // Send the wave to I2S
  }

  i2s_zero_dma_buffer((i2s_port_t)I2S_NUM); // Clear the I2S buffer after the beep
}



void MakeKB_Button(const char type[][13])
{
  tft.setTextSize(2);
  tft.setTextColor(0xffff, 0xf000);
  for (int y = 0; y < 3; y++)
  {
    int ShiftRight = 15 * pgm_read_byte(&(type[y][0]));
    for (int x = 3; x < 13; x++)
    {
      if (x >= pgm_read_byte(&(type[y][1]))) break;

      drawButton(15 + (30 * (x - 3)) + ShiftRight, 100 + (30 * y), 20, 25); // this will draw the button on the screen by so many pixels
      tft.setCursor(20 + (30 * (x - 3)) + ShiftRight, 105 + (30 * y));
      tft.print(char(pgm_read_byte(&(type[y][x]))));
    }
  }
  //ShiftKey
  drawButton(15, 160, 35, 25);
  tft.setCursor(27, 168);
  tft.print('^');

  //Special Characters
  drawButton(15, 190, 35, 25);
  tft.setCursor(21, 195);
  tft.print(F("SP"));

  //BackSpace
  drawButton(270, 160, 35, 25);
  tft.setCursor(276, 165);
  tft.print(F("BS"));

  //Return
  drawButton(270, 190, 35, 25);
  tft.setCursor(276, 195);
  tft.print(F("RT"));

  //Spacebar
  drawButton(60, 190, 200, 25);
  tft.setCursor(105, 195);
  tft.print(F("SPACE BAR"));
}

void drawButton(int x, int y, int w1, int h1)
{
  // grey
  tft.fillRoundRect(x - 3, y + 3, w1, h1, 3, 0x8888); //Button Shading

  // white
  tft.fillRoundRect(x, y, w1, h1, 3, 0xffff);// outter button color

  //red
  tft.fillRoundRect(x + 1, y + 1, w1 - 1 * 2, h1 - 1 * 2, 3, 0xf800); //inner button color
}

void GetKeyPress(char * textBuffer)
{
  char key = 0;
  static bool shift = false, special = false, back = false, lastSp = false, lastSh = false;
  static char bufIndex = 0;

  //if (!touchscreen.bufferEmpty())
  if (touchscreen.tirqTouched() && touchscreen.touched()) 
  {
    //ShiftKey
    if (TouchButton(15, 160, 35, 25))
    {
      shift = !shift;
      delay(200);
    }

    //Special Characters
    if (TouchButton(15, 190, 35, 25))
    {
      special = !special;
      delay(200);
    }

    if (special != lastSp || shift != lastSh)
    {
      if (special)
      {
        if (shift)
        {
          tft.fillScreen(ILI9341_BLUE);
          MakeKB_Button(Mobile_SymKeys);
        }
        else
        {
          tft.fillScreen(ILI9341_BLUE);
          MakeKB_Button(Mobile_NumKeys);
        }
      }
      else
      {
        tft.fillScreen(ILI9341_BLUE);
        MakeKB_Button(Mobile_KB);
        tft.setTextColor(0xffff, 0xf800);
      }

      if (special)
        tft.setTextColor(0x0FF0, 0xf800);
      else
        tft.setTextColor(0xFFFF, 0xf800);

      tft.setCursor(21, 195);
      tft.print(F("SP"));

      if (shift)
        tft.setTextColor(0x0FF0, 0xf800);
      else
        tft.setTextColor(0xffff, 0xf800);

      tft.setCursor(27, 168);
      tft.print('^');

      lastSh = shift;

      lastSp = special;
      lastSh = shift;
    }

    for (int y = 0; y < 3; y++)
    {
      int ShiftRight;
      if (special)
      {
        if (shift)
          ShiftRight = 15 * pgm_read_byte(&(Mobile_SymKeys[y][0]));
        else
          ShiftRight = 15 * pgm_read_byte(&(Mobile_NumKeys[y][0]));
      }
      else
        ShiftRight = 15 * pgm_read_byte(&(Mobile_KB[y][0]));

      for (int x = 3; x < 13; x++)
      {
        if (x >=  (special ? (shift ? pgm_read_byte(&(Mobile_SymKeys[y][1])) : pgm_read_byte(&(Mobile_NumKeys[y][1]))) : pgm_read_byte(&(Mobile_KB[y][1])) )) break;

        if (TouchButton(15 + (30 * (x - 3)) + ShiftRight, 100 + (30 * y), 20, 25)) // this will draw the button on the screen by so many pixels
        {
          if (bufIndex < (textLimit - 1))
          {
            delay(200);

            if (special)
            {
              if (shift)
                textBuffer[bufIndex] = pgm_read_byte(&(Mobile_SymKeys[y][x]));
              else
                textBuffer[bufIndex] = pgm_read_byte(&(Mobile_NumKeys[y][x]));
            }
            else
              textBuffer[bufIndex] = (pgm_read_byte(&(Mobile_KB[y][x])) + (shift ? 0 : ('a' - 'A')));

            bufIndex++;
          }
          break;
        }
      }
    }

    //Spacebar
    if (TouchButton(60, 190, 200, 25))
    {
      textBuffer[bufIndex++] = ' ';
      delay(200);
    }

    //BackSpace
    if (TouchButton(270, 160, 35, 25))
    {
      if ((bufIndex) > 0)
        bufIndex--;
      textBuffer[bufIndex] = '\0';
      tft.setTextColor(0, ILI9341_BLUE);
      tft.setCursor(15, 80);
      tft.print(F("                          "));
      delay(200);
    }

    //Return
    if (TouchButton(270, 190, 35, 25))
    {
      strcpy(keyboardInput, textBuffer);
      while (bufIndex > 0)
      {
        bufIndex--;
        textBuffer[bufIndex] = 0;
      }

      tft.setTextColor(0, ILI9341_BLUE);
      tft.setCursor(15, 80);
      tft.print(F("                         "));
    }
  }
  tft.setTextColor(0xffff, 0xf800);
  tft.setCursor(15, 80);
  tft.print(textBuffer);
}

byte TouchButton(int x, int y, int w1, int h1)
{
  int X, Y;
  // Retrieve a point
  TS_Point p = touchscreen.getPoint();
  // Y = map(p.y, TS_MINY, TS_MAXY, tft.height(), 0);
  // X = map(p.x, TS_MINX, TS_MAXX, tft.width(), 0);
  Y = map(p.y, TS_MINY, TS_MAXY, tft.height(), 0);
  X = map(p.x, TS_MINX + 175, 3800 +20, tft.width(), 0);

  return (IsWithin(X, x, x + w1) & IsWithin(Y, y, y + h1));
}

void printOfflineScreen()
{
  if(offlineScreen == MAIN)
  {
    if (touchscreen.tirqTouched() && touchscreen.touched()) 
    {
      lastTouchTime = -SLEEP_TIMEOUT;
      // Get Touchscreen points
      TS_Point p = touchscreen.getPoint();
      // Calibrate Touchscreen points with map function to the correct width and height
      x = map(p.x, 200, 3700, 1, w); //200 - 2700
      y = map(p.y, 240, 3800, 1, h);
      z = p.z;
      if(x <= w-65 && x >= w-150 && y <= h-50 && y >= h-90)
      {
        keyboardDes = SSID;
        offlineScreen = KEYBOARD;
      }
      else if(x <= w-165 && x >= w-250 && y <= h-50 && y >= h-90)
      {
        keyboardDes = PASS;
        offlineScreen = KEYBOARD;
      }
      
    }
    if(offlineScreen == MAIN) 
    {
      fillScreen();
      printHeaderOffline();
      tft.setTextSize(2);
      updateTextColor();
      if(brightness == BRIGHT_HIGH)
      {
        if(theme == THEME_1)
        {
          tft.fillRect(65, 50, 85, 40, 0x04d3); 
          tft.fillRect(66, 51, 83, 38, 0x1866); 
        }
        else if(theme == THEME_2)
        {
          tft.fillRect(65, 50, 85, 40, 0x0000); 
          tft.fillRect(66, 51, 83, 38, 0xFFFF); 
        }
        else if(theme == THEME_3)
        {
          tft.fillRect(65, 50, 85, 40, 0x9000); 
          tft.fillRect(66, 51, 83, 38, 0x2862); 
        }
        else if(theme == THEME_4)
        {
          tft.fillRect(65, 50, 85, 40, 0x580b); 
          tft.fillRect(66, 51, 83, 38, 0x0000);
        }
      }
      else
      {
        if(theme == THEME_1)
        {
          tft.fillRect(65, 50, 85, 40, 0x07ff);
          tft.fillRect(66, 51, 83, 38, 0x1025);
        }
        else if(theme == THEME_3)
        {
          tft.fillRect(65, 50, 85, 40, 0xf800);
          tft.fillRect(66, 51, 83, 38, 0x1821);
        }
        else if(theme == THEME_2)
        {
          tft.fillRect(65, 50, 85, 40, 0x0000); 
          tft.fillRect(66, 51, 83, 38, 0x9cf3); 
        }
        else if(theme == THEME_4)
        {
          tft.fillRect(65, 50, 85, 40, 0xd81b); 
          tft.fillRect(66, 51, 83, 38, 0x0000);
        }
      }
      tft.drawCentreString("SSID", 110, 65, 1);
      
      if(brightness == BRIGHT_HIGH)
      {
        if(theme == THEME_1)
        {
          tft.fillRect(165, 50, 85, 40, 0x04d3); 
          tft.fillRect(166, 51, 83, 38, 0x1866); 
        }
        else if(theme == THEME_2)
        {
          tft.fillRect(165, 50, 85, 40, 0x0000); 
          tft.fillRect(166, 51, 83, 38, 0xFFFF); 
        }
        else if(theme == THEME_3)
        {
          tft.fillRect(165, 50, 85, 40, 0x9000); 
          tft.fillRect(166, 51, 83, 38, 0x2862); 
        }
        else if(theme == THEME_4)
        {
          tft.fillRect(165, 50, 85, 40, 0x580b); 
          tft.fillRect(166, 51, 83, 38, 0x0000);
        }
      }
      else
      {
        if(theme == THEME_1)
        {
          tft.fillRect(165, 50, 85, 40, 0x07ff);
          tft.fillRect(166, 51, 83, 38, 0x1025);
        }
        else if(theme == THEME_3)
        {
          tft.fillRect(165, 50, 85, 40, 0xf800);
          tft.fillRect(166, 51, 83, 38, 0x1821);
        }
        else if(theme == THEME_2)
        {
          tft.fillRect(165, 50, 85, 40, 0x0000); 
          tft.fillRect(166, 51, 83, 38, 0x9cf3); 
        }
        else if(theme == THEME_4)
        {
          tft.fillRect(165, 50, 85, 40, 0xd81b); 
          tft.fillRect(166, 51, 83, 38, 0x0000);
        }
      }
      tft.drawCentreString("PASS", 210, 65, 1);

      tft.setTextSize(3);
      tft.setCursor(65, 110);
      tft.print("ssid:");
      tft.setTextSize(2);
      tft.setCursor(65, 140);
      tft.print(ssid);

      if(retry == 20)
      {
        WiFi.begin(ssid, password);
        retry = 0;
      }
      else
        retry++;
    }
  }
  if(offlineScreen == KEYBOARD)
  {
    if(keyboardInput[0] == '\0')
    {
      tft.fillScreen(ILI9341_BLUE);

      tft.setTextColor(0xffff, 0xf000);
      if(keyboardDes == SSID)
          tft.drawCentreString("ssid", w/2, 35, 1);
      if(keyboardDes == PASS)
          tft.drawCentreString("Password", w/2, 35, 1);


      MakeKB_Button(Mobile_KB);
      //if (!touchscreen.bufferEmpty())
      //{
        GetKeyPress(MyBuffer);
      //}
    }
    else
    {
      if(keyboardDes == SSID)
      {
        strcpy(ssid, keyboardInput);
        keyboardInput[0] = '\0';
      }
      else if(keyboardDes == PASS)
      {
        strcpy(password, keyboardInput);
        keyboardInput[0] = '\0';
      }
      offlineScreen = MAIN;
    }
  }
}

void printHeaderOffline() {
  if(brightness == BRIGHT_HIGH)
    tft.fillRect(0, 0, w, 40, 0x07E0); // Green bar
  else
    tft.fillRect(0, 0, w, 40, 0x03a0); // Green bar
  tft.setTextSize(2);
  if(brightness == BRIGHT_HIGH)
    tft.setTextColor(0xFFFF); // White text
  else
    tft.setTextColor(0x9cf3);
  tft.setCursor(10, 10);
  if(offlineScreen == MAIN)
    tft.drawCentreString("Offline", w/2, 5, 2);
}
