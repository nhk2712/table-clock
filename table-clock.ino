// Include the dependencies
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include <SPI.h>
#include "RTClib.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_ST7735.h>
#include "fonts/NotoSansJP_Regular10pt7b.h"
#include "fonts/NotoSansJP_Regular25pt7b.h"
#include "fonts/NotoSansJP_Regular12pt7b.h"
#include "icons/temperature.h"

// Define the pins
#define ONE_WIRE_BUS 17  // For temperature sensor
#define TFT_CS 5
#define TFT_RST 4
#define TFT_DC 2
#define BLK_PWM 33  // For backlight, use PWM to adjust brightness

// Define the constants
#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET -1  // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Create an instance of the ST7735 class
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

// Init a RTC instance
RTC_DS1307 rtc;
char daysOfTheWeek[7][4] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
char months[12][4] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature sensor
DallasTemperature temperature(&oneWire);

// Variables for tracking of time
unsigned long currentMillis = 0;
unsigned long prevUpdateMillis = 0;
unsigned long prevTempCMillis = 0;

// Variables for displaying
int hour_int = 0;
int minute_int = 0;
int second_int = 0;

char hour_buffer[3];
char minute_buffer[3];

int day_int = 0;
int month_int = 0;
int year_int = 0;
int weekday_int = 0;

const uint8_t degreeSymbol[] PROGMEM = {
  0x1C,  // ....####  (top row)
  0x36,  // ...##..## (second row)
  0x36,  // ...##..## (third row)
  0x1C,  // ....####  (fourth row)
  0x00,  // ........  (empty rows)
  0x00,  // ........  
  0x00   // ........ 
};

// display = SSD1306
// tft = ST7735

void displayTime() {
  // Clear the screen with a color
  tft.fillScreen(ST77XX_BLACK);

  // Get the current time from the RTC
  DateTime now = rtc.now();

  // Get date and time in integers
  hour_int = now.hour();
  minute_int = now.minute();
  second_int = now.second();

  day_int = now.day();
  month_int = now.month() - 1;  // Starting from 0, which is January
  year_int = now.year();
  weekday_int = now.dayOfTheWeek();  // Starting from 0, which is Sunday

  //
  // Display the data into ST7735
  //

  // Display hour
  tft.setFont(&NotoSansJP_Regular25pt7b);

  sprintf(hour_buffer, "%02d", hour_int);
  tft.setCursor(10, 50);
  tft.print(hour_buffer);

  // Display minute
  sprintf(minute_buffer, "%02d", minute_int);
  tft.setCursor(93, 50);
  tft.print(minute_buffer);

  // Display colon
  tft.setFont(&NotoSansJP_Regular12pt7b);

  tft.setCursor(73, 40);
  tft.print(":");

  // Display day of the week
  tft.setCursor(10, 99);
  tft.print(daysOfTheWeek[weekday_int]);

  // Display date
  if (day_int < 10){
    tft.setCursor(75, 99);
  }
  else{
    tft.setCursor(67, 99);
  }
  tft.print(day_int);

  // Display month
  tft.setCursor(107, 99);
  tft.print(months[month_int]);

  //
  // The lines below are for debugging
  //

  // Getting each time field in individual variables
  // And adding a leading zero when needed;
  String yearStr = String(now.year(), DEC);
  String monthStr = (now.month() < 10 ? "0" : "") + String(now.month(), DEC);
  String dayStr = (now.day() < 10 ? "0" : "") + String(now.day(), DEC);
  String hourStr = (now.hour() < 10 ? "0" : "") + String(now.hour(), DEC);
  String minuteStr = (now.minute() < 10 ? "0" : "") + String(now.minute(), DEC);
  String secondStr = (now.second() < 10 ? "0" : "") + String(now.second(), DEC);
  String dayOfWeek = daysOfTheWeek[now.dayOfTheWeek()];

  // Complete time string
  String formattedTime = dayOfWeek + ", " + yearStr + "-" + monthStr + "-" + dayStr + " " + hourStr + ":" + minuteStr + ":" + secondStr;

  // Print the complete formatted time
  Serial.println(formattedTime);

}

void displayTempC(){
  // Clear previous display
  display.clearDisplay();

  // Get the temperature in Celsius
  temperature.requestTemperatures();
  float temp_c_float = temperature.getTempCByIndex(0);
  int temp_c = int(round(temp_c_float));

  //
  // Display the data into SSD1306
  //

  // Display temperature
  display.drawBitmap(22, 11, temperature_bitmap, 24, 24, WHITE);

  display.setFont(&NotoSansJP_Regular10pt7b);
  display.setCursor(20, 64);
  display.printf("%02d", temp_c);  // Print temp C with leading zero

  display.drawBitmap(22, 80, degreeSymbol, 8, 8, WHITE);  // Degree symbol
  display.setCursor(33, 95);
  display.print("C");  // Print letter C

  // Show everything on screen
  display.display();
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Hello, ESP32!");

  // Config the RTC
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1) delay(10);
  }

  if (!rtc.isrunning()) {
    Serial.println("RTC is NOT running, let's set the time!");
    // When time needs to be set on a new device, or after a power loss, the
    // following line sets the RTC to the date & time this sketch was compiled
    // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    //rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }
  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  // Init the Dallas Temperature
  temperature.begin();

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {  // default I2C address for SSD1306
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;  // Don't proceed, loop forever
  }

  // Set the displayed content rotated 90deg clockwise
  display.setRotation(1);
  display.setTextColor(WHITE);
  display.setTextSize(1);

  // Initialize the display for ST7735
  tft.initR(INITR_BLACKTAB);  // For 1.8" displays
  tft.setRotation(1);         // Adjust orientation to 90deg clockwise
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);

  // Set brightness for ST7735
  ledcAttachPin(BLK_PWM, 1); // Attach pin to PWM channel 1
  ledcSetup(1, 5000, 8);  // 5kHz frequency, 8-bit resolution
  ledcWrite(1, 128);  // Set brightness (0-255, 128 = 50%)

  // Start displaying
  displayTime();
  displayTempC();
}


void loop() {
  // Check if a period has passed
  currentMillis = millis();

  // Update time every 1 min
  if (currentMillis - prevUpdateMillis >= 60000) {
    prevUpdateMillis = currentMillis;

    displayTime();
  }

  // Update TempC every 1 sec
  if (currentMillis - prevTempCMillis >= 1000) {
    prevTempCMillis = currentMillis;

    displayTempC();
  }
}

// References:
// [1] https://lastminuteengineers.com/tm1637-arduino-tutorial/
// [2] https://www.electronicwings.com/esp32/esp32-wi-fi-basics-getting-started
// [3] https://randomnerdtutorials.com/esp32-ntp-client-date-time-arduino-ide/
// [4] https://github.com/arduino-libraries/NTPClient
// [5] https://esp32io.com/tutorials/esp32-button
// [6] https://bytesim.com/blogs/esim/how-to-improve-wifi-signal-strength
// [7] https://lastminuteengineers.com/esp32-ntp-server-date-time-tutorial/
// [8] https://www.geeksforgeeks.org/time-h-header-file-in-c-with-examples/
// [9] https://www.upesy.com/blogs/tutorials/how-to-use-gpio-pins-of-esp32-with-arduino?srsltid=AfmBOorA66QR6KnZLyeLmbsXRWgIx_rGInB9UIDc2FU-AX1Tg714yGDv
// [10] https://randomnerdtutorials.com/guide-for-ds18b20-temperature-sensor-with-arduino/
// [11] https://randomnerdtutorials.com/esp32-ds1307-real-time-clock-rtc-arduino/
// [12] https://randomnerdtutorials.com/esp32-flash-memory/
// [13] https://randomnerdtutorials.com/esp32-pinout-reference-gpios/
// [14] https://randomnerdtutorials.com/esp32-ssd1306-oled-display-arduino-ide/
// [15] https://simple-circuit.com/arduino-ds1307-ssd1306-oled/
// [16] https://www.teachmemicro.com/using-the-1-77-st7735-tft-lcd-with-esp32/
