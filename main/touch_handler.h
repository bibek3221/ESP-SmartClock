
#ifndef TOUCH_HANDLER_H
#define TOUCH_HANDLER_H

#include <Arduino.h>

enum DisplayMode {
  SHOW_TIME,
  SHOW_DATE,
  SHOW_WEATHER
};

enum TouchEvent {
  NO_EVENT,
  SINGLE_CLICK,
  DOUBLE_CLICK
};

class TouchHandler {
private:
  uint8_t pin;
  bool lastState;
  unsigned long lastClickTime;
  int clickCount;
  
  // Constants
  static const unsigned long DOUBLE_CLICK_INTERVAL = 500;
  
public:
  TouchHandler(uint8_t touchPin) : pin(touchPin), lastState(LOW), lastClickTime(0), clickCount(0) {}
  
  void begin() {
    pinMode(pin, INPUT);
  }
  
  TouchEvent checkTouch(unsigned long currentMillis) {
    bool currentState = digitalRead(pin);
    TouchEvent event = NO_EVENT;
    
    // Detect rising edge
    if (currentState == HIGH && lastState == LOW) {
      unsigned long timeSinceLastClick = currentMillis - lastClickTime;
      
      if (timeSinceLastClick < DOUBLE_CLICK_INTERVAL) {
        // Double-click detected
        event = DOUBLE_CLICK;
        clickCount = 0;  // Reset immediately for double-click
      } else {
        // First click detected
        clickCount = 1;
      }
      
      lastClickTime = currentMillis;
    }
    lastState = currentState;
    
    // Check for single click (after waiting for potential double-click)
    if (clickCount == 1 && (currentMillis - lastClickTime > DOUBLE_CLICK_INTERVAL)) {
      event = SINGLE_CLICK;
      clickCount = 0;
    }
    
    return event;
  }
};

#endif