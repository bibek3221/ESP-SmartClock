#ifndef SPECIAL_DATES_H
#define SPECIAL_DATES_H

#include "Songs.h"
#include <Arduino.h>
#include <MD_Parola.h>

extern MD_Parola Display;  // Declare external display object

// Function declarations
void checkSpecialDates(byte month, byte day, int year, byte hour, byte minute);
void new_year(byte hour);
void happy(byte hour);
void crish(byte hour);
void independence_day(byte hour);
void republic_day(byte hour);
void playSongAtIntervals(void (*songFunction)(), const char* message1, const char* message2, byte hour);

// Global time tracking
extern byte hour_, minute_;  // Declare external time variables

// Date checking implementation
void checkSpecialDates(byte month, byte day, int year, byte hour, byte minute)
{
  static bool jan1_called[4] = { false, false, false, false };  // 8,12,16,20 hour slots
  static bool jan15_called[4] = { false, false, false, false };
  static bool jan26_called[4] = { false, false, false, false };
  static bool aug15_called[4] = { false, false, false, false };
  static bool feb4_called[4] = { false, false, false, false };
  static bool dec25_called[4] = { false, false, false, false };

  static int lastCheckedDay = 0;
  static bool missed8am = false;

  // Reset flags when day changes
  if (day != lastCheckedDay) {
    for (int i = 0; i < 4; i++) {
      jan1_called[i] = false;
      jan15_called[i] = false;
      jan26_called[i] = false;
      aug15_called[i] = false;
      feb4_called[i] = false;
      dec25_called[i] = false;
    }
    missed8am = false;
    lastCheckedDay = day;
  }

  // Check specific dates
  switch (month) {
    case 1:  // January
      switch (day) {
        case 1:
          if (!jan1_called[0] && hour >= 8) {
            // If we missed 8 AM, play immediately when checking starts
            if (hour > 8 && minute > 0 && !missed8am) {
              new_year(hour);
              jan1_called[0] = true;
              missed8am = true;
            }
            // Check for scheduled times
            if ((hour == 8 || hour == 12 || hour == 16 || hour == 20) && minute == 0) {
              new_year(hour);
              // Mark the corresponding time slot as played
              if (hour == 8) jan1_called[0] = true;
              else if (hour == 12) jan1_called[1] = true;
              else if (hour == 16) jan1_called[2] = true;
              else if (hour == 20) jan1_called[3] = true;
            }
          }
          break;

        case 15:
          if (!jan15_called[0] && hour >= 8) {
            if (hour > 8 && minute > 0 && !missed8am) {
              happy(hour);
              jan15_called[0] = true;
              missed8am = true;
            }
            if ((hour == 8 || hour == 12 || hour == 16 || hour == 20) && minute == 0) {
              happy(hour);
              if (hour == 8) jan15_called[0] = true;
              else if (hour == 12) jan15_called[1] = true;
              else if (hour == 16) jan15_called[2] = true;
              else if (hour == 20) jan15_called[3] = true;
            }
          }
          break;

        case 26:
          if (!jan26_called[0] && hour >= 8) {
            if (hour > 8 && minute > 0 && !missed8am) {
              republic_day(hour);
              jan26_called[0] = true;
              missed8am = true;
            }
            if ((hour == 8 || hour == 12 || hour == 16 || hour == 20) && minute == 0) {
              republic_day(hour);
              if (hour == 8) jan26_called[0] = true;
              else if (hour == 12) jan26_called[1] = true;
              else if (hour == 16) jan26_called[2] = true;
              else if (hour == 20) jan26_called[3] = true;
            }
          }
          break;
      }
      break;

    case 2:  // February
      if (day == 4 && !feb4_called[0] && hour >= 8) {
        if (hour > 8 && minute > 0 && !missed8am) {
          happy(hour);
          feb4_called[0] = true;
          missed8am = true;
        }
        if ((hour == 8 || hour == 12 || hour == 16 || hour == 20) && minute == 0) {
          happy(hour);
          if (hour == 8) feb4_called[0] = true;
          else if (hour == 12) feb4_called[1] = true;
          else if (hour == 16) feb4_called[2] = true;
          else if (hour == 20) feb4_called[3] = true;
        }
      }
      break;

    case 8:  // August
      if (day == 15 && !aug15_called[0] && hour >= 8) {
        if (hour > 8 && minute > 0 && !missed8am) {
          independence_day(hour);
          aug15_called[0] = true;
          missed8am = true;
        }
        if ((hour == 8 || hour == 12 || hour == 16 || hour == 20) && minute == 0) {
          independence_day(hour);
          if (hour == 8) aug15_called[0] = true;
          else if (hour == 12) aug15_called[1] = true;
          else if (hour == 16) aug15_called[2] = true;
          else if (hour == 20) aug15_called[3] = true;
        }
      }
      break;

    case 12:  // December
      if (day == 25 && !dec25_called[0] && hour >= 8) {
        if (hour > 8 && minute > 0 && !missed8am) {
          crish(hour);
          dec25_called[0] = true;
          missed8am = true;
        }
        if ((hour == 8 || hour == 12 || hour == 16 || hour == 20) && minute == 0) {
          crish(hour);
          if (hour == 8) dec25_called[0] = true;
          else if (hour == 12) dec25_called[1] = true;
          else if (hour == 16) dec25_called[2] = true;
          else if (hour == 20) dec25_called[3] = true;
        }
      }
      break;
  }
}

// Helper function to play song at intervals
void playSongAtIntervals(void (*songFunction)(), const char* message1, const char* message2, byte hour) {
  // Only play during specific hours (8, 12, 16, 20)
  if (hour == 8 || hour == 12 || hour == 16 || hour == 20 || hour > 8) {
    Serial.print("Playing at hour: ");
    Serial.println(hour);

    // Display message
    Display.displayZoneText(0, message1, PA_CENTER, 50, 1000, PA_SCROLL_LEFT);
    Display.displayZoneText(1, message2, PA_CENTER, 50, 1000, PA_SCROLL_LEFT);
    Display.displayAnimate();

    // Wait for animation to start
    while (!Display.displayAnimate()) {
      delay(10);
    }

    // Play the full song
    songFunction();

    // Keep displaying message while song plays
    delay(2000);
  }
}

// Special date functions (modified to accept hour parameter)
void new_year(byte hour) {
  Serial.println("Happy New Year!");
  playSongAtIntervals(playHappyNewYear, "HAPPY", "NEW YEAR!", hour);
}

void happy(byte hour) {
  Serial.println("Special Day Celebration!");
  playSongAtIntervals(playHappyBirthday, "CELEBRATE", "SPECIAL DAY!", hour);
}

void independence_day(byte hour) {
  Serial.println("Happy Independence Day!");
  playSongAtIntervals(playJanaGanaManaOriginalStereo, "INDEPENDENCE", "DAY!", hour);
}

void republic_day(byte hour) {
  Serial.println("Happy Republic Day!");
  playSongAtIntervals(playJanaGanaManaOriginalStereo, "REPUBLIC", "DAY!", hour);
}

void crish(byte hour) {
  Serial.println("Merry Christmas!");
  playSongAtIntervals(playChristmasCarol, "MERRY", "CHRISTMAS!", hour);
}

#endif