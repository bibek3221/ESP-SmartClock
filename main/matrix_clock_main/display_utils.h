
#ifndef DISPLAY_UTILS_H
#define DISPLAY_UTILS_H

#include <MD_Parola.h>
#include <Arduino.h>

// Declare external display object (defined in main)
extern MD_Parola Display;

// Function declarations
void showScrollingMessage(const char* message, unsigned long duration = 5000);
void showScrollingMessageContinuous(const char* message, unsigned long duration = 5000);
void showContinuousScroll(const char* message, unsigned long duration = 5000);
void showMarqueeScroll(const char* message, unsigned long duration = 5000);
void showCenteredMessage(const char* line1, const char* line2, int duration = 2000);
// void showLeftAlignedMessage(const char* line1, const char* line2, unsigned long duration = 2000);
void restoreDisplayConfiguration();
void configureDisplayForScrolling();

#endif