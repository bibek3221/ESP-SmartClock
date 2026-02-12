#include "Songs.h"
#include "Notes.h"
#include <Arduino.h>  // ADD THIS LINE
#include "display_utils.h"

// Buzzer pins (must match main program)
#define BUZZER_LEFT 05   // D2
#define BUZZER_RIGHT 04  // D1

bool isSongPlaying = false;
unsigned long songStartTime = 0;
const char* currentSongMessage = "";
const unsigned long SONG_DURATION = 30000;

// ============================================
// HAPPY BIRTHDAY MELODY
// ============================================
const int happyBirthdayMelody[] = {
  NOTE_C4, 4, NOTE_C4, 8,
  NOTE_D4, 4, NOTE_C4, 4, NOTE_F4, 4,
  NOTE_E4, 2,
  NOTE_C4, 4, NOTE_C4, 8,
  NOTE_D4, 4, NOTE_C4, 4, NOTE_G4, 4,
  NOTE_F4, 2,
  NOTE_C4, 4, NOTE_C4, 8,
  NOTE_C5, 4, NOTE_A4, 4, NOTE_F4, 4,
  NOTE_E4, 4, NOTE_D4, 4,
  NOTE_AS4, 4, NOTE_AS4, 8,
  NOTE_A4, 4, NOTE_F4, 4, NOTE_G4, 4,
  NOTE_F4, 2,
  REST, 4
};

// ============================================
// CHRISTMAS CAROL MELODY (from your code)
// ============================================
const int christmasCarolMelody[] = {
  NOTE_C5, 4,  //1
  NOTE_F5, 4, NOTE_F5, 8, NOTE_G5, 8, NOTE_F5, 8, NOTE_E5, 8,
  NOTE_D5, 4, NOTE_D5, 4, NOTE_D5, 4,
  NOTE_G5, 4, NOTE_G5, 8, NOTE_A5, 8, NOTE_G5, 8, NOTE_F5, 8,
  NOTE_E5, 4, NOTE_C5, 4, NOTE_C5, 4,
  NOTE_A5, 4, NOTE_A5, 8, NOTE_AS5, 8, NOTE_A5, 8, NOTE_G5, 8,
  NOTE_F5, 4, NOTE_D5, 4, NOTE_C5, 8, NOTE_C5, 8,
  NOTE_D5, 4, NOTE_G5, 4, NOTE_E5, 4,

  NOTE_F5, 2, NOTE_C5, 4,  //8
  NOTE_F5, 4, NOTE_F5, 8, NOTE_G5, 8, NOTE_F5, 8, NOTE_E5, 8,
  NOTE_D5, 4, NOTE_D5, 4, NOTE_D5, 4,
  NOTE_G5, 4, NOTE_G5, 8, NOTE_A5, 8, NOTE_G5, 8, NOTE_F5, 8,
  NOTE_E5, 4, NOTE_C5, 4, NOTE_C5, 4,
  NOTE_A5, 4, NOTE_A5, 8, NOTE_AS5, 8, NOTE_A5, 8, NOTE_G5, 8,
  NOTE_F5, 4, NOTE_D5, 4, NOTE_C5, 8, NOTE_C5, 8,
  NOTE_D5, 4, NOTE_G5, 4, NOTE_E5, 4,
  NOTE_F5, 2, NOTE_C5, 4,

  NOTE_F5, 4, NOTE_F5, 4, NOTE_F5, 4,  //17
  NOTE_E5, 2, NOTE_E5, 4,
  NOTE_F5, 4, NOTE_E5, 4, NOTE_D5, 4,
  NOTE_C5, 2, NOTE_A5, 4,
  NOTE_AS5, 4, NOTE_A5, 4, NOTE_G5, 4,
  NOTE_C6, 4, NOTE_C5, 4, NOTE_C5, 8, NOTE_C5, 8,
  NOTE_D5, 4, NOTE_G5, 4, NOTE_E5, 4,
  NOTE_F5, 2, NOTE_C5, 4,
  NOTE_F5, 4, NOTE_F5, 8, NOTE_G5, 8, NOTE_F5, 8, NOTE_E5, 8,
  NOTE_D5, 4, NOTE_D5, 4, NOTE_D5, 4,

  NOTE_G5, 4, NOTE_G5, 8, NOTE_A5, 8, NOTE_G5, 8, NOTE_F5, 8,  //27
  NOTE_E5, 4, NOTE_C5, 4, NOTE_C5, 4,
  NOTE_A5, 4, NOTE_A5, 8, NOTE_AS5, 8, NOTE_A5, 8, NOTE_G5, 8,
  NOTE_F5, 4, NOTE_D5, 4, NOTE_C5, 8, NOTE_C5, 8,
  NOTE_D5, 4, NOTE_G5, 4, NOTE_E5, 4,
  NOTE_F5, 2, NOTE_C5, 4,
  NOTE_F5, 4, NOTE_F5, 4, NOTE_F5, 4,
  NOTE_E5, 2, NOTE_E5, 4,
  NOTE_F5, 4, NOTE_E5, 4, NOTE_D5, 4,

  NOTE_C5, 2, NOTE_A5, 4,  //36
  NOTE_AS5, 4, NOTE_A5, 4, NOTE_G5, 4,
  NOTE_C6, 4, NOTE_C5, 4, NOTE_C5, 8, NOTE_C5, 8,
  NOTE_D5, 4, NOTE_G5, 4, NOTE_E5, 4,
  NOTE_F5, 2, NOTE_C5, 4,
  NOTE_F5, 4, NOTE_F5, 8, NOTE_G5, 8, NOTE_F5, 8, NOTE_E5, 8,
  NOTE_D5, 4, NOTE_D5, 4, NOTE_D5, 4,
  NOTE_G5, 4, NOTE_G5, 8, NOTE_A5, 8, NOTE_G5, 8, NOTE_F5, 8,
  NOTE_E5, 4, NOTE_C5, 4, NOTE_C5, 4,

  NOTE_A5, 4, NOTE_A5, 8, NOTE_AS5, 8, NOTE_A5, 8, NOTE_G5, 8,  //45
  NOTE_F5, 4, NOTE_D5, 4, NOTE_C5, 8, NOTE_C5, 8,
  NOTE_D5, 4, NOTE_G5, 4, NOTE_E5, 4,
  NOTE_F5, 2, NOTE_C5, 4,
  NOTE_F5, 4, NOTE_F5, 8, NOTE_G5, 8, NOTE_F5, 8, NOTE_E5, 8,
  NOTE_D5, 4, NOTE_D5, 4, NOTE_D5, 4,
  NOTE_G5, 4, NOTE_G5, 8, NOTE_A5, 8, NOTE_G5, 8, NOTE_F5, 8,
  NOTE_E5, 4, NOTE_C5, 4, NOTE_C5, 4,

  NOTE_A5, 4, NOTE_A5, 8, NOTE_AS5, 8, NOTE_A5, 8, NOTE_G5, 8,  //53
  NOTE_F5, 4, NOTE_D5, 4, NOTE_C5, 8, NOTE_C5, 8,
  NOTE_D5, 4, NOTE_G5, 4, NOTE_E5, 4,
  NOTE_F5, 2, REST, 4
};

// ============================================
// HAPPY NEW YEAR MELODY
// ============================================
const int happyNewYearMelody[] = {
  NOTE_C5, 4, NOTE_C5, 4,
  NOTE_C5, 4, NOTE_F5, 4, NOTE_F5, 4, NOTE_E5, 4,
  NOTE_D5, 4, NOTE_D5, 4, NOTE_C5, 4,

  NOTE_A5, 4, NOTE_A5, 4, NOTE_G5, 4,
  NOTE_F5, 4, NOTE_F5, 4, NOTE_E5, 4,
  NOTE_D5, 4, NOTE_D5, 4, NOTE_C5, 4,

  NOTE_A5, 4, NOTE_A5, 4, NOTE_G5, 4,
  NOTE_F5, 4, NOTE_F5, 4, NOTE_E5, 4,
  NOTE_D5, 4, NOTE_D5, 4, NOTE_C5, 4,

  NOTE_C5, 4, NOTE_C5, 4,
  NOTE_C5, 4, NOTE_F5, 4, NOTE_F5, 4, NOTE_E5, 4,
  NOTE_D5, 4, NOTE_D5, 4, NOTE_C5, 4,
  REST, 4
};

// ============================================
// JANA GANA MANA MELODY
// ============================================
const int janaGanaManaMelody[] = {
  // First line: "Jana Gana Mana"
  NOTE_C5, 4, NOTE_D5, 4, NOTE_E5, 4, NOTE_E5, 4, NOTE_E5, 4, NOTE_E5, 4,
  NOTE_E5, 4, NOTE_E5, 4, NOTE_E5, 2, NOTE_D5, 4, NOTE_E5, 4, NOTE_F5, 4,

  NOTE_E5, 2, NOTE_E5, 4, NOTE_E5, 4, NOTE_D5, 2, NOTE_D5, 4, NOTE_D5, 4,
  NOTE_B4, 4, NOTE_D5, 4, NOTE_C5, 4,

  NOTE_C5, 2, NOTE_C5, 2, NOTE_G5, 4, NOTE_G5, 4, NOTE_G5, 4, NOTE_G5, 4,
  NOTE_G5, 4, NOTE_G5, 4, NOTE_G5, 2, NOTE_A5, 4, NOTE_F5, 4,

  NOTE_F5, 2, NOTE_F5, 4, NOTE_F5, 4, NOTE_F5, 4, NOTE_E5, 4, NOTE_D5, 4,
  NOTE_D5, 2, NOTE_F5, 4,

  NOTE_E5, 4, NOTE_E5, 4, NOTE_D5, 4, NOTE_E5, 4, NOTE_E5, 4, NOTE_D5, 4,
  NOTE_E5, 2, NOTE_G5, 4, NOTE_G5, 4, NOTE_A5, 4, NOTE_F5, 2, NOTE_F5, 4,

  NOTE_E5, 2, NOTE_E5, 4, NOTE_E5, 4, NOTE_E5, 4, NOTE_E5, 4, NOTE_D5, 4,
  NOTE_D5, 2, NOTE_B4, 4, NOTE_D5, 4, NOTE_C5, 4,

  NOTE_C5, 4, NOTE_D5, 4, NOTE_E5, 4, NOTE_E5, 4, NOTE_E5, 4, NOTE_E5, 4,
  NOTE_D5, 4, NOTE_F5, 4,

  NOTE_E5, 4, NOTE_F5, 4, NOTE_G5, 4, NOTE_G5, 4, NOTE_G5, 4, NOTE_F5, 4,
  NOTE_E5, 4, NOTE_D5, 4, NOTE_F5, 4, NOTE_E5, 4,

  NOTE_F5, 4, NOTE_E5, 4, NOTE_E5, 4, NOTE_E5, 4, NOTE_D5, 4, NOTE_D5, 2,
  NOTE_B4, 4, NOTE_D5, 4, NOTE_C5, 4,

  NOTE_C5, 2, NOTE_C5, 2, NOTE_G5, 4, NOTE_G5, 4, NOTE_G5, 4, NOTE_G5, 4,
  NOTE_E5, 4, NOTE_G5, 4, NOTE_G5, 4, NOTE_G5, 4, NOTE_G5, 4, NOTE_A5, 4,

  NOTE_F5, 2, NOTE_F5, 4, NOTE_F5, 4, NOTE_F5, 4, NOTE_E5, 4, NOTE_D5, 4,
  NOTE_F5, 4, NOTE_E5, 4,

  NOTE_E5, 4, NOTE_G5, 4, NOTE_C5, 4, NOTE_B4, 4, NOTE_A4, 4, NOTE_B4, 4,
  NOTE_A4, 4, NOTE_G5, 4, NOTE_A5, 4,

  NOTE_C5, 4, NOTE_C5, 4, NOTE_D5, 4, NOTE_D5, 4, NOTE_E5, 4, NOTE_E5, 4,
  NOTE_D5, 4, NOTE_E5, 4, NOTE_F5, 2,

  REST, 4
};

// ============================================
// HAPPY ANNIVERSARY MELODY
// ============================================
const int happyAnniversaryMelody[] = {
  NOTE_C5, 4, NOTE_E5, 4, NOTE_G5, 4, NOTE_C6, 2,
  NOTE_G5, 4, NOTE_E5, 4, NOTE_C5, 2,

  NOTE_F5, 4, NOTE_A5, 4, NOTE_C6, 4, NOTE_F6, 2,
  NOTE_C6, 4, NOTE_A5, 4, NOTE_F5, 2,

  NOTE_G5, 4, NOTE_B5, 4, NOTE_D6, 4, NOTE_G6, 2,
  NOTE_D6, 4, NOTE_B5, 4, NOTE_G5, 2,

  NOTE_C6, 4, NOTE_E6, 4, NOTE_G6, 4, NOTE_C7, 2,
  NOTE_G6, 4, NOTE_E6, 4, NOTE_C6, 2,

  NOTE_C5, 8, NOTE_E5, 8, NOTE_G5, 4,
  NOTE_C5, 8, NOTE_E5, 8, NOTE_G5, 4,
  NOTE_A5, 4, NOTE_G5, 4, NOTE_F5, 4, NOTE_E5, 4,
  NOTE_D5, 2, NOTE_G5, 2,

  NOTE_C5, 8, NOTE_E5, 8, NOTE_G5, 4,
  NOTE_C5, 8, NOTE_E5, 8, NOTE_G5, 4,
  NOTE_A5, 4, NOTE_G5, 4, NOTE_F5, 4, NOTE_E5, 4,
  NOTE_D5, 2, NOTE_C5, 2,

  REST, 4
};

// ============================================
// ORIGINAL JANA GANA MANA PLAYER FUNCTIONS
// ============================================

// Original frequency mapping from your code
int getOriginalPeriod(char note) {
  switch (note) {
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

/// ============================================
// NON-BLOCKING SONG PLAYBACK SYSTEM
// ============================================

// Global variables for song playback state
static const int* currentMelody = nullptr;
static int melodySize = 0;
static int currentNoteIndex = 0;
static int tempo = 120;
static unsigned long noteStartTime = 0;
static int currentNoteDuration = 0;
static bool notePlaying = false;
static bool melodyActive = false;

// Start a song with message
void startSongWithMessage(const char* message, void (*songFunction)()) {
  Serial.print("Starting song: ");
  Serial.println(message);

  isSongPlaying = true;
  songStartTime = millis();
  currentSongMessage = message;

  // Configure display for song message
  Display.setZone(0, 0, 3);  // Use all modules for song display
  Display.setZone(1, 0, 0);  // Disable second zone
  Display.setFont(0, NULL);  // Use default font
  Display.displayClear(0);
  Display.displayClear(1);

  // Play the song (non-blocking)
  songFunction();
}


// Start melody playback (non-blocking)
void startMelodyPlayback(const int melody[], int arraySize, int songTempo) {
  currentMelody = melody;
  melodySize = arraySize;
  currentNoteIndex = 0;
  tempo = songTempo;
  notePlaying = false;
  noteStartTime = 0;
  melodyActive = true;

  Serial.print("Starting melody playback, size: ");
  Serial.print(melodySize);
  Serial.print(", tempo: ");
  Serial.println(tempo);
}

// Update melody playback (call this from main loop)
bool updateMelodyPlayback() {
  if (!melodyActive || !currentMelody || currentNoteIndex >= melodySize) {
    return true;  // Done or not active
  }

  unsigned long currentTime = millis();
  int wholenote = (60000 * 4) / tempo;

  if (!notePlaying) {
    // Start a new note
    int notePitch = currentMelody[currentNoteIndex];
    int noteType = currentMelody[currentNoteIndex + 1];

    if (noteType > 0) {
      currentNoteDuration = wholenote / noteType;
    } else {
      currentNoteDuration = wholenote / abs(noteType);
      currentNoteDuration *= 1.5;
    }

    int pin = ((currentNoteIndex / 2) % 2 == 0) ? BUZZER_LEFT : BUZZER_RIGHT;
    pinMode(pin, OUTPUT);

    if (notePitch != 0) {
      tone(pin, notePitch, currentNoteDuration * 0.9);
    }

    noteStartTime = currentTime;
    notePlaying = true;
    currentNoteIndex += 2;

  } else if (currentTime - noteStartTime >= currentNoteDuration) {
    // Note finished
    int pin = (((currentNoteIndex - 2) / 2) % 2 == 0) ? BUZZER_LEFT : BUZZER_RIGHT;
    noTone(pin);
    notePlaying = false;

    // Check if we're done with the melody
    if (currentNoteIndex >= melodySize) {
      melodyActive = false;
      Serial.println("Melody playback completed");
      return true;
    }
  }

  return false;  // Still playing
}

// Stop melody playback
// Stop melody playback
void stopMelodyPlayback() {
  noTone(BUZZER_LEFT);
  noTone(BUZZER_RIGHT);
  currentMelody = nullptr;
  currentNoteIndex = 0;
  notePlaying = false;
  melodyActive = false;
  isSongPlaying = false;

  // DON'T try to restore display from here
  // Just clear this flag - main loop will handle display

  Serial.println("Song stopped");
}

// Check if melody is playing
bool isMelodyPlaying() {
  return melodyActive;
}

// ============================================
// STANDARD SONG FUNCTIONS (NON-BLOCKING)
// ============================================

void playHappyBirthday() {
  startMelodyPlayback(happyBirthdayMelody,
                      sizeof(happyBirthdayMelody) / sizeof(int),
                      120);
}

void playHappyNewYear() {
  startMelodyPlayback(happyNewYearMelody,
                      sizeof(happyNewYearMelody) / sizeof(int),
                      140);
}

void playChristmasCarol() {
  startMelodyPlayback(christmasCarolMelody,
                      sizeof(christmasCarolMelody) / sizeof(int),
                      140);
}

void playJanaGanaMana() {
  startMelodyPlayback(janaGanaManaMelody,
                      sizeof(janaGanaManaMelody) / sizeof(int),
                      100);
}

void playHappyAnniversary() {
  startMelodyPlayback(happyAnniversaryMelody,
                      sizeof(happyAnniversaryMelody) / sizeof(int),
                      120);
}

// ============================================
// SIMPLER PLAYBACK FUNCTIONS (Alternative)
// ============================================

// Simple blocking playback (for testing)
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

    int pin = ((i / 2) % 2 == 0) ? BUZZER_LEFT : BUZZER_RIGHT;

    if (notePitch != 0) {
      tone(pin, notePitch, noteDuration * 0.9);
    }

    delay(noteDuration);
    noTone(pin);
    delay(noteDuration * 0.1);
  }
}