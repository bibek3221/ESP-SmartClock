#include "ChristmasSong.h"

#define BUZZER D1
#define TOUCH_PIN D2

void setup() {
  pinMode(TOUCH_PIN, INPUT);
}

void loop() {
  if (digitalRead(TOUCH_PIN) == HIGH) {
    playChristmas(BUZZER);
    delay(500); // debounce
  }
}

