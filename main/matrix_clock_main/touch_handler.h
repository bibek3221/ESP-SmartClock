#ifndef TOUCH_HANDLER_H
#define TOUCH_HANDLER_H

#include <Arduino.h>

enum TouchEvent {
  NONE,          // Make sure this is NONE, not NO_EVENT
  SINGLE_CLICK,
  DOUBLE_CLICK,
  LONG_PRESS     // Add this
};

class TouchHandler {
  private:
    int touchPin;
    unsigned long lastTouchTime = 0;
    unsigned long touchStartTime = 0;
    bool touchActive = false;
    int clickCount = 0;
    const unsigned long DEBOUNCE_DELAY = 50;
    const unsigned long DOUBLE_CLICK_DELAY = 300;
    const unsigned long LONG_PRESS_DELAY = 1000;
    
  public:
    TouchHandler(int pin) : touchPin(pin) {}
    
    void begin() {
      pinMode(touchPin, INPUT);
    }
    
    TouchEvent checkTouch(unsigned long currentMillis) {
      int touchState = digitalRead(touchPin);
      TouchEvent event = NONE;  // Changed from NO_EVENT to NONE
      
      if (touchState == HIGH && !touchActive) {
        // Touch started
        touchStartTime = currentMillis;
        touchActive = true;
      } 
      else if (touchState == LOW && touchActive) {
        // Touch ended
        touchActive = false;
        unsigned long touchDuration = currentMillis - touchStartTime;
        
        if (touchDuration >= LONG_PRESS_DELAY) {
          clickCount = 0;
          return LONG_PRESS;
        } else if (touchDuration >= DEBOUNCE_DELAY) {
          clickCount++;
          lastTouchTime = currentMillis;
        }
      }
      
      // Check for double click timeout
      if (clickCount > 0 && (currentMillis - lastTouchTime) >= DOUBLE_CLICK_DELAY) {
        event = (clickCount == 1) ? SINGLE_CLICK : DOUBLE_CLICK;
        clickCount = 0;
        return event;
      }
      
      return NONE;  // Changed from NO_EVENT to NONE
    }
};

#endif