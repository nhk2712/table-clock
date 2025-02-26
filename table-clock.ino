// Include the libraries
#include <TM1637Display.h>
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#include "time.h"

// Define the pins
#define CLK 23
#define DIO 22
#define BTN1 21  // Use for changing display mode

// Define WiFi SSID and password
const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASSWORD = "";

// Define the timezone
int timezone = 8;

// Create a display object of type TM1637Display
TM1637Display display = TM1637Display(CLK, DIO);

// Create an array that turns all segments ON
const uint8_t allON[] = { 0xff, 0xff, 0xff, 0xff };

// Create an array that turns all segments OFF
const uint8_t allOFF[] = { 0x00, 0x00, 0x00, 0x00 };

// Create an array that sets individual segments per digit to display the word "dOnE"
const uint8_t done[] = {
  SEG_B | SEG_C | SEG_D | SEG_E | SEG_G,          // d
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,  // O
  SEG_C | SEG_E | SEG_G,                          // n
  SEG_A | SEG_D | SEG_E | SEG_F | SEG_G           // E
};

// Create degree celsius symbol
const uint8_t celsius[] = {
  SEG_A | SEG_B | SEG_F | SEG_G,  // Degree symbol
  SEG_A | SEG_D | SEG_E | SEG_F   // C
};

// Store button state
// BTN1 use pullup => by default is HIGH, when pressed is LOW
int btn1_prevState = HIGH;
int btn1_currentState;

// Multiple display modes
int maxMode = 5; // maximum number of modes
int mode = 0;
// 0 = HH:MM;
// 1 = MM:SS;
// 2 = DD:MM;
// 3 = YYYY;
// 4 = DD°C;

// Establish WiFi connection
void initWiFi() {
  WiFi.mode(WIFI_STA);  //Set Wi-Fi Mode as station
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(100);
  }

  Serial.println();
  Serial.println(WiFi.localIP());

  Serial.print("RSSI: ");
  Serial.print(WiFi.RSSI());
  Serial.println(" dBm");

  Serial.print("Connected to WiFi successfully at: ");
  Serial.println(WIFI_SSID);
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Hello, ESP32!");

  // Init the pin config
  pinMode(BTN1, INPUT_PULLUP);  // config BTN1 as input pin and enable the internal pull-up resistor

  // Init the WiFi connection
  initWiFi();

  // Set the brightness to 5 (0=dimmest 7=brightest)
  display.setBrightness(5);

  // Init and get the time
  configTime(timezone*3600, 0, "pool.ntp.org");
}

unsigned long previousMillis = 0;
const long interval = 1000;  // 1 second

void displayInfo() {
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }

  // Get date and time in integers
  int hour_int = timeinfo.tm_hour;
  int minute_int = timeinfo.tm_min;
  int second_int = timeinfo.tm_sec;

  int day_int = timeinfo.tm_mday;
  int month_int = timeinfo.tm_mon + 1;
  int year_int = timeinfo.tm_year + 1900;
  int weekday_int = timeinfo.tm_wday;

  // Get the temperature in Celsius
  int temp_c = 23;

  // Display the integers into the TM1637
  if (mode == 0) {
    display.showNumberDec(hour_int, true, 2, 0);
    display.showNumberDecEx(minute_int, 0b11100000, true, 2, 2);
  } else if (mode == 1) {
    display.showNumberDec(minute_int, true, 2, 0);
    display.showNumberDecEx(second_int, 0b11100000, true, 2, 2);
  } else if (mode == 2) {
    display.showNumberDec(day_int, true, 2, 0);
    display.showNumberDecEx(month_int, 0b11100000, true, 2, 2);
  } else if (mode == 3) {
    display.showNumberDecEx(year_int, 0, true, 4, 0);
  } else if (mode == 4) {
    display.showNumberDec(temp_c, false, 2, 0);
    display.setSegments(celsius, 2, 2);
  }

  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

void loop() {
  // Attempt to reconnect when WiFi connection drops
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi lost, reconnecting...");
    WiFi.disconnect();
    WiFi.reconnect();
  }

  // Read BTN1 state
  btn1_currentState = digitalRead(BTN1);

  if (btn1_prevState == HIGH && btn1_currentState == LOW) {
    delay(50);  // Debounce delay
    if (digitalRead(BTN1) == LOW) {
      Serial.println("The button has been pressed");

      mode += 1;
      if (mode == maxMode) mode = 0;

      displayInfo();
    }
  }

  btn1_prevState = btn1_currentState;

  // Check if 1 second has passed
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

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
