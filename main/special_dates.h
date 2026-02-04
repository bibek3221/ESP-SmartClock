
#ifndef SPECIAL_DATES_H
#define SPECIAL_DATES_H

#include <Arduino.h>
#include <MD_Parola.h>

extern MD_Parola Display;  // Declare external display object

// Function declarations
void checkSpecialDates(int month, int day, int year);
void new_year();
void happy();
void crish();

// Date checking implementation
void checkSpecialDates(int month, int day, int year) {
  static bool jan1_called = false;
  static bool jan15_called = false;
  static bool dec25_called = false;
  static int lastCheckedDay = 0;
  
  // Reset flags when day changes
  if (day != lastCheckedDay) {
    jan1_called = false;
    jan15_called = false;
    dec25_called = false;
    lastCheckedDay = day;
  }
  
  // Check specific dates
  switch (month) {
    case 1:  // January
      switch (day) {
        case 1:
          if (!jan1_called) {
            new_year();
            jan1_called = true;
          }
          break;
        case 15:
          if (!jan15_called) {
            happy();
            jan15_called = true;
          }
          break;
      }
      break;
      
    case 12:  // December
      if (day == 25 && !dec25_called) {
        crish();
        dec25_called = true;
      }
      break;
  }
}

// Special date functions
void new_year() {
  Serial.println("Happy New Year!");
  
  Display.displayZoneText(0, "HAPPY", PA_CENTER, 50, 1000, PA_SCROLL_LEFT);
  Display.displayZoneText(1, "2026!", PA_CENTER, 50, 1000, PA_SCROLL_LEFT);
  Display.displayAnimate();
  
  // Wait for animation
  while (!Display.displayAnimate()) {
    delay(10);
  }
  delay(2000);
}

void happy() {
  Serial.println("Happy January 15th!");
  
  Display.displayZoneText(0, "JAN", PA_CENTER, 50, 1000, PA_SCROLL_LEFT);
  Display.displayZoneText(1, "15TH", PA_CENTER, 50, 1000, PA_SCROLL_LEFT);
  Display.displayAnimate();
  
  while (!Display.displayAnimate()) {
    delay(10);
  }
  delay(2000);
}

void crish() {
  Serial.println("Merry Christmas!");
  
  Display.displayZoneText(0, "XMAS", PA_CENTER, 50, 1000, PA_SCROLL_LEFT);
  Display.displayZoneText(1, "JOY!", PA_CENTER, 50, 1000, PA_SCROLL_LEFT);
  Display.displayAnimate();
  
  while (!Display.displayAnimate()) {
    delay(10);
  }
  delay(2000);
}

#endif