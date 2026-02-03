# ESP8266 Dual Buzzer Music Player 🎵

A modular music player system for ESP8266 that plays multiple songs on dual buzzers with stereo effect. Just call a function to play any song!

## 🎯 Features

- **Modular Design**: Clean separation between player engine and songs
- **Dual Buzzer Stereo**: Notes alternate between left and right buzzers for spatial effect
- **One-Line Playback**: Simply call `playSongName()` to play any song
- **Multiple Song Formats**: Supports both standard frequency notation and original letter notation
- **5 Pre-loaded Songs**: Ready-to-play popular melodies

## 📦 Included Songs

1. **🎂 Happy Birthday** - Standard version
2. **🎄 Christmas Carol** (We Wish You a Merry Christmas)
3. **🎆 Happy New Year** 
4. **🇮🇳 Jana Gana Mana** - Indian National Anthem (Original + Standard versions)
5. **💑 Happy Anniversary** - Romantic melody

## 🔧 Hardware Requirements

- ESP8266 (NodeMCU, Wemos D1 Mini, etc.)
- 2x Passive Buzzers (or speakers)
- Jumper wires

## 🔌 Connections

| ESP8266 Pin | Buzzer Connection | Purpose |
|------------|-------------------|---------|
| **D5 (GPIO14)** | Buzzer 1 Positive | Left channel |
| **D6 (GPIO12)** | Buzzer 2 Positive | Right channel |
| **GND** | Both Buzzer Negatives | Ground |

**Note**: For better sound quality, add a 100Ω resistor in series with each buzzer.

## 📁 File Structure

```
ESP_Music_Player/
├── ESP_Music_Player.ino      # Main program
├── Songs.h                   # Song function declarations
├── Songs.cpp                 # Song implementations + melody arrays
└── Notes.h                   # Musical note definitions
```

## 🚀 Quick Start

1. **Upload the code** to your ESP8266
2. **Open Serial Monitor** (115200 baud)
3. **Listen** to the demo sequence
4. **Call song functions** from your own code

## 💻 Usage Examples

### Basic Playback
```cpp
// In setup() or loop():
playHappyBirthday();           // Plays Happy Birthday
playChristmasCarol();          // Plays Christmas Carol
playJanaGanaManaOriginal();    // Plays ORIGINAL Jana Gana Mana
```

### Conditional Playback
```cpp
void loop() {
  // Play when button is pressed
  if (digitalRead(BUTTON_PIN) == LOW) {
    playHappyBirthday();
  }
  
  // Play at specific time
  if (hour() == 12 && minute() == 0) {
    playHappyNewYear();
  }
  
  // Play based on sensor reading
  if (temperature > 30) {
    playChristmasCarol();  // Cool down with Christmas tunes!
  }
}
```

### Auto-play Sequence
```cpp
void loop() {
  static unsigned long lastPlay = 0;
  if (millis() - lastPlay > 60000) {  // Every minute
    playJanaGanaManaOriginalStereo();
    lastPlay = millis();
  }
}
```

## 🎵 Song Functions Reference

### Standard Songs (Stereo)
```cpp
playHappyBirthday();          // 🎂 Happy Birthday
playChristmasCarol();         // 🎄 We Wish You a Merry Christmas  
playHappyNewYear();           // 🎆 Happy New Year
playJanaGanaMana();           // 🇮🇳 Jana Gana Mana (Standard)
playHappyAnniversary();       // 💑 Happy Anniversary
```

### Original Jana Gana Mana Versions
```cpp
playJanaGanaManaOriginal();   // Mono - EXACT original sound
playJanaGanaManaOriginalStereo();  // Stereo - Original with spatial effect
```

## 🎛️ Customization

### Change Tempo
Modify the tempo parameter in `Songs.cpp`:
```cpp
// In playStandardMelody() function calls:
playStandardMelody(melody, size, 120);  // 120 BPM (default)
playStandardMelody(melody, size, 80);   // Slower
playStandardMelody(melody, size, 180);  // Faster
```

### Add New Songs
1. Add melody array to `Songs.cpp`:
```cpp
const int myNewSong[] = {
  NOTE_C4,4, NOTE_D4,4, NOTE_E4,4, // Notes and durations
  // ... more notes
  REST,4
};
```

2. Add function declaration to `Songs.h`:
```cpp
void playMyNewSong();
```

3. Add function implementation to `Songs.cpp`:
```cpp
void playMyNewSong() {
  playStandardMelody(myNewSong, sizeof(myNewSong)/sizeof(int), 120);
}
```

## 🔍 Technical Details

### Stereo Effect
The system creates a "ping-pong" stereo effect by:
- Even-numbered notes → Left buzzer (D5)
- Odd-numbered notes → Right buzzer (D6)
- Creates spatial movement illusion

### Two Jana Gana Mana Versions
1. **Original Version**: Uses exact frequencies (1915, 1700, 1519, etc.) from the YouTube code
2. **Standard Version**: Uses proper musical notes (NOTE_C4, NOTE_D4, etc.)

The original version sounds exactly like the YouTube tutorial, while the standard version uses correct musical tuning.

### Memory Usage
- Songs are stored in program memory (PROGMEM compatible)
- No dynamic memory allocation
- Efficient for ESP8266's limited RAM

## 🐛 Troubleshooting

| Problem | Solution |
|---------|----------|
| No sound | Check buzzer polarity (+ to D5/D6, - to GND) |
| Weak sound | Use active buzzers or add amplifier |
| One buzzer not working | Check wiring to correct GPIO pin |
| Compilation errors | Ensure all files are in same folder |
| Upload fails | Hold FLASH button during upload |

## 📊 Serial Output
```
🎵 ESP8266 Music Player 🎵
==========================

1. Playing Jana Gana Mana (ORIGINAL - Mono)...
2. Playing Jana Gana Mana (ORIGINAL - Stereo)...
3. Playing Happy Birthday...
4. Playing Christmas Carol...
5. Playing Happy New Year...
6. Playing Jana Gana Mana (Standard)...
7. Playing Happy Anniversary...

✅ All songs played!
```

## 🎨 Extending the System

### Add Button Controls
```cpp
const int NEXT_BTN = 5;  // D1
const int PLAY_BTN = 4;  // D2

void loop() {
  if (digitalRead(NEXT_BTN) == LOW) {
    playNextSong();
  }
  if (digitalRead(PLAY_BTN) == LOW) {
    playCurrentSong();
  }
}
```

### Create Playlists
```cpp
void playBirthdayParty() {
  playHappyBirthday();
  delay(1000);
  playHappyAnniversary();
  delay(1000);
  playJanaGanaManaStereo();
}

void playFestivalSet() {
  playChristmasCarol();
  delay(1000);
  playHappyNewYear();
  delay(1000);
  playJanaGanaManaOriginal();
}
```

### Add LED Effects
```cpp
void playWithLights() {
  digitalWrite(LED_PIN, HIGH);
  playSong();
  digitalWrite(LED_PIN, LOW);
}
```

## 🤝 Contributing

Want to add more songs? Submit a pull request with:
1. The melody array
2. Appropriate credits/source
3. Brief description

## 📄 License

MIT License - Feel free to use in personal and commercial projects.

## 🙏 Credits

- **YouTube|Tech at Home**: Original Jana Gana Mana implementation
- **Arduino Tone Library**: Base functionality
- **Musical Note Frequencies**: Standard A440 tuning

## 🎯 Project Status

✅ **Working Features:**
- All 5 songs play correctly
- Stereo effect functional
- Modular architecture
- Easy to extend

🔧 **Future Enhancements:**
- Volume control
- Song selection via buttons
- WiFi remote control
- SD card song storage

---

**Happy Coding!** 🎶 If you enjoy this project, give it a star! ⭐
