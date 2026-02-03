

---

# ⏰ ESP8266 Smart Clock

A Wi-Fi–enabled **smart digital clock** built using **ESP8266 (NodeMCU)** and **MAX7219 LED Matrix**, featuring real-time clock synchronization via NTP, buzzer alerts, touch/switch interaction, and expandable IoT features.

---

## 📌 Features

* 🌐 **Wi-Fi Time Sync (NTP)** – Accurate real-time clock via internet
* 🕒 **24-hour Digital Time Display**
* 🔢 **Seconds Display**
* 🔔 **Buzzer Support**

  * Alarm / sound alerts
  * Melody playback (Happy Birthday, Christmas songs, etc.)
* 🖐 **Touch Sensor / Switch Input**

  * Change display mode
  * Trigger sound or special functions
* 💡 **LED Matrix Display (MAX7219)**
* ⚡ Low-power and compact design
* 🛠 Easily expandable (weather, date, temperature, web control)

---

## 🧰 Hardware Components

| Component                  | Description                              |
| -------------------------- | ---------------------------------------- |
| ESP8266 NodeMCU            | Main microcontroller                     |
| MAX7219 LED Matrix         | Time & text display                      |
| Buzzer (Active/Passive)    | Alarm & sound output                     |
| Touch Sensor / Push Button | User input                               |
| Resistors                  | Current limiting (LEDs/Buzzer if needed) |
| Power Supply               | 5V (via USB or external supply)          |

---

## 🔌 Pin Configuration (Example)

| Function              | ESP8266 Pin |
| --------------------- | ----------- |
| MAX7219 DIN           | D7          |
| MAX7219 CS            | D8          |
| MAX7219 CLK           | D5          |
| Buzzer                | D1          |
| Touch / Switch        | D2          |
| Status LED (optional) | D4          |

> ⚠️ **Note:** Avoid using boot-critical pins (D3, D4) incorrectly to prevent boot issues.

---

## 📚 Libraries Used

Make sure the following libraries are installed in Arduino IDE:

* `ESP8266WiFi`
* `WiFiUdp`
* `NTPClient`
* `TimeLib`
* `MD_Parola`
* `MD_MAX72xx`
* `SPI`

---

## 🚀 How It Works

1. ESP8266 connects to Wi-Fi
2. Time is fetched from an **NTP server**
3. Time is formatted and displayed on the **LED matrix**
4. Touch sensor or switch triggers:

   * Display mode change
   * Buzzer sound / melody
5. Clock auto-updates time periodically

---

## 🛠 Setup Instructions

1. Clone this repository

   ```bash
   git clone https://github.com/bibek3221/esp-SmartClock.git
   ```

2. Open the project in **Arduino IDE**

3. Select board:

   * **NodeMCU 1.0 (ESP-12E Module)**

4. Enter your Wi-Fi credentials:

   ```cpp
   const char* ssid = "YOUR_WIFI_NAME";
   const char* password = "YOUR_WIFI_PASSWORD";
   ```

5. Upload the code to ESP8266

6. Power the board and enjoy ⏱️

---

## 🎵 Sound Functions

Songs are modularized into separate files/functions, for example:

```cpp
playHappyBirthday();
playChristmas();
```

You can easily add new melodies and call them when needed.

---

## 🧪 Future Enhancements

* 📅 Date & Day display
* 🌡 Temperature & Humidity (DHT11/DHT22)
* 🌦 Weather via API
* 🌐 Web dashboard control
* ⏰ Alarm scheduling
* 🔋 Battery backup

---

## 🧠 Learning Outcomes

* ESP8266 Wi-Fi handling
* NTP time synchronization
* LED matrix control
* Modular Arduino coding
* GPIO & hardware safety

---


## 👤 Author

**Bibek**
ESP8266 • IoT • Embedded Systems
🚀 Learning by building

---

