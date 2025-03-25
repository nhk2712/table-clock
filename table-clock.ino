// Include the dependencies
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include <SPI.h>
#include "RTClib.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "fonts/NotoSansJP_Regular8pt7b.h"
#include "fonts/NotoSansJP_Regular5pt7b.h"
#include "icons/thermometer.h"

// Define the pins
#define ONE_WIRE_BUS 18  // For temperature sensor

// Define the constants
#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET -1  // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

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

char buffer[10];

// Variables for displaying
int hour_int = 0;
int minute_int = 0;
int second_int = 0;

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

void displayInfo() {
  // Clear previous display
  display.clearDisplay();

  // Get the current time from the RTC
  DateTime now = rtc.now();

  // Get date and time in integers
  hour_int = now.hour();
  minute_int = now.minute();
  second_int = now.second();

  day_int = now.day();
  month_int = now.month() - 1; // Starting from 0, which is January
  year_int = now.year();
  weekday_int = now.dayOfTheWeek();  // Starting from 0, which is Sunday

  // Get the temperature in Celsius
  temperature.requestTemperatures();
  float temp_c_float = temperature.getTempCByIndex(0);
  int temp_c = int(round(temp_c_float));

  //
  // Display the data into SSD1306
  //

  // Display hour and minute
  display.setFont(&NotoSansJP_Regular8pt7b);  // Set custom font
  display.setTextSize(1);                     // Text size (1 = small, 2 = medium, etc.)

  display.setCursor(6, 16);                   // Position (x, y)
  sprintf(buffer, "%02d:%02d", hour_int, minute_int);
  display.print(buffer);  // Print hour and minute in HH:MM format, with leading zero

  // Display second
  display.setFont(&NotoSansJP_Regular5pt7b);
  display.setTextSize(1);
  
  display.setCursor(48, 20);
  display.printf("%02d", second_int); // Print second with leading zero

  // Display day of the week
  display.setCursor(6, 42);
  display.print(daysOfTheWeek[weekday_int]); // Print day of week (DDD)

  // Display date and month
  display.setCursor(6, 58);
  display.printf("%02d", day_int); // Print date with leading zero

  display.setCursor(22, 58);
  display.print(months[month_int]); // Print month (MMM)

  // Display year
  display.setCursor(6, 74);
  display.print(year_int); // Print year (YYYY)

  // Display temperature
  display.drawBitmap(6, 96, thermometer, 16, 16, WHITE);

  display.setCursor(25, 108);
  display.printf("%02d", temp_c); // Print temp C with leading zero

  display.drawBitmap(35, 102, degreeSymbol, 8, 8, WHITE); // Degree symbol
  display.setCursor(43, 108);
  display.print("C"); // Print letter C

  // Show everything on screen
  display.display();

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

  // Print the temperature
  Serial.print(temp_c);
  Serial.println("°C");
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

  // Set the displayed content rotated 90deg counter-clockwise
  display.setRotation(3);
  display.setTextColor(WHITE);

  // Start displaying
  displayInfo();
}


void loop() {
  // Check if a period has passed
  currentMillis = millis();

  // Update every 1 sec
  if (currentMillis - prevUpdateMillis >= 1000) {
    prevUpdateMillis = currentMillis;

    displayInfo();
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
