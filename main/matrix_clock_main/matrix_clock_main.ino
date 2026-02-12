#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include "Fonts.h"
#include <SPI.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <TimeLib.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

// Include modular files
#include "Songs.h"
#include "special_dates.h"
#include "touch_handler.h"
#include "weather_handler.h"
#include "wifi_manager.h"
#include "display_utils.h"

// ===== DISPLAY MODE ENUM =====
enum DisplayMode {
  SHOW_TIME,
  SHOW_DATE,
  SHOW_WEATHER
};

// Hardware configuration
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4
#define CS_PIN 15

// Buzzer configuration - Passive buzzer
#define BUZZER_PIN 5      // D1 (GPIO5) - Tone output
#define BUZZER_GND 4      // D2 (GPIO4) - Ground reference
int lastBeepedHour = -1;  // Track last hour we beeped

// Global song state variables (from Songs.h)
extern bool isSongPlaying;
extern unsigned long songStartTime;
extern const char* currentSongMessage;
extern const unsigned long SONG_DURATION;

// Additional song state
bool songDisplayActive = false;
unsigned long songMessageStart = 0;
const char* activeSongMessage = "";

// Global objects
MD_Parola Display = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);
WiFiClockManager wifiManager;


// Time variables
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "asia.pool.ntp.org", 19800, 60000);
char Time[] = "00:00";
char Seconds[] = "00";
char Date[] = "00-00-2000";
byte last_second, second_, minute_, hour_, day_, month_;
int year_;
// bool isResetting = false;

// Display mode
DisplayMode currentMode = SHOW_TIME;
unsigned long modeDisplayStart = 0;

// Touch handler object
TouchHandler touchHandler(12);  // GPIO4

// Weather handler object
WeatherHandler weatherHandler;

void showLongPressFeedback(int percent) {
  // Show progress on display (optional)
  char progress[6];
  sprintf(progress, "%d%%", percent);
  Display.displayZoneText(0, "RESET WIFI", PA_CENTER, 50, 0, PA_NO_EFFECT);
  Display.displayZoneText(1, progress, PA_CENTER, 50, 0, PA_NO_EFFECT);
  Display.displayAnimate();
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

// ===== SONG HELPER FUNCTIONS =====

void startSongWithDisplay(const char* message) {
  Serial.print("Starting song display: ");
  Serial.println(message);

  songDisplayActive = true;
  songMessageStart = millis();
  activeSongMessage = message;

  // Configure display for song message
  Display.setZone(0, 0, 3);
  Display.setZone(1, 0, 0);
  Display.setFont(0, NULL);
  Display.displayClear(0);
  Display.displayClear(1);

  // 🔽 ADD THESE 3 LINES 🔽
  isSongPlaying = true;
  currentSongMessage = message;
  songStartTime = millis();
}

void restoreNormalDisplay() {
  Display.setZone(0, 1, 3);           // Back to original: modules 1-3
  Display.setZone(1, 0, 0);           // Back to original: module 0
  Display.setFont(0, SmallDigits);    // Restore original font
  Display.setFont(1, SmallerDigits);  // Restore seconds font
  Display.displayClear(0);
  Display.displayClear(1);
  currentMode = SHOW_TIME;
}

void updateSongDisplay() {
  if (isSongPlaying) {
    // Make the message blink to be noticeable
    static bool blinkState = false;
    static unsigned long lastBlink = 0;

    if (millis() - lastBlink > 500) {  // Blink every 500ms
      blinkState = !blinkState;
      lastBlink = millis();

      if (blinkState) {
        // Show song message
        Display.displayZoneText(0, currentSongMessage, PA_CENTER, 0, 0, PA_NO_EFFECT);
      } else {
        // Show empty during blink off
        Display.displayZoneText(0, "     ", PA_CENTER, 0, 0, PA_NO_EFFECT);
      }
    }

    // Check if song should be done (time-based)
    if (millis() - songStartTime >= SONG_DURATION) {
      isSongPlaying = false;
      stopMelodyPlayback();    // This now restores display
      restoreNormalDisplay();  // <-- ADD THIS LINE
      Serial.println("Song finished (timeout)");
    }
  }
}

void stopSongDisplay() {
  songDisplayActive = false;
  isSongPlaying = false;
  stopMelodyPlayback();

  // Restore normal display configuration
  Display.setZone(0, 1, 3);           // Back to original: modules 1-3
  Display.setZone(1, 0, 0);           // Back to original: module 0
  Display.setFont(0, SmallDigits);    // Restore original font
  Display.setFont(1, SmallerDigits);  // Restore seconds font
  Display.displayClear(0);
  Display.displayClear(1);

  // Restore time display
  currentMode = SHOW_TIME;
  Serial.println("Song display ended");
}

// ===== SPECIAL DATE FUNCTIONS (UPDATED) =====

void new_year(byte hour) {
  Serial.println("Happy New Year!");
  if (!isSongPlaying && !songDisplayActive) {
    startSongWithDisplay("HAPPY NEW YEAR!");
    playHappyNewYear();
  }
}

void happy(byte hour) {
  Serial.println("Special Day Celebration!");
  if (!isSongPlaying && !songDisplayActive) {
    startSongWithDisplay("HAPPY SPECIAL DAY!");
    playHappyBirthday();
  }
}

void independence_day(byte hour) {
  Serial.println("Happy Independence Day!");
  if (!isSongPlaying && !songDisplayActive) {
    startSongWithDisplay("HAPPY INDEPENDENCE DAY!");
    playJanaGanaManaOriginalStereo();
  }
}

void republic_day(byte hour) {
  Serial.println("Happy Republic Day!");
  if (!isSongPlaying && !songDisplayActive) {
    startSongWithDisplay("HAPPY REPUBLIC DAY!");
    playJanaGanaManaOriginalStereo();
  }
}

void crish(byte hour) {
  Serial.println("Merry Christmas!");
  if (!isSongPlaying && !songDisplayActive) {
    if (hour == 8 || hour == 12 || hour == 16 || hour == 20) {
      // Play full song at specific hours
      startSongWithDisplay("MERRY CHRISTMAS!");
      playChristmasCarol();
    } else {
      // Just show scrolling message for other hours
      showContinuousScroll("MERRY CHRISTMAS!", 3000);
    }
  }
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
  Display.setIntensity(1);
  Display.setCharSpacing(0);

  Display.displayClear(0);
  Display.displayClear(1);

  // Initialize touch handler
  touchHandler.begin();

  // Initialize buzzer pins
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(BUZZER_GND, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(BUZZER_GND, LOW);  // Set as ground

  // Initialize WiFi Manager
  wifiManager.begin(Display);

  // Show connected message
  if (wifiManager.isConfigured()) {
    String ip = wifiManager.getIPAddress();
    int lastDot = ip.lastIndexOf('.');
    String lastOctet = ip.substring(lastDot + 1);

    showScrollingMessage("Connected", 4000);
    Display.displayAnimate();
    delay(2000);
  }

  // Initialize time client
  timeClient.begin();

  // Initialize weather handler
  weatherHandler.begin();

  // Test beep on startup
  testBeep();
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

// void updateDisplay() {
//   switch (currentMode) {
//     case SHOW_DATE:
//       Display.displayZoneText(0, getDayOfWeekString(timeClient.getDay()), PA_LEFT, Display.getSpeed(), Display.getPause(), PA_NO_EFFECT);
//       Display.displayZoneText(1, Date, PA_LEFT, Display.getSpeed(), Display.getPause(), PA_NO_EFFECT);
//       break;

//     case SHOW_WEATHER:
//       Display.displayZoneText(0, weatherHandler.getLine1(), PA_LEFT, Display.getSpeed(), Display.getPause(), PA_NO_EFFECT);
//       Display.displayZoneText(1, weatherHandler.getLine2(), PA_LEFT, Display.getSpeed(), Display.getPause(), PA_NO_EFFECT);
//       break;

//     case SHOW_TIME:
//     default:
//       Display.displayZoneText(0, Time, PA_LEFT, Display.getSpeed(), Display.getPause(), PA_NO_EFFECT);
//       Display.displayZoneText(1, Seconds, PA_LEFT, Display.getSpeed(), Display.getPause(), PA_NO_EFFECT);
//       break;
//   }
// }
void updateDisplay() {
  // ONLY update time display – weather/date are handled separately
  if (currentMode == SHOW_TIME) {
    Display.setZone(0, 1, 3);
    Display.setZone(1, 0, 0);
    Display.setFont(0, SmallDigits);
    Display.setFont(1, SmallerDigits);
    Display.displayZoneText(0, Time, PA_LEFT, Display.getSpeed(), Display.getPause(), PA_NO_EFFECT);
    Display.displayZoneText(1, Seconds, PA_LEFT, Display.getSpeed(), Display.getPause(), PA_NO_EFFECT);
  }
}


// ===== UPDATED TOUCH HANDLER =====

void handleTouchEvent(TouchEvent event, unsigned long currentMillis) {
  if (event == NONE) return;
  Serial.print("Touch event: ");

  switch (event) {
    case SINGLE_CLICK:
      if (!isSongPlaying && !songDisplayActive) {
        Serial.println("SINGLE_CLICK - Show date");
        currentMode = SHOW_DATE;
        modeDisplayStart = currentMillis;

        // 🔥 FORCE CLEAN CONFIG AND DRAW IMMEDIATELY
        Display.setZone(0, 1, 3);  // big digits zone
        Display.setZone(1, 0, 0);  // small digits zone
        Display.setFont(0, SmallDigits);
        Display.setFont(1, SmallerDigits);
        Display.displayClear(0);
        Display.displayClear(1);

        // Show date on big digits (zone0), day on small digits (zone1)
        char dateStr[9];
        sprintf(dateStr, "%02d-%02d", day_, month_);
        Display.displayZoneText(0, dateStr, PA_LEFT, 0, 0, PA_NO_EFFECT);
        Display.displayZoneText(1, getDayOfWeekString(timeClient.getDay()), PA_LEFT, 0, 0, PA_NO_EFFECT);
        Display.displayAnimate();
      }
      break;

    case DOUBLE_CLICK:
      Serial.println("DOUBLE_CLICK - Quick action");
      showScrollingMessage("DOUBLE TAP", 2000);
      break;

    case LONG_PRESS:
      if (!isSongPlaying && !songDisplayActive) {
        Serial.println("LONG_PRESS - Show weather");

        // 🔥 RESET DISPLAY FOR WEATHER
        Display.setZone(0, 1, 3);  // big digits zone
        Display.setZone(1, 0, 0);  // small digits zone
        Display.setFont(0, SmallDigits);
        Display.setFont(1, SmallerDigits);
        Display.displayClear(0);
        Display.displayClear(1);
        Display.displayAnimate();

        if (weatherHandler.isAvailable()) {
          currentMode = SHOW_WEATHER;
          modeDisplayStart = currentMillis;
          weatherHandler.resetDisplayState(currentMillis);

          // Draw weather immediately
          Display.displayZoneText(0, weatherHandler.getLine1(), PA_LEFT, 0, 0, PA_NO_EFFECT);
          Display.displayZoneText(1, weatherHandler.getLine2(), PA_LEFT, 0, 0, PA_NO_EFFECT);
          Display.displayAnimate();
        } else {
          // Show error message using all modules
          Display.setZone(0, 0, 3);
          Display.setFont(0, NULL);
          Display.displayZoneText(0, "NO WEATHER", PA_CENTER, 0, 2000, PA_NO_EFFECT);
          Display.displayAnimate();
          delay(2000);
          restoreNormalDisplay();
        }
      }
      break;

    case WIFI_RESET:
      Serial.println("WIFI_RESET - Reset WiFi");
      handleWiFiReset();
      break;

    default: break;
  }
}

// ===== MAIN LOOP =====

void loop() {
  // Update time
  timeClient.update();
  updateSongDisplay();
  // updateMelodyPlayback();
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
    if (!isSongPlaying && !songDisplayActive) {
      checkSpecialDates(month_, day_, year_, hour_, minute_);
    }

    // Update time strings
    updateTimeStrings();

    // Update display based on mode (only if not showing song)
    if (!songDisplayActive) {
      updateDisplay();
    }

    last_second = second_;
  }

  // ===== KEY CHANGE: NON-BLOCKING SONG SYSTEM =====

  // 1. Update melody playback (call ONCE per loop)
  bool melodyDone = updateMelodyPlayback();
  if (melodyDone && isSongPlaying) {
    // Melody finished
    stopMelodyPlayback();
    restoreNormalDisplay();  // 🔥 THIS RESTORES THE CLOCK
    isSongPlaying = false;
    songDisplayActive = false;
    Serial.println("Melody finished – display restored");
  }

  // 2. Update song display if active
  if (songDisplayActive) {
    updateSongDisplay();

    // Also update the display animation
    Display.displayAnimate();
  }

  // 3. Handle weather display (only if no song)
  // 3. Handle weather display (only if no song) - THROTTLED
  static unsigned long lastWeatherUpdate = 0;
  if (!songDisplayActive && currentMode == SHOW_WEATHER) {
    static unsigned long lastWeatherUpdate = 0;
    if (currentMillis - lastWeatherUpdate >= 200) {
      weatherHandler.updateDisplayState(currentMillis);

      // --- USE THE SAME FONTS AS TIME DISPLAY ---
      Display.setZone(0, 1, 3);  // modules 1-3 for big digits
      Display.setZone(1, 0, 0);  // module 0 for small label
      Display.setFont(0, SmallDigits);
      Display.setFont(1, SmallerDigits);

      // Show number on zone0, label on zone1
      Display.displayZoneText(0, weatherHandler.getLine1(), PA_LEFT, 0, 0, PA_NO_EFFECT);
      Display.displayZoneText(1, weatherHandler.getLine2(), PA_LEFT, 0, 0, PA_NO_EFFECT);

      lastWeatherUpdate = currentMillis;
    }
  }

  // 4. Handle mode timeout (only if no song)
  if (!songDisplayActive && currentMode != SHOW_TIME) {
    if (currentMillis - modeDisplayStart >= getModeDuration(currentMode)) {
      currentMode = SHOW_TIME;
      restoreNormalDisplay();
    }
  }

  // 3. Handle weather display (only if no song)
  static unsigned long lastWeatherUpdate = 0;
  if (!songDisplayActive && currentMode == SHOW_WEATHER) {
    if (currentMillis - lastWeatherUpdate >= 200) {
      weatherHandler.updateDisplayState(currentMillis);

      // ENSURE CORRECT ZONES/FONTS
      Display.setZone(0, 1, 3);
      Display.setZone(1, 0, 0);
      Display.setFont(0, SmallDigits);
      Display.setFont(1, SmallerDigits);

      Display.displayZoneText(0, weatherHandler.getLine1(), PA_LEFT, 0, 0, PA_NO_EFFECT);
      Display.displayZoneText(1, weatherHandler.getLine2(), PA_LEFT, 0, 0, PA_NO_EFFECT);

      lastWeatherUpdate = currentMillis;
    }
  }

  // Always animate the display (except during song display where we handle it separately)
  if (!songDisplayActive) {
    Display.displayAnimate();
  }

  delay(50);
}

// Update handleWiFiReset to prevent multiple calls:
void handleWiFiReset() {
  Serial.println("=== Starting WiFi Reset ===");

  // Clear display first
  Display.displayClear(0);
  Display.displayClear(1);

  // Step 1: Show immediate feedback
  Display.displayZoneText(0, "WIFI", PA_CENTER, 50, 0, PA_NO_EFFECT);
  Display.displayZoneText(1, "RESET", PA_CENTER, 50, 0, PA_NO_EFFECT);
  Display.displayAnimate();
  delay(2000);

  // Step 2: Clear WiFi settings
  Display.displayZoneText(0, "CLEARING", PA_CENTER, 50, 0, PA_NO_EFFECT);
  Display.displayZoneText(1, "SETTINGS", PA_CENTER, 50, 0, PA_NO_EFFECT);
  Display.displayAnimate();
  delay(1000);

  wifiManager.resetSettings();

  // Step 3: Show reset complete
  Display.displayZoneText(0, "SETTINGS", PA_CENTER, 50, 0, PA_NO_EFFECT);
  Display.displayZoneText(1, "CLEARED", PA_CENTER, 50, 0, PA_NO_EFFECT);
  Display.displayAnimate();
  delay(2000);

  // Step 4: Show reboot message
  Display.displayZoneText(0, "REBOOTING", PA_CENTER, 50, 0, PA_NO_EFFECT);
  Display.displayZoneText(1, "DEVICE", PA_CENTER, 50, 0, PA_NO_EFFECT);
  Display.displayAnimate();
  delay(1000);

  // Step 5: Quick countdown
  for (int i = 3; i > 0; i--) {
    char countStr[4];
    sprintf(countStr, "%d", i);
    Display.displayZoneText(0, "REBOOT IN", PA_CENTER, 50, 0, PA_NO_EFFECT);
    Display.displayZoneText(1, countStr, PA_CENTER, 50, 0, PA_NO_EFFECT);
    Display.displayAnimate();
    delay(1000);
  }

  // Step 6: Final message
  Display.displayZoneText(0, "CONNECT TO", PA_CENTER, 50, 0, PA_NO_EFFECT);
  Display.displayZoneText(1, "SETUP AP", PA_CENTER, 50, 0, PA_NO_EFFECT);
  Display.displayAnimate();
  delay(2000);

  // Clear display before reboot
  Display.displayClear(0);
  Display.displayClear(1);
  Display.displayAnimate();
  delay(500);

  Serial.println("=== Rebooting ===");
  ESP.reset();
  delay(5000);
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