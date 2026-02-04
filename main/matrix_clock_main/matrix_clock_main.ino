
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

// Include modular files
#include "special_dates.h"
#include "touch_handler.h"
#include "weather_handler.h"

// Hardware configuration
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4
#define CS_PIN 15

// Global display object
MD_Parola Display = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

// Time variables
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "asia.pool.ntp.org", 19800, 60000);
char Time[] = "00:00";
char Seconds[] = "00";
char Date[] = "00-00-2000";
byte last_second, second_, minute_, hour_, day_, month_;
int year_;

// Display mode
DisplayMode currentMode = SHOW_TIME;
unsigned long modeDisplayStart = 0;

// Touch handler object
TouchHandler touchHandler(4);  // GPIO4

// Weather handler object
WeatherHandler weatherHandler;

void setup() {
  Serial.begin(115200);
  
  // Initialize display
  Display.begin(2);
  Display.setZone(0, 1, 3);
  Display.setZone(1, 0, 0);
  Display.setFont(0, SmallDigits);
  Display.setFont(1, SmallerDigits);
  Display.setIntensity(2);
  Display.setCharSpacing(0);
  
  // Initialize touch handler
  touchHandler.begin();
  
  // Show WiFi connecting message
  Display.displayZoneText(0, "WIFI", PA_LEFT, Display.getSpeed(), Display.getPause(), PA_NO_EFFECT);
  Display.displayZoneText(1, "....", PA_LEFT, Display.getSpeed(), Display.getPause(), PA_NO_EFFECT);
  Display.displayAnimate();
  
  // Connect to WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected");
  
  // Initialize time client
  timeClient.begin();
  
  // Initialize weather handler
  weatherHandler.begin();
}

void loop() {
  // Update time
  timeClient.update();
  unsigned long currentMillis = millis();
  unsigned long unix_epoch = timeClient.getEpochTime();
  
  // Update weather periodically
  weatherHandler.update(currentMillis);
  
  // Check for touch input
  TouchEvent touchEvent = touchHandler.checkTouch(currentMillis);
  handleTouchEvent(touchEvent, currentMillis);
  
  // Get time components
  second_ = second(unix_epoch);
  if (last_second != second_) {
    minute_ = minute(unix_epoch);
    hour_ = hour(unix_epoch);
    day_ = day(unix_epoch);
    month_ = month(unix_epoch);
    year_ = year(unix_epoch);
    
    // Check for special dates
    if (hour_ == 0 && minute_ == 0 && second_ == 0) {
      checkSpecialDates(month_, day_, year_);
    }
    
    // Update time strings
    updateTimeStrings();
    
    // Update display based on mode
    updateDisplay();
    
    last_second = second_;
  }
  
  // Handle weather display cycling
  if (currentMode == SHOW_WEATHER) {
    weatherHandler.updateDisplayState(currentMillis);
    Display.displayZoneText(0, weatherHandler.getLine1(), PA_LEFT, Display.getSpeed(), Display.getPause(), PA_NO_EFFECT);
    Display.displayZoneText(1, weatherHandler.getLine2(), PA_LEFT, Display.getSpeed(), Display.getPause(), PA_NO_EFFECT);
  }
  
  // Check if we should return to time display
  if (currentMode != SHOW_TIME) {
    if (currentMillis - modeDisplayStart >= getModeDuration(currentMode)) {
      currentMode = SHOW_TIME;
    }
  }
  
  Display.displayAnimate();
  delay(50);
}

void updateTimeStrings() {
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
}

void updateDisplay() {
  switch (currentMode) {
    case SHOW_DATE:
      Display.displayZoneText(0, getDayOfWeekString(timeClient.getDay()), PA_LEFT, Display.getSpeed(), Display.getPause(), PA_NO_EFFECT);
      Display.displayZoneText(1, Date, PA_LEFT, Display.getSpeed(), Display.getPause(), PA_NO_EFFECT);
      break;
      
    case SHOW_WEATHER:
      Display.displayZoneText(0, weatherHandler.getLine1(), PA_LEFT, Display.getSpeed(), Display.getPause(), PA_NO_EFFECT);
      Display.displayZoneText(1, weatherHandler.getLine2(), PA_LEFT, Display.getSpeed(), Display.getPause(), PA_NO_EFFECT);
      break;
      
    case SHOW_TIME:
    default:
      Display.displayZoneText(0, Time, PA_LEFT, Display.getSpeed(), Display.getPause(), PA_NO_EFFECT);
      Display.displayZoneText(1, Seconds, PA_LEFT, Display.getSpeed(), Display.getPause(), PA_NO_EFFECT);
      break;
  }
}

void handleTouchEvent(TouchEvent event, unsigned long currentMillis) {
  switch (event) {
    case SINGLE_CLICK:
      currentMode = SHOW_DATE;
      modeDisplayStart = currentMillis;
      break;
      
    case DOUBLE_CLICK:
      if (weatherHandler.isAvailable()) {
        currentMode = SHOW_WEATHER;
        modeDisplayStart = currentMillis;
        weatherHandler.resetDisplayState(currentMillis);
      }
      break;
      
    default:
      break;
  }
}

unsigned long getModeDuration(DisplayMode mode) {
  switch (mode) {
    case SHOW_DATE: return 5000;
    case SHOW_WEATHER: return 10000;
    default: return 0;
  }
}

const char* getDayOfWeekString(int day) {
  const char* DaysWeek[] = { "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT" };
  return DaysWeek[day];
}