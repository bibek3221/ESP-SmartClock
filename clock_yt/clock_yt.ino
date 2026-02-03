#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include "Fonts.h"
#include <SPI.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <TimeLib.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include "secrets.h"

// Uncomment according to your hardware type
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
//#define HARDWARE_TYPE MD_MAX72XX::GENERIC_HW

// Defining size, and output pins
#define MAX_DEVICES 4
#define CS_PIN 15
#define TOUCH_PIN 4  // GPIO4 (D2 on NodeMCU) - Change this to your connected pin

char* ssid = WIFI_SSID;              //wifi ssid
char* password = WIFI_PASSWORD;      //wifi password
const long utcOffsetInSeconds = 19800;  // Time zone offset

WiFiUDP ntpUDP;
MD_Parola Display = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);
NTPClient timeClient(ntpUDP, "asia.pool.ntp.org", utcOffsetInSeconds, 60000);

char Time[] = "00:00";
char Seconds[] = "00";
char Date[] = "00-00-2000";
byte last_second, second_, minute_, hour_, day_, month_;
int year_;

const char* DaysWeek[] = { "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT" };

// Variables for touch switch
enum DisplayMode { SHOW_TIME, SHOW_DATE, SHOW_WEATHER };
DisplayMode currentMode = SHOW_TIME;
unsigned long modeDisplayStart = 0;
const unsigned long DATE_DISPLAY_DURATION = 5000; // 5 seconds in milliseconds
const unsigned long WEATHER_DISPLAY_DURATION = 10000; // 10 seconds for weather
bool lastTouchState = LOW;

// Double-click detection variables
unsigned long lastClickTime = 0;
const unsigned long DOUBLE_CLICK_INTERVAL = 500; // 500ms window for double-click
int clickCount = 0;

// Weather variables
float temperature = 0.0;
float feelsLike = 0.0;
int humidity = 0;
unsigned long lastWeatherUpdate = 0;
const unsigned long WEATHER_UPDATE_INTERVAL = 1800000; // 30 minutes in milliseconds
bool weatherDataAvailable = false;

char weatherLine1[20] = "";
char weatherLine2[20] = "";
int weatherDisplayState = 0; // 0=temp, 1=feels like, 2=humidity
unsigned long weatherStateChangeTime = 0;
const unsigned long WEATHER_STATE_DURATION = 3000; // Show each weather info for 3 seconds

// const char* WEATHER_API_URL = "http://api.weatherapi.com/v1/current.json?key=4e8068c62ffc4034a07163742262401&q=22.5697,88.3697";

// Build API URL using secrets
char WEATHER_API_FULL_URL[150];

void buildWeatherURL() {
  snprintf(WEATHER_API_FULL_URL, sizeof(WEATHER_API_FULL_URL), 
           "%s?key=%s&q=%s", 
           WEATHER_API_URL, 
           WEATHER_API_KEY, 
           WEATHER_LOCATION);
}

// Variables to track if date functions have been called today
bool dateFunction_Jan1_called = false;
bool dateFunction_Jan15_called = false;
bool dateFunction_Dec25_called = false;
int lastCheckedDay = 0;

// ========== DATE-SPECIFIC FUNCTIONS ==========

void new_year() {
  Serial.println("Happy New Year!");
  
  // Display special message on LED
  Display.displayZoneText(0, "HAPPY", PA_CENTER, 50, 1000, PA_SCROLL_LEFT);
  Display.displayZoneText(1, "2026!", PA_CENTER, 50, 1000, PA_SCROLL_LEFT);
  Display.displayAnimate();
  
  // Wait for animation to complete
  while (!Display.displayAnimate()) {
    delay(10);
  }
  
  delay(2000); // Show message for 2 seconds
  
  // Add your custom New Year code here
  // For example: play a tune, blink LEDs, etc.
}

void happy() {
  Serial.println("Happy January 15th!");
  
  // Display special message on LED
  Display.displayZoneText(0, "JAN", PA_CENTER, 50, 1000, PA_SCROLL_LEFT);
  Display.displayZoneText(1, "15TH", PA_CENTER, 50, 1000, PA_SCROLL_LEFT);
  Display.displayAnimate();
  
  // Wait for animation to complete
  while (!Display.displayAnimate()) {
    delay(10);
  }
  
  delay(2000);
  
  // Add your custom January 15th code here
}

void crish() {
  Serial.println("Merry Christmas!");
  
  // Display special message on LED
  Display.displayZoneText(0, "XMAS", PA_CENTER, 50, 1000, PA_SCROLL_LEFT);
  Display.displayZoneText(1, "JOY!", PA_CENTER, 50, 1000, PA_SCROLL_LEFT);
  Display.displayAnimate();
  
  // Wait for animation to complete
  while (!Display.displayAnimate()) {
    delay(10);
  }
  
  delay(2000);
  
  // Add your custom Christmas code here
  // For example: play carol, special light pattern, etc.
}

// Function to check and call date-specific functions
void checkSpecialDates() {
  // Reset flags when day changes
  if (day_ != lastCheckedDay) {
    dateFunction_Jan1_called = false;
    dateFunction_Jan15_called = false;
    dateFunction_Dec25_called = false;
    lastCheckedDay = day_;
  }
  
  // Check for January 1st
  if (month_ == 1 && day_ == 1 && !dateFunction_Jan1_called) {
    new_year();
    dateFunction_Jan1_called = true;
  }
  
  // Check for January 15th
  if (month_ == 1 && day_ == 15 && !dateFunction_Jan15_called) {
    happy();
    dateFunction_Jan15_called = true;
  }
  
  // Check for December 25th
  if (month_ == 12 && day_ == 25 && !dateFunction_Dec25_called) {
    crish();
    dateFunction_Dec25_called = true;
  }
}

// ========== END DATE-SPECIFIC FUNCTIONS ==========

void setup() {
  Serial.begin(115200);
  
  Display.begin(2);
  Display.setZone(0, 1, 3);
  Display.setZone(1, 0, 0);
  
  /*If making changes to the display, don't use these fonts
      as they only have numbers and do not support characters*/
  Display.setFont(0, SmallDigits);
  Display.setFont(1, SmallerDigits);
  Display.setIntensity(2);
  Display.setCharSpacing(0);
  
  // Setup touch pin
  pinMode(TOUCH_PIN, INPUT);
  
  Display.displayZoneText(0, "WIFI", PA_LEFT, Display.getSpeed(), Display.getPause(), PA_NO_EFFECT);
  Display.displayZoneText(1, "....", PA_LEFT, Display.getSpeed(), Display.getPause(), PA_NO_EFFECT);
  Display.displayAnimate();
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\nWiFi Connected");
  
  timeClient.begin();
  
  // Get initial weather data
  getWeatherData();
}

void getWeatherData() {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;
    HTTPClient http;
    
    Serial.println("Fetching weather data...");
    http.begin(client, WEATHER_API_URL);
    int httpCode = http.GET();
    
    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      Serial.println("Weather data received");
      
      // Parse JSON
      DynamicJsonDocument doc(2048);
      DeserializationError error = deserializeJson(doc, payload);
      
      if (!error) {
        temperature = doc["current"]["temp_c"];
        feelsLike = doc["current"]["feelslike_c"];
        humidity = doc["current"]["humidity"];
        
        weatherDataAvailable = true;
        lastWeatherUpdate = millis();
        
        Serial.print("Temp: ");
        Serial.print(temperature);
        Serial.print("C, Feels: ");
        Serial.print(feelsLike);
        Serial.print("C, Humidity: ");
        Serial.print(humidity);
        Serial.println("%");
      } else {
        Serial.println("JSON parsing failed");
      }
    } else {
      Serial.print("HTTP error: ");
      Serial.println(httpCode);
    }
    
    http.end();
  }
}

void updateWeatherDisplay() {
  unsigned long currentMillis = millis();
  
  // Cycle through weather states every 3 seconds
  if (currentMillis - weatherStateChangeTime >= WEATHER_STATE_DURATION) {
    weatherDisplayState++;
    if (weatherDisplayState > 2) {
      weatherDisplayState = 0;
    }
    weatherStateChangeTime = currentMillis;
  }
  
  // Update display based on current state
  switch (weatherDisplayState) {
    case 0: // Temperature
      dtostrf(temperature, 4, 1, weatherLine1);
      strcat(weatherLine1, "C");
      strcpy(weatherLine2, "TEMP");
      break;
      
    case 1: // Feels like
      dtostrf(feelsLike, 4, 1, weatherLine1);
      strcat(weatherLine1, "C");
      strcpy(weatherLine2, "FEEL");
      break;
      
    case 2: // Humidity
      sprintf(weatherLine1, "%d%%", humidity);
      strcpy(weatherLine2, "HUMI");
      break;
  }
}

void loop() {
  timeClient.update();
  unsigned long currentMillis = millis();
  unsigned long unix_epoch = timeClient.getEpochTime();
  int Day = timeClient.getDay();
  
  // Update weather data every 30 minutes
  if (currentMillis - lastWeatherUpdate >= WEATHER_UPDATE_INTERVAL) {
    getWeatherData();
  }
  
  // Read touch switch state
  bool currentTouchState = digitalRead(TOUCH_PIN);
  
  // Detect touch (rising edge) for click detection
  if (currentTouchState == HIGH && lastTouchState == LOW) {
    unsigned long timeSinceLastClick = currentMillis - lastClickTime;
    
    if (timeSinceLastClick < DOUBLE_CLICK_INTERVAL) {
      // Double-click detected
      clickCount = 2;
      Serial.println("Double-click detected!");
      
      if (weatherDataAvailable) {
        currentMode = SHOW_WEATHER;
        modeDisplayStart = currentMillis;
        weatherDisplayState = 0;
        weatherStateChangeTime = currentMillis;
      }
    } else {
      // First click
      clickCount = 1;
    }
    
    lastClickTime = currentMillis;
  }
  lastTouchState = currentTouchState;
  
  // Handle single click (with delay to detect if it becomes double-click)
  if (clickCount == 1 && (currentMillis - lastClickTime > DOUBLE_CLICK_INTERVAL)) {
    // Single click confirmed
    currentMode = SHOW_DATE;
    modeDisplayStart = currentMillis;
    clickCount = 0;
  }
  
  if (clickCount == 2) {
    clickCount = 0; // Reset after processing double-click
  }
  
  // Check if display mode duration has elapsed
  if (currentMode == SHOW_DATE && (currentMillis - modeDisplayStart >= DATE_DISPLAY_DURATION)) {
    currentMode = SHOW_TIME;
  }
  
  if (currentMode == SHOW_WEATHER && (currentMillis - modeDisplayStart >= WEATHER_DISPLAY_DURATION)) {
    currentMode = SHOW_TIME;
  }
  
  second_ = second(unix_epoch);
  if (last_second != second_) {
    minute_ = minute(unix_epoch);
    hour_ = hour(unix_epoch);
    day_ = day(unix_epoch);
    month_ = month(unix_epoch);
    year_ = year(unix_epoch);
    
    // Check for special dates at the start of each new day (at midnight)
    if (hour_ == 0 && minute_ == 0 && second_ == 0) {
      checkSpecialDates();
    }
    
    Seconds[1] = second_ % 10 + 48;
    Seconds[0] = second_ / 10 + 48;
    Time[4] = minute_ % 10 + 48;
    Time[3] = minute_ / 10 + 48;
    Time[1] = hour_ % 10 + 48;
    Time[0] = hour_ / 10 + 48;
    Date[0] = day_ / 10 + 48;
    Date[1] = day_ % 10 + 48;
    Date[3] = month_ / 10 + 48;
    Date[4] = month_ % 10 + 48;
    Date[8] = (year_ / 10) % 10 + 48;
    Date[9] = year_ % 10 % 10 + 48;

    
    
    // Display based on current mode
    switch (currentMode) {
      case SHOW_DATE:
        Display.displayZoneText(0, DaysWeek[Day], PA_LEFT, Display.getSpeed(), Display.getPause(), PA_NO_EFFECT);
        Display.displayZoneText(1, Date, PA_LEFT, Display.getSpeed(), Display.getPause(), PA_NO_EFFECT);
        break;
        
      case SHOW_WEATHER:
        updateWeatherDisplay();
        Display.displayZoneText(0, weatherLine1, PA_LEFT, Display.getSpeed(), Display.getPause(), PA_NO_EFFECT);
        Display.displayZoneText(1, weatherLine2, PA_LEFT, Display.getSpeed(), Display.getPause(), PA_NO_EFFECT);
        break;
        
      case SHOW_TIME:
      default:
        Display.displayZoneText(0, Time, PA_LEFT, Display.getSpeed(), Display.getPause(), PA_NO_EFFECT);
        Display.displayZoneText(1, Seconds, PA_LEFT, Display.getSpeed(), Display.getPause(), PA_NO_EFFECT);
        break;
    }
    
    Display.displayAnimate();
    last_second = second_;
  }
  
  // Update weather display cycling even when second hasn't changed
  if (currentMode == SHOW_WEATHER) {
    updateWeatherDisplay();
    Display.displayZoneText(0, weatherLine1, PA_LEFT, Display.getSpeed(), Display.getPause(), PA_NO_EFFECT);
    Display.displayZoneText(1, weatherLine2, PA_LEFT, Display.getSpeed(), Display.getPause(), PA_NO_EFFECT);
  }
  
  delay(50);  // Reduced delay for better touch response
}