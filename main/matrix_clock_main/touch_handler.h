#ifndef TOUCH_HANDLER_H
#define TOUCH_HANDLER_H

#include <Arduino.h>

enum TouchEvent {
  NONE,
  SINGLE_CLICK,
  DOUBLE_CLICK,
  LONG_PRESS,
  WIFI_RESET
};

class TouchHandler {
  private:
    int touchPin;
    unsigned long lastTouchTime = 0;
    unsigned long touchStartTime = 0;
    bool touchActive = false;
    int clickCount = 0;
    bool longPressDetected = false;
    bool wifiResetDetected = false;
    
    // Timing constants
    const unsigned long DEBOUNCE_TIME = 50;
    const unsigned long CLICK_TIME = 400;      // Max time for a click
    const unsigned long DOUBLE_CLICK_TIME = 400; // Time between clicks
    const unsigned long LONG_PRESS_TIME = 2000;  // 2 seconds
    const unsigned long WIFI_RESET_TIME = 7000;  // 7 seconds
    
    // Debounce tracking
    unsigned long lastDebounceTime = 0;
    int lastStableState = LOW;
    int currentState = LOW;
    
  public:
    TouchHandler(int pin) : touchPin(pin) {}
    
    void begin() {
      pinMode(touchPin, INPUT);
      lastStableState = digitalRead(touchPin);
    }
    
    TouchEvent checkTouch(unsigned long currentMillis) {
      int reading = digitalRead(touchPin);
      
      // Debounce logic
      if (reading != lastStableState) {
        lastDebounceTime = currentMillis;
      }
      
      if ((currentMillis - lastDebounceTime) > DEBOUNCE_TIME) {
        currentState = reading;
      }
      
      lastStableState = reading;
      
      // Handle touch start
      if (currentState == HIGH && !touchActive) {
        touchStartTime = currentMillis;
        touchActive = true;
        longPressDetected = false;
        wifiResetDetected = false;
        Serial.println("[TOUCH] START");
      }
      
      // Handle touch end
      if (currentState == LOW && touchActive) {
        touchActive = false;
        unsigned long duration = currentMillis - touchStartTime;
        
        Serial.print("[TOUCH] END - Duration: ");
        Serial.println(duration);
        
        // Check for WiFi reset (detected during press)
        if (wifiResetDetected) {
          Serial.println("[EVENT] WIFI_RESET");
          return WIFI_RESET;
        }
        
        // Check for long press (detected during press)
        if (longPressDetected) {
          Serial.println("[EVENT] LONG_PRESS");
          return LONG_PRESS;
        }
        
        // Check for click (short press)
        if (duration < CLICK_TIME) {
          clickCount++;
          lastTouchTime = currentMillis;
          Serial.print("[TOUCH] Click count: ");
          Serial.println(clickCount);
        }
      }
      
      // Handle active touch (checking for long presses)
      if (touchActive) {
        unsigned long duration = currentMillis - touchStartTime;
        
        // Check for WiFi reset
        if (duration >= WIFI_RESET_TIME && !wifiResetDetected) {
          wifiResetDetected = true;
          Serial.println("[TOUCH] WiFi reset threshold reached");
        }
        // Check for long press
        else if (duration >= LONG_PRESS_TIME && !longPressDetected) {
          longPressDetected = true;
          Serial.println("[TOUCH] Long press threshold reached");
        }
      }
      
      // Check for double click timeout
      if (clickCount > 0 && (currentMillis - lastTouchTime) > DOUBLE_CLICK_TIME) {
        TouchEvent event;
        if (clickCount == 1) {
          event = SINGLE_CLICK;
          Serial.println("[EVENT] SINGLE_CLICK");
        } else {
          event = DOUBLE_CLICK;
          Serial.println("[EVENT] DOUBLE_CLICK");
        }
        clickCount = 0;
        return event;
      }
      
      return NONE;
    }
};

#endif