// Include the dependencies
#include <TM1637Display.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "RTClib.h"

// Define the pins
#define CLK 32
#define DIO 25
#define BTN1 27          // Use to turn into setting value mode
#define BTN2 14          // Use to edit value
#define ONE_WIRE_BUS 18  // For temperature sensor

// Create a display object of type TM1637Display
TM1637Display display = TM1637Display(CLK, DIO);

// Create letter d (lower-case) symbol
const uint8_t d_letter[] = {
  SEG_B | SEG_C | SEG_D | SEG_E | SEG_G  // d
};

// Create underscore (_) symbol
const uint8_t underscore[] = {
  SEG_D // _
};

// Create degree celsius symbol
const uint8_t celsius[] = {
  SEG_A | SEG_B | SEG_F | SEG_G,  // Degree symbol
  SEG_A | SEG_D | SEG_E | SEG_F   // C
};

// Store button state
// BTN use pullup => by default is HIGH, when pressed is LOW
int btn1_prevState = HIGH;
int btn1_currentState;
int btn2_prevState = HIGH;
int btn2_currentState;

// Init a RTC instance
RTC_DS1307 rtc;
char daysOfTheWeek[7][12] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature sensor
DallasTemperature temperature(&oneWire);

// Multiple display modes
int maxMode = 5;  // maximum number of modes
int mode = 0;
// 0 = HH:MM;
// 1 = _d0x; // Day of the week, d01 = Sunday
// 2 = DD:MM; // Date and month
// 3 = YYYY;
// 4 = DD°C;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Hello, ESP32!");

  // Init the pin config
  pinMode(BTN1, INPUT_PULLUP);  // config BTN1 as input pin and enable the internal pull-up resistor
  pinMode(BTN2, INPUT_PULLUP);  // config BTN2 as input pin and enable the internal pull-up resistor

  // Set the brightness (0=dimmest 7=brightest)
  display.setBrightness(3);

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
}

unsigned long prevRefreshMillis = 0;

void displayInfo() {
  // Get the current time from the RTC
  DateTime now = rtc.now();

  // Get date and time in integers
  int hour_int = now.hour();
  int minute_int = now.minute();
  int second_int = now.second();

  int day_int = now.day();
  int month_int = now.month();
  int year_int = now.year();
  int weekday_int = now.dayOfTheWeek() + 1;  // Starting from 1, which is Sunday

  // Get the temperature in Celsius
  temperature.requestTemperatures();
  float temp_c_float = temperature.getTempCByIndex(0);
  int temp_c = int(round(temp_c_float));

  // Display the integers into the TM1637
  if (mode == 0) {
    display.showNumberDecEx(hour_int * 100 + minute_int, 0b01000000, true, 4, 0);
  } else if (mode == 1) {
    display.setSegments(underscore, 1, 0);
    display.setSegments(d_letter, 1, 1);
    display.showNumberDec(weekday_int, true, 2, 2);
  } else if (mode == 2) {
    display.showNumberDec(day_int * 100 + month_int, true, 4, 0);
  } else if (mode == 3) {
    display.showNumberDec(year_int, true, 4, 0);
  } else if (mode == 4) {
    display.showNumberDec(temp_c, false, 2, 0);
    display.setSegments(celsius, 2, 2);
  }

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

void loop() {

  // Read BTN1 state // BTN is for turning among setting modes
  btn1_currentState = digitalRead(BTN1);
  if (btn1_prevState == HIGH && btn1_currentState == LOW) {
    delay(50);  // Debounce delay
    if (digitalRead(BTN1) == LOW) {
      Serial.println("BTN1 has been pressed");
    }
  }
  btn1_prevState = btn1_currentState;

  // Read BTN2 state
  btn2_currentState = digitalRead(BTN2);
  if (btn2_prevState == HIGH && btn2_currentState == LOW) {
    delay(50);  // Debounce delay
    if (digitalRead(BTN2) == LOW) {
      Serial.println("BTN2 has been pressed");
    }
  }
  btn2_prevState = btn2_currentState;

  // Check if a period has passed
  unsigned long currentMillis = millis();

  // Refresh mode after 5 sec
  if (currentMillis - prevRefreshMillis >= 5000) {
    prevRefreshMillis = currentMillis;
    mode += 1;
    if (mode == maxMode) mode = 0;

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
