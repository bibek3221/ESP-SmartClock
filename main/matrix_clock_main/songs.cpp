#include "Songs.h"
#include "Notes.h"
#include <Arduino.h>  // ADD THIS LINE

// Buzzer pins (must match main program)
#define BUZZER_LEFT 05  // D2
#define BUZZER_RIGHT 04  // D1


// ============================================
// HAPPY BIRTHDAY MELODY
// ============================================
const int happyBirthdayMelody[] = {
  NOTE_C4,4, NOTE_C4,8, 
  NOTE_D4,4, NOTE_C4,4, NOTE_F4,4,
  NOTE_E4,2,
  NOTE_C4,4, NOTE_C4,8,
  NOTE_D4,4, NOTE_C4,4, NOTE_G4,4,
  NOTE_F4,2,
  NOTE_C4,4, NOTE_C4,8,
  NOTE_C5,4, NOTE_A4,4, NOTE_F4,4,
  NOTE_E4,4, NOTE_D4,4,
  NOTE_AS4,4, NOTE_AS4,8,
  NOTE_A4,4, NOTE_F4,4, NOTE_G4,4,
  NOTE_F4,2,
  REST,4
};

// ============================================
// CHRISTMAS CAROL MELODY (from your code)
// ============================================
const int christmasCarolMelody[] = {
  NOTE_C5,4, //1
  NOTE_F5,4, NOTE_F5,8, NOTE_G5,8, NOTE_F5,8, NOTE_E5,8,
  NOTE_D5,4, NOTE_D5,4, NOTE_D5,4,
  NOTE_G5,4, NOTE_G5,8, NOTE_A5,8, NOTE_G5,8, NOTE_F5,8,
  NOTE_E5,4, NOTE_C5,4, NOTE_C5,4,
  NOTE_A5,4, NOTE_A5,8, NOTE_AS5,8, NOTE_A5,8, NOTE_G5,8,
  NOTE_F5,4, NOTE_D5,4, NOTE_C5,8, NOTE_C5,8,
  NOTE_D5,4, NOTE_G5,4, NOTE_E5,4,

  NOTE_F5,2, NOTE_C5,4, //8 
  NOTE_F5,4, NOTE_F5,8, NOTE_G5,8, NOTE_F5,8, NOTE_E5,8,
  NOTE_D5,4, NOTE_D5,4, NOTE_D5,4,
  NOTE_G5,4, NOTE_G5,8, NOTE_A5,8, NOTE_G5,8, NOTE_F5,8,
  NOTE_E5,4, NOTE_C5,4, NOTE_C5,4,
  NOTE_A5,4, NOTE_A5,8, NOTE_AS5,8, NOTE_A5,8, NOTE_G5,8,
  NOTE_F5,4, NOTE_D5,4, NOTE_C5,8, NOTE_C5,8,
  NOTE_D5,4, NOTE_G5,4, NOTE_E5,4,
  NOTE_F5,2, NOTE_C5,4,

  NOTE_F5,4, NOTE_F5,4, NOTE_F5,4,//17
  NOTE_E5,2, NOTE_E5,4,
  NOTE_F5,4, NOTE_E5,4, NOTE_D5,4,
  NOTE_C5,2, NOTE_A5,4,
  NOTE_AS5,4, NOTE_A5,4, NOTE_G5,4,
  NOTE_C6,4, NOTE_C5,4, NOTE_C5,8, NOTE_C5,8,
  NOTE_D5,4, NOTE_G5,4, NOTE_E5,4,
  NOTE_F5,2, NOTE_C5,4, 
  NOTE_F5,4, NOTE_F5,8, NOTE_G5,8, NOTE_F5,8, NOTE_E5,8,
  NOTE_D5,4, NOTE_D5,4, NOTE_D5,4,
  
  NOTE_G5,4, NOTE_G5,8, NOTE_A5,8, NOTE_G5,8, NOTE_F5,8, //27
  NOTE_E5,4, NOTE_C5,4, NOTE_C5,4,
  NOTE_A5,4, NOTE_A5,8, NOTE_AS5,8, NOTE_A5,8, NOTE_G5,8,
  NOTE_F5,4, NOTE_D5,4, NOTE_C5,8, NOTE_C5,8,
  NOTE_D5,4, NOTE_G5,4, NOTE_E5,4,
  NOTE_F5,2, NOTE_C5,4,
  NOTE_F5,4, NOTE_F5,4, NOTE_F5,4,
  NOTE_E5,2, NOTE_E5,4,
  NOTE_F5,4, NOTE_E5,4, NOTE_D5,4,
  
  NOTE_C5,2, NOTE_A5,4,//36
  NOTE_AS5,4, NOTE_A5,4, NOTE_G5,4,
  NOTE_C6,4, NOTE_C5,4, NOTE_C5,8, NOTE_C5,8,
  NOTE_D5,4, NOTE_G5,4, NOTE_E5,4,
  NOTE_F5,2, NOTE_C5,4, 
  NOTE_F5,4, NOTE_F5,8, NOTE_G5,8, NOTE_F5,8, NOTE_E5,8,
  NOTE_D5,4, NOTE_D5,4, NOTE_D5,4,
  NOTE_G5,4, NOTE_G5,8, NOTE_A5,8, NOTE_G5,8, NOTE_F5,8, 
  NOTE_E5,4, NOTE_C5,4, NOTE_C5,4,
  
  NOTE_A5,4, NOTE_A5,8, NOTE_AS5,8, NOTE_A5,8, NOTE_G5,8,//45
  NOTE_F5,4, NOTE_D5,4, NOTE_C5,8, NOTE_C5,8,
  NOTE_D5,4, NOTE_G5,4, NOTE_E5,4,
  NOTE_F5,2, NOTE_C5,4,
  NOTE_F5,4, NOTE_F5,8, NOTE_G5,8, NOTE_F5,8, NOTE_E5,8,
  NOTE_D5,4, NOTE_D5,4, NOTE_D5,4,
  NOTE_G5,4, NOTE_G5,8, NOTE_A5,8, NOTE_G5,8, NOTE_F5,8,
  NOTE_E5,4, NOTE_C5,4, NOTE_C5,4,
  
  NOTE_A5,4, NOTE_A5,8, NOTE_AS5,8, NOTE_A5,8, NOTE_G5,8, //53
  NOTE_F5,4, NOTE_D5,4, NOTE_C5,8, NOTE_C5,8,
  NOTE_D5,4, NOTE_G5,4, NOTE_E5,4,
  NOTE_F5,2, REST,4
};

// ============================================
// HAPPY NEW YEAR MELODY
// ============================================
const int happyNewYearMelody[] = {
  NOTE_C5,4, NOTE_C5,4,
  NOTE_C5,4, NOTE_F5,4, NOTE_F5,4, NOTE_E5,4,
  NOTE_D5,4, NOTE_D5,4, NOTE_C5,4,
  
  NOTE_A5,4, NOTE_A5,4, NOTE_G5,4,
  NOTE_F5,4, NOTE_F5,4, NOTE_E5,4,
  NOTE_D5,4, NOTE_D5,4, NOTE_C5,4,
  
  NOTE_A5,4, NOTE_A5,4, NOTE_G5,4,
  NOTE_F5,4, NOTE_F5,4, NOTE_E5,4,
  NOTE_D5,4, NOTE_D5,4, NOTE_C5,4,
  
  NOTE_C5,4, NOTE_C5,4,
  NOTE_C5,4, NOTE_F5,4, NOTE_F5,4, NOTE_E5,4,
  NOTE_D5,4, NOTE_D5,4, NOTE_C5,4,
  REST,4
};

// ============================================
// JANA GANA MANA MELODY
// ============================================
const int janaGanaManaMelody[] = {
  // First line: "Jana Gana Mana"
  NOTE_C5,4, NOTE_D5,4, NOTE_E5,4, NOTE_E5,4, NOTE_E5,4, NOTE_E5,4, 
  NOTE_E5,4, NOTE_E5,4, NOTE_E5,2, NOTE_D5,4, NOTE_E5,4, NOTE_F5,4,
  
  NOTE_E5,2, NOTE_E5,4, NOTE_E5,4, NOTE_D5,2, NOTE_D5,4, NOTE_D5,4,
  NOTE_B4,4, NOTE_D5,4, NOTE_C5,4,
  
  NOTE_C5,2, NOTE_C5,2, NOTE_G5,4, NOTE_G5,4, NOTE_G5,4, NOTE_G5,4,
  NOTE_G5,4, NOTE_G5,4, NOTE_G5,2, NOTE_A5,4, NOTE_F5,4,
  
  NOTE_F5,2, NOTE_F5,4, NOTE_F5,4, NOTE_F5,4, NOTE_E5,4, NOTE_D5,4,
  NOTE_D5,2, NOTE_F5,4,
  
  NOTE_E5,4, NOTE_E5,4, NOTE_D5,4, NOTE_E5,4, NOTE_E5,4, NOTE_D5,4,
  NOTE_E5,2, NOTE_G5,4, NOTE_G5,4, NOTE_A5,4, NOTE_F5,2, NOTE_F5,4,
  
  NOTE_E5,2, NOTE_E5,4, NOTE_E5,4, NOTE_E5,4, NOTE_E5,4, NOTE_D5,4,
  NOTE_D5,2, NOTE_B4,4, NOTE_D5,4, NOTE_C5,4,
  
  NOTE_C5,4, NOTE_D5,4, NOTE_E5,4, NOTE_E5,4, NOTE_E5,4, NOTE_E5,4,
  NOTE_D5,4, NOTE_F5,4,
  
  NOTE_E5,4, NOTE_F5,4, NOTE_G5,4, NOTE_G5,4, NOTE_G5,4, NOTE_F5,4,
  NOTE_E5,4, NOTE_D5,4, NOTE_F5,4, NOTE_E5,4,
  
  NOTE_F5,4, NOTE_E5,4, NOTE_E5,4, NOTE_E5,4, NOTE_D5,4, NOTE_D5,2,
  NOTE_B4,4, NOTE_D5,4, NOTE_C5,4,
  
  NOTE_C5,2, NOTE_C5,2, NOTE_G5,4, NOTE_G5,4, NOTE_G5,4, NOTE_G5,4,
  NOTE_E5,4, NOTE_G5,4, NOTE_G5,4, NOTE_G5,4, NOTE_G5,4, NOTE_A5,4,
  
  NOTE_F5,2, NOTE_F5,4, NOTE_F5,4, NOTE_F5,4, NOTE_E5,4, NOTE_D5,4,
  NOTE_F5,4, NOTE_E5,4,
  
  NOTE_E5,4, NOTE_G5,4, NOTE_C5,4, NOTE_B4,4, NOTE_A4,4, NOTE_B4,4,
  NOTE_A4,4, NOTE_G5,4, NOTE_A5,4,
  
  NOTE_C5,4, NOTE_C5,4, NOTE_D5,4, NOTE_D5,4, NOTE_E5,4, NOTE_E5,4,
  NOTE_D5,4, NOTE_E5,4, NOTE_F5,2,
  
  REST,4
};

// ============================================
// HAPPY ANNIVERSARY MELODY
// ============================================
const int happyAnniversaryMelody[] = {
  NOTE_C5,4, NOTE_E5,4, NOTE_G5,4, NOTE_C6,2,
  NOTE_G5,4, NOTE_E5,4, NOTE_C5,2,
  
  NOTE_F5,4, NOTE_A5,4, NOTE_C6,4, NOTE_F6,2,
  NOTE_C6,4, NOTE_A5,4, NOTE_F5,2,
  
  NOTE_G5,4, NOTE_B5,4, NOTE_D6,4, NOTE_G6,2,
  NOTE_D6,4, NOTE_B5,4, NOTE_G5,2,
  
  NOTE_C6,4, NOTE_E6,4, NOTE_G6,4, NOTE_C7,2,
  NOTE_G6,4, NOTE_E6,4, NOTE_C6,2,
  
  NOTE_C5,8, NOTE_E5,8, NOTE_G5,4,
  NOTE_C5,8, NOTE_E5,8, NOTE_G5,4,
  NOTE_A5,4, NOTE_G5,4, NOTE_F5,4, NOTE_E5,4,
  NOTE_D5,2, NOTE_G5,2,
  
  NOTE_C5,8, NOTE_E5,8, NOTE_G5,4,
  NOTE_C5,8, NOTE_E5,8, NOTE_G5,4,
  NOTE_A5,4, NOTE_G5,4, NOTE_F5,4, NOTE_E5,4,
  NOTE_D5,2, NOTE_C5,2,
  
  REST,4
};

// ============================================
// ORIGINAL JANA GANA MANA PLAYER FUNCTIONS
// ============================================

// Original frequency mapping from your code
int getOriginalPeriod(char note) {
  switch(note) {
    case 'c': return 1915;
    case 'd': return 1700;
    case 'e': return 1519;
    case 'f': return 1432;
    case 'g': return 1275;
    case 'a': return 1136;
    case 'b': return 1014;
    case 'C': return 956;
    default: return 0;
  }
}

void playOriginalTone(int pin, int period, int duration) {
  for (long i = 0; i < duration * 1000L; i += period * 2) {
    digitalWrite(pin, HIGH);
    delayMicroseconds(period);
    digitalWrite(pin, LOW);
    delayMicroseconds(period);
  }
}

// Play ORIGINAL Jana Gana Mana (mono)
void playJanaGanaManaOriginal() {
  char notes[] = "cdeeeeeeeeedef eeedddbdc ccgggggggggaf fffffeddf eedeedeggaff eeeeeddbdc cdeeeedf efgggfedfe feeeddbdc ccggggeggggga fffffedfe egc bab aga ccddeedef ";
  int beats[] = { 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1,
                  2, 1, 1, 2, 1, 1, 1, 1, 1, 1,
                  2, 2, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1,
                  2, 1, 1, 2, 1, 1, 1, 1, 1, 1,
                  2, 1, 1, 2, 1, 1, 1, 1, 2, 1, 1, 1, 1,
                  2, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1,
                  1, 1, 1, 1, 2, 2, 2, 1, 1,
                  1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1,
                  2, 2, 1, 1, 1, 1, 1, 1, 1, 1,
                  1, 1, 1, 1, 2, 1, 1, 2, 1, 1, 1, 1, 1, 1,
                  2, 1, 1, 2, 1, 1, 1, 1, 1, 1,
                  1, 1, 4, 1, 1, 1, 4, 1, 1, 1, 4, 1,
                  1, 1, 1, 1, 1, 1, 1, 1, 4, 1 };
  int length = 149;
  int tempo = 250;
  
  pinMode(BUZZER_LEFT, OUTPUT);
  
  for (int i = 0; i < length; i++) {
    if (notes[i] == ' ') {
      delay(beats[i] * tempo);
    } else {
      int period = getOriginalPeriod(notes[i]);
      if (period > 0) {
        playOriginalTone(BUZZER_LEFT, period, beats[i] * tempo);
      }
    }
    delay(tempo / 2);
  }
}

// Play ORIGINAL Jana Gana Mana (stereo)
void playJanaGanaManaOriginalStereo() {
  char notes[] = "cdeeeeeeeeedef eeedddbdc ccgggggggggaf fffffeddf eedeedeggaff eeeeeddbdc cdeeeedf efgggfedfe feeeddbdc ccggggeggggga fffffedfe egc bab aga ccddeedef ";
  int beats[] = { 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1,
                  2, 1, 1, 2, 1, 1, 1, 1, 1, 1,
                  2, 2, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1,
                  2, 1, 1, 2, 1, 1, 1, 1, 1, 1,
                  2, 1, 1, 2, 1, 1, 1, 1, 2, 1, 1, 1, 1,
                  2, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1,
                  1, 1, 1, 1, 2, 2, 2, 1, 1,
                  1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1,
                  2, 2, 1, 1, 1, 1, 1, 1, 1, 1,
                  1, 1, 1, 1, 2, 1, 1, 2, 1, 1, 1, 1, 1, 1,
                  2, 1, 1, 2, 1, 1, 1, 1, 1, 1,
                  1, 1, 4, 1, 1, 1, 4, 1, 1, 1, 4, 1,
                  1, 1, 1, 1, 1, 1, 1, 1, 4, 1 };
  int length = 149;
  int tempo = 250;
  
  pinMode(BUZZER_LEFT, OUTPUT);
  pinMode(BUZZER_RIGHT, OUTPUT);
  
  for (int i = 0; i < length; i++) {
    if (notes[i] == ' ') {
      delay(beats[i] * tempo);
    } else {
      int period = getOriginalPeriod(notes[i]);
      if (period > 0) {
        // Alternate between left and right for stereo
        int pin = (i % 2 == 0) ? BUZZER_LEFT : BUZZER_RIGHT;
        playOriginalTone(pin, period, beats[i] * tempo);
      }
    }
    delay(tempo / 2);
  }
}

// ============================================
// STANDARD FREQUENCY SONGS
// ============================================

// Helper function for standard songs
void playStandardMelody(const int melody[], int arraySize, int tempo) {
  int wholenote = (60000 * 4) / tempo;
  
  pinMode(BUZZER_LEFT, OUTPUT);
  pinMode(BUZZER_RIGHT, OUTPUT);
  
  for (int i = 0; i < arraySize; i += 2) {
    int notePitch = melody[i];
    int noteType = melody[i + 1];
    
    int noteDuration;
    if (noteType > 0) {
      noteDuration = wholenote / noteType;
    } else {
      noteDuration = wholenote / abs(noteType);
      noteDuration *= 1.5;
    }
    
    // Stereo effect: alternate between buzzers
    int pin = ((i/2) % 2 == 0) ? BUZZER_LEFT : BUZZER_RIGHT;
    
    if (notePitch != 0) {
      tone(pin, notePitch, noteDuration * 0.9);
    }
    
    delay(noteDuration);
    noTone(pin);
    delay(noteDuration * 0.1);
  }
}

// Include melody arrays (make sure these are defined elsewhere)
extern const int happyBirthdayMelody[];
extern const int happyNewYearMelody[];
extern const int christmasCarolMelody[];
extern const int janaGanaManaMelody[];
extern const int happyAnniversaryMelody[];

void playHappyBirthday() {
  playStandardMelody(happyBirthdayMelody, sizeof(happyBirthdayMelody)/sizeof(int), 120);
}

void playHappyNewYear() {
  playStandardMelody(happyNewYearMelody, sizeof(happyNewYearMelody)/sizeof(int), 140);
}

void playChristmasCarol() {
  playStandardMelody(christmasCarolMelody, sizeof(christmasCarolMelody)/sizeof(int), 140);
}

void playJanaGanaMana() {
  playStandardMelody(janaGanaManaMelody, sizeof(janaGanaManaMelody)/sizeof(int), 100);
}

void playHappyAnniversary() {
  playStandardMelody(happyAnniversaryMelody, sizeof(happyAnniversaryMelody)/sizeof(int), 120);
}