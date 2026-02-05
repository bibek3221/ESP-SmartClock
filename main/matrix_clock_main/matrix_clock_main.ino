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
#include "Songs.h"
#include "special_dates.h"
#include "touch_handler.h"
#include "weather_handler.h"

// Hardware configuration
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4
#define CS_PIN 15

// Buzzer configuration - Passive buzzer
#define BUZZER_PIN 5      // D1 (GPIO5) - Tone output
#define BUZZER_GND 4      // D2 (GPIO4) - Ground reference
int lastBeepedHour = -1;  // Track last hour we beeped

// Song display variables
bool isSongPlaying = false;
unsigned long songStartTime = 0;
const char* currentSongMessage = "";
const unsigned long SONG_DURATION = 15000;  // Adjust based on your song length

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
int _hour , _minute;

// Display mode
DisplayMode currentMode = SHOW_TIME;
unsigned long modeDisplayStart = 0;

// Touch handler object
TouchHandler touchHandler(12);  // GPIO4

// Weather handler object
WeatherHandler weatherHandler;

// ===== SONG AND MESSAGE FUNCTIONS =====

// Function to show scrolling message using ALL 4 displays
void showSimpleScrollingMessage(const char* message) {
  Serial.print("Showing message: ");
  Serial.println(message);
  
  // Save current display configuration
  Display.setZone(0, 0, 3);  // Use ALL 4 modules for zone 0
  Display.setZone(1, 0, 0);  // Disable zone 1
  Display.setFont(0, NULL);  // Use default font for better readability
  
  // Clear display
  Display.displayClear(0);
  Display.displayClear(1);
  
  // Display scrolling message across all 4 modules
  Display.displayZoneText(0, message, PA_LEFT, 40, 0, PA_SCROLL_LEFT);
  
  // Animate for 5 seconds
  unsigned long startTime = millis();
  while (millis() - startTime < 5000) {
    Display.displayAnimate();
    delay(10);
  }
  
  // Restore original display configuration
  Display.setZone(0, 1, 3);  // Back to original: modules 1-3
  Display.setZone(1, 0, 0);  // Back to original: module 0
  Display.setFont(0, SmallDigits);  // Restore original font
}

// Function to start song with message
void startSongWithMessage(void (*songFunction)(), const char* message, byte hour) {
  if (hour == 8 || hour == 12 || hour == 16 || hour == 20 || hour > 8) {
    Serial.print("Starting song at hour: ");
    Serial.println(hour);
    
    // Show scrolling message first
    showSimpleScrollingMessage(message);
    
    // Setup for song display
    isSongPlaying = true;
    songStartTime = millis();
    currentSongMessage = message;
    
    // Configure display for song message
    Display.setZone(0, 0, 3);  // Use ALL 4 modules
    Display.setZone(1, 0, 0);  // Disable zone 1
    Display.setFont(0, NULL);  // Default font
    Display.displayZoneText(0, message, PA_CENTER, 50, 0, PA_NO_EFFECT);
    Display.displayAnimate();
    
    // Start playing song
    songFunction();
  }
}

// Function to check and update song display
void updateSongDisplay() {
  if (isSongPlaying) {
    // Keep message displayed while song is playing
    if (millis() - songStartTime < SONG_DURATION) {
      // Make the message blink to be noticeable
      static bool blinkState = false;
      static unsigned long lastBlink = 0;
      
      if (millis() - lastBlink > 500) {  // Blink every 500ms
        blinkState = !blinkState;
        lastBlink = millis();
        
        if (blinkState) {
          Display.displayZoneText(0, currentSongMessage, PA_CENTER, 50, 0, PA_NO_EFFECT);
        } else {
          // Show empty during blink off
          Display.displayZoneText(0, "     ", PA_CENTER, 50, 0, PA_NO_EFFECT);
        }
        Display.displayAnimate();
      }
    } else {
      // Song finished
      isSongPlaying = false;
      
      // Restore original display configuration
      Display.setZone(0, 1, 3);  // Back to original
      Display.setZone(1, 0, 0);  // Back to original
      Display.setFont(0, SmallDigits);
      Display.displayClear(0);
      Display.displayClear(1);
      
      // Restore time display
      currentMode = SHOW_TIME;
      updateDisplay();
    }
  }
}

// Helper function to play song at intervals
void playSongAtIntervals(void (*songFunction)(), const char* message, byte hour) {
  startSongWithMessage(songFunction, message, hour);
}

// ===== SPECIAL DATE FUNCTIONS =====

void new_year(byte hour) {
  Serial.println("Happy New Year!");
  playSongAtIntervals(playHappyNewYear, "HAPPY NEW YEAR!", hour);
}

void happy(byte hour) {
  Serial.println("Special Day Celebration!");
  playSongAtIntervals(playHappyBirthday, "HAPPY SPECIAL DAY!", hour);
}

void independence_day(byte hour) {
  Serial.println("Happy Independence Day!");
  playSongAtIntervals(playJanaGanaManaOriginalStereo, "HAPPY INDEPENDENCE DAY!", hour);
}

void republic_day(byte hour) {
  Serial.println("Happy Republic Day!");
  playSongAtIntervals(playJanaGanaManaOriginalStereo, "HAPPY REPUBLIC DAY!", hour);
}

void crish(byte hour) {
  Serial.println("Merry Christmas!");
  
  if (hour == 8 || hour == 12 || hour == 16 || hour == 20) {
    // Play full song at specific hours
    playSongAtIntervals(playChristmasCarol, "MERRY CHRISTMAS!", hour);
  } else {
    // Just show scrolling message for other hours
    showSimpleScrollingMessage("MERRY CHRISTMAS!");
  }
}

// ===== BUZZER FUNCTIONS =====

// Simple tone function for passive buzzer
void playTone(int frequency, int duration) {
  tone(BUZZER_PIN, frequency, duration);
  delay(duration);  // Wait for tone to finish
  noTone(BUZZER_PIN);
}

// Double beep for hourly indicator
void hourlyDoubleBeep() {
  // First beep
  playTone(800, 150);
  delay(100);
  // Second beep
  playTone(800, 150);
}

// Test beep on startup
void testBeep() {
  playTone(600, 100);
  delay(50);
  playTone(600, 100);
}

// ===== SETUP =====

void setup() {
  Serial.begin(115200);
  
  // Initialize display
  Display.begin(2);
  Display.setZone(0, 1, 3);  // Zone 0: modules 1-3 (for time HH:MM)
  Display.setZone(1, 0, 0);  // Zone 1: module 0 (for seconds SS)
  Display.setFont(0, SmallDigits);
  Display.setFont(1, SmallerDigits);
  Display.setIntensity(2);
  Display.setCharSpacing(0);
  
  // Initialize touch handler
  touchHandler.begin();
  
  // Initialize buzzer pins
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(BUZZER_GND, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(BUZZER_GND, LOW);  // Set as ground
  
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
  
  // Test beep on startup
  testBeep();
}

// ===== MAIN LOOP =====

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
    
    // Hourly beep - double beep at the start of each hour
    if (minute_ == 0 && second_ == 0) {
      // Only beep if it's a new hour (prevent multiple beeps in same hour)
      if (hour_ != lastBeepedHour) {
        hourlyDoubleBeep();
        lastBeepedHour = hour_;
      }
    }
    
    // Check for special dates (don't run if song is already playing)
    if (!isSongPlaying) {
      checkSpecialDates(month_, day_, year_, hour_, minute_);
    }
    
    // Update time strings
    updateTimeStrings();
    
    // Update display based on mode (only if not showing song)
    if (!isSongPlaying) {
      updateDisplay();
    }
    
    last_second = second_;
  }
  
  // Handle weather display cycling (only if not showing song)
  if (!isSongPlaying && currentMode == SHOW_WEATHER) {
    weatherHandler.updateDisplayState(currentMillis);
    Display.displayZoneText(0, weatherHandler.getLine1(), PA_LEFT, Display.getSpeed(), Display.getPause(), PA_NO_EFFECT);
    Display.displayZoneText(1, weatherHandler.getLine2(), PA_LEFT, Display.getSpeed(), Display.getPause(), PA_NO_EFFECT);
  }
  
  // Check if we should return to time display (only if not showing song)
  if (!isSongPlaying && currentMode != SHOW_TIME) {
    if (currentMillis - modeDisplayStart >= getModeDuration(currentMode)) {
      currentMode = SHOW_TIME;
    }
  }
  
  // Update song display if playing
  updateSongDisplay();
  
  Display.displayAnimate();
  delay(50);
}

// ===== HELPER FUNCTIONS =====

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