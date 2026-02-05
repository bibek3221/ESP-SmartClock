#ifndef SPECIAL_DATES_H
#define SPECIAL_DATES_H

#include <Arduino.h>

// Function declarations
void checkSpecialDates(byte month, byte day, int year, byte hour, byte minute);
void new_year(byte hour);
void happy(byte hour);
void crish(byte hour);
void independence_day(byte hour);
void republic_day(byte hour);

// Global time tracking
extern byte hour_, minute_, second_;

// Date checking implementation
void checkSpecialDates(byte month, byte day, int year, byte hour, byte minute)
{
  static bool jan1_called[4] = { false, false, false, false };
  static bool jan15_called[4] = { false, false, false, false };
  static bool jan26_called[4] = { false, false, false, false };
  static bool aug15_called[4] = { false, false, false, false };
  static bool feb4_called[4] = { false, false, false, false };
  static bool dec25_called[24] = { false };
  
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
    }
    for (int i = 0; i < 24; i++) {
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
            if (hour > 8 && minute > 0 && !missed8am) {
              new_year(hour);
              jan1_called[0] = true;
              missed8am = true;
            }
            if ((hour == 8 || hour == 12 || hour == 16 || hour == 20) && minute == 0) {
              new_year(hour);
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
      if (day == 5 && !feb4_called[0] && hour >= 8) {
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
      if (day == 25) {
        // Christmas: Show message EVERY HOUR when minute == 0 and second == 0
        if (minute == 0 && second_ == 0) {
          if (!dec25_called[hour]) {
            crish(hour);
            dec25_called[hour] = true;
          }
        }
      }
      break;
  }
}

#endif