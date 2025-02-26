// Include the libraries
#include <TM1637Display.h>
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

// Define the pins
#define CLK 23
#define DIO 22

// Define WiFi SSID and password
const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASSWORD = "";

// Define the timezone
int timezone = 8;

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", timezone*3600, 60000);

// Variables to read the time
String formattedTime;

// Create a display object of type TM1637Display
TM1637Display display = TM1637Display(CLK, DIO);

// Create an array that turns all segments ON
const uint8_t allON[] = {0xff, 0xff, 0xff, 0xff};

// Create an array that turns all segments OFF
const uint8_t allOFF[] = {0x00, 0x00, 0x00, 0x00};

// Create an array that sets individual segments per digit to display the word "dOnE"
const uint8_t done[] = {
  SEG_B | SEG_C | SEG_D | SEG_E | SEG_G,           // d
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,   // O
  SEG_C | SEG_E | SEG_G,                           // n
  SEG_A | SEG_D | SEG_E | SEG_F | SEG_G            // E
};

// Create degree celsius symbol
const uint8_t celsius[] = {
  SEG_A | SEG_B | SEG_F | SEG_G,  // Degree symbol
  SEG_A | SEG_D | SEG_E | SEG_F   // C
};

// Establish WiFi connection
void initWiFi() {
 WiFi.mode(WIFI_STA);    //Set Wi-Fi Mode as station
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

  // Init the WiFi connection
  initWiFi();

  // Set the brightness to 5 (0=dimmest 7=brightest)
	display.setBrightness(5);

  // Init the time client
  timeClient.begin();
  timeClient.forceUpdate();
}

void loop() {
  // put your main code here, to run repeatedly:

  // Get the time
  formattedTime = timeClient.getFormattedTime(); // in format HH:MM:SS
  Serial.println(formattedTime);

  // Get substrings indicating hr, min, sec separately
  String hour_str = formattedTime.substring(0, 2);
  String minute_str = formattedTime.substring(3, 5);
  String second_str = formattedTime.substring(6, 8);

  // Parse the substrings to integer
  int hour_int = hour_str.toInt();
  int minute_int = minute_str.toInt();
  int second_int = second_str.toInt();

  // Display the integers into the TM1637
  display.showNumberDec(minute_int, true, 2, 0);
  display.showNumberDecEx(second_int, 0b11100000, true, 2, 2);

  delay(1000);
}

// References: 
// [1] https://lastminuteengineers.com/tm1637-arduino-tutorial/
// [2] https://www.electronicwings.com/esp32/esp32-wi-fi-basics-getting-started
// [3] https://randomnerdtutorials.com/esp32-ntp-client-date-time-arduino-ide/
// [4] https://github.com/arduino-libraries/NTPClient
