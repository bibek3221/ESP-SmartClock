/*
 ESP8266 Music Player - Simple Modular System
 Just call playSongName() to play any song!
 
 Buzzer Left:  D5 (GPIO14)
 Buzzer Right: D6 (GPIO12)
*/

#include "Songs.h"

void setup() {
  Serial.begin(115200);
  Serial.println("\n🎵 ESP8266 Music Player 🎵");
  Serial.println("==========================");
  
  delay(1000);
  
  // Demo: Play all songs
  Serial.println("\n1. Playing Happy Birthday...");
  // playHappyBirthday();
  delay(2000);
  
  Serial.println("\n2. Playing Christmas Carol...");
  // playChristmasCarol();
  delay(2000);
  
  Serial.println("\n3. Playing Happy New Year...");
  // playHappyNewYear();
  delay(2000);
  
  
  Serial.println("\n6. Playing Jana Gana Mana (ORIGINAL - Stereo)...");
  // playJanaGanaManaOriginalStereo();
  delay(2000);
  
  Serial.println("\n7. Playing Happy Anniversary...");
  // playHappyAnniversary();
  
  Serial.println("\n✅ All songs played!");
  Serial.println("\nYou can now call these functions from anywhere:");
  Serial.println("  playHappyBirthday()");
  Serial.println("  playChristmasCarol()");
  Serial.println("  playHappyNewYear()");
  Serial.println("  playJanaGanaMana() - Standard version");
  Serial.println("  playJanaGanaManaOriginal() - Original sound");
  Serial.println("  playJanaGanaManaOriginalStereo() - Original with stereo");
  Serial.println("  playHappyAnniversary()");
}

void loop() {
  // Your main code here
  // Example: play songs based on conditions
  
  // Uncomment to play random song every 10 seconds
  /*
  static unsigned long lastPlay = 0;
  if (millis() - lastPlay > 10000) {
    int song = random(0, 7);
    switch(song) {
      case 0: playHappyBirthday(); break;
      case 1: playChristmasCarol(); break;
      case 2: playHappyNewYear(); break;
      case 3: playJanaGanaMana(); break;
      case 4: playJanaGanaManaOriginal(); break;
      case 5: playJanaGanaManaOriginalStereo(); break;
      case 6: playHappyAnniversary(); break;
    }
    lastPlay = millis();
  }
  */
}