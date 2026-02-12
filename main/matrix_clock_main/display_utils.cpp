#include "display_utils.h"
#include "Fonts.h"

// External display object (defined in main)
extern MD_Parola Display;

// Function to show scrolling message with configurable duration
void showScrollingMessage(const char* message, unsigned long duration) {
  Serial.print("Showing scrolling message: ");
  Serial.println(message);
  Serial.print("Duration: ");
  Serial.print(duration);
  Serial.println(" ms");

  // Configure display for scrolling
  configureDisplayForScrolling();
  
  // Clear display
  Display.displayClear(0);
  Display.displayClear(1);

  // Display scrolling message across all 4 modules
  Display.displayZoneText(0, message, PA_LEFT, 40, 0, PA_SCROLL_LEFT);

  // Animate for specified duration
  unsigned long startTime = millis();
  while (millis() - startTime < duration) {
    Display.displayAnimate();
    delay(10);
  }

  // Restore original display configuration
  restoreDisplayConfiguration();
}

// Function to show scrolling message that continues left to right for duration
// Message scrolls continuously without stopping
void showScrollingMessageContinuous(const char* message, unsigned long duration) {
  Serial.print("Showing continuous scrolling message: ");
  Serial.println(message);
  Serial.print("Duration: ");
  Serial.print(duration);
  Serial.println(" ms");

  // Configure display for scrolling
  configureDisplayForScrolling();
  
  // Clear display
  Display.displayClear(0);
  Display.displayClear(1);

  // Display scrolling message with SCROLL LEFT effect
  // We'll use a different approach for continuous scrolling
  Display.displayZoneText(0, message, PA_LEFT, 40, 0, PA_SCROLL_LEFT);
  
  unsigned long startTime = millis();
  int scrollCount = 0;
  
  while (millis() - startTime < duration) {
    // Animate the display
    if (Display.displayAnimate()) {
      // Animation completed one cycle
      scrollCount++;
      
      // Reset to start scrolling again
      Display.displayReset(0);
      
      // Optional: Slow down a bit between cycles
      delay(100);
    }
    delay(10);
  }

  // Restore original display configuration
  restoreDisplayConfiguration();
}

// Alternative: Even better continuous scrolling
void showContinuousScroll(const char* message, unsigned long duration) {
  Serial.print("Showing continuous scroll: ");
  Serial.println(message);
  
  configureDisplayForScrolling();
  Display.displayClear(0);
  Display.displayClear(1);
  
  // Calculate scroll speed based on message length
  int messageLength = strlen(message);
  int scrollSpeed = 40; // pixels per second
  
  // Show the message
  Display.displayZoneText(0, message, PA_LEFT, scrollSpeed, 0, PA_SCROLL_LEFT);
  
  unsigned long startTime = millis();
  while (millis() - startTime < duration) {
    // Continuously animate
    if (Display.displayAnimate()) {
      // When animation completes, reset it to continue scrolling
      Display.displayReset(0);
    }
    delay(10);
  }
  
  restoreDisplayConfiguration();
}

void showCenteredMessage(const char* line1, const char* line2, int duration) {
  Display.displayZoneText(0, line1, PA_CENTER, 50, duration, PA_NO_EFFECT);
  Display.displayZoneText(1, line2, PA_CENTER, 50, duration, PA_NO_EFFECT);
  Display.displayAnimate();
  
  if (duration > 0) {
    delay(duration);
  }
}


// Best version: Shows message scrolling continuously without gaps
void showMarqueeScroll(const char* message, unsigned long duration) {
  Serial.print("Showing marquee scroll: ");
  Serial.println(message);
  
  configureDisplayForScrolling();
  Display.displayClear(0);
  Display.displayClear(1);
  
  // Create a longer string by repeating the message with spaces
  // This creates a seamless continuous scroll
  String extendedMessage = String(message) + "     " + String(message) + "     " + String(message);
  
  // Use slower speed for better readability
  Display.displayZoneText(0, extendedMessage.c_str(), PA_LEFT, 30, 0, PA_SCROLL_LEFT);
  
  unsigned long startTime = millis();
  unsigned long lastResetTime = startTime;
  
  while (millis() - startTime < duration) {
    // Animate continuously
    if (Display.displayAnimate()) {
      // Reset animation after it completes
      Display.displayReset(0);
      lastResetTime = millis();
    }
    
    // Small delay to prevent CPU hogging
    delay(10);
  }
  
  restoreDisplayConfiguration();
}

// Configure display for scrolling messages
void configureDisplayForScrolling() {
  Display.setZone(0, 0, 3);  // Use ALL 4 modules for zone 0
  Display.setZone(1, 0, 0);  // Disable zone 1
  Display.setFont(0, NULL);  // Use default font for better readability
}

// Restore display to normal time configuration
void restoreDisplayConfiguration() {
  Display.setZone(0, 1, 3);         // Back to original: modules 1-3
  Display.setZone(1, 0, 0);         // Back to original: module 0
  Display.setFont(0, SmallDigits);  // Restore original font
  Display.setFont(1, SmallerDigits);// Restore seconds font
}