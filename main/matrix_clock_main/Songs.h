#ifndef SONGS_H
#define SONGS_H

#include <Arduino.h>
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
// #include "Fonts.h"  // For SmallDigits, SmallerDigits

// Buzzer pins
#define BUZZER_LEFT 05  // D2
#define BUZZER_RIGHT 04 // D1

extern MD_Parola Display;

// Melody arrays
extern const int happyBirthdayMelody[];
extern const int happyNewYearMelody[];
extern const int christmasCarolMelody[];
extern const int janaGanaManaMelody[];
extern const int happyAnniversaryMelody[];

// Song state tracking
extern bool isSongPlaying;
extern unsigned long songStartTime;
extern const char* currentSongMessage;
extern const unsigned long SONG_DURATION;

// Function declarations
void playHappyBirthday();
void playHappyNewYear();
void playChristmasCarol();
void playJanaGanaMana();
void playHappyAnniversary();
void playJanaGanaManaOriginal();
void playJanaGanaManaOriginalStereo();

// Song playback helpers
void startSongWithMessage(const char* message, void (*songFunction)());
void updateSongDisplay();
void playSongNote(int pin, int notePitch, int noteDuration);

// Non-blocking melody playback functions
void startMelodyPlayback(const int melody[], int arraySize, int songTempo);

bool updateMelodyPlayback();
void stopMelodyPlayback();
bool isMelodyPlaying();

// Original functions
void playStandardMelody(const int melody[], int arraySize, int tempo);
int getOriginalPeriod(char note);
void playOriginalTone(int pin, int period, int duration);

#endif