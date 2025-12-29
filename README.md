# SK Smart Watch Firmware (v2.0)

[![Platform](https://img.shields.io/badge/Platform-ESP8266%20%2F%20ESP32-blue)](https://arduino.cc)
[![License](https://img.shields.io/badge/License-MIT-green)](LICENSE)

A custom, feature-rich smartwatch firmware built for ESP8266 (NodeMCU/D1 Mini) and ESP32 microcontrollers. This project features a clean UI, weather integration, games, and utility tools on a 128x64 OLED display.

## 🚀 Features

* **Clock & Weather:**
    * NTP Time Synchronization.
    * Live Weather & Temperature (via Open-Meteo API).
    * Sunrise & Sunset Times.
    * **New in v2.0:** Daylight Duration Tracking.
    * Moon Phases calculation.
* **Menu System:** Smooth navigation with intuitive icons.
* **Utility Apps:**
    * **Stopwatch:** Counts minutes, seconds, and milliseconds.
    * **Wi-Fi Meter:** Visual signal strength analyzer.
    * **Flashlight:** Turns the screen white for emergency light.
    * **Julian Date:** Displays current day of the year.
    * **Day Stats:** View history (Sunrise, Sunset, Temp, Duration) for the last 3 days.
* **Games:**
    * **Dice Roll:** Digital 6-sided dice.
    * **Coin Toss:** Heads or Tails simulation.
* **System:**
    * **OTA Updates:** Update firmware wirelessly.
    * **Battery Monitoring:** Visual percentage readout.
    * **Deep Sleep:** Automatic screen timeout to save power.

## 🛠 Hardware Required

1.  **Microcontroller:** ESP8266 (NodeMCU / Wemos D1 Mini) or ESP32.
2.  **Display:** 0.96" OLED I2C (SSD1306 Driver).
3.  **Control:** 1x Push Button.
4.  **Battery:** 3.7V Li-ion/LiPo (with resistor divider on A0 for monitoring).

## 🔌 Circuit / Pinout

| Component | ESP Pin (Default) | Description |
| :--- | :--- | :--- |
| **OLED SDA** | D2 (GPIO 4) | I2C Data |
| **OLED SCL** | D1 (GPIO 5) | I2C Clock |
| **Button** | D5 (GPIO 14) | Active Low / OneButton One pin to D5 and second pin to gnd |
| **Battery** | positive 5v and Gnd to Gnd  | 
| **Voltage divider** | 330K resistor 3.3v to A0 and 47k resisito Gnd to A0 for Battry percentage |

## ⚙️ Configuration

Before uploading, open the firmware file and edit the **[1] CONFIGURATION** section:

```cpp
// 1. Enter your Wi-Fi Details
const char* WIFI_SSID     = "YOUR_WIFI_NAME";
const char* WIFI_PASS     = "YOUR_WIFI_PASSWORD"; 

// 2. Enter your Location (for Weather/Sun data)
// Get coordinates from: [https://www.latlong.net/](https://www.latlong.net/)
const String GEO_LAT      = "00.00"; 
const String GEO_LON      = "00.00";
🎮 Controls (Single Button)
Single Click: Wake screen / Next Menu Item / Action in App.

Double Click: Previous Menu Item / Back / Exit App.

Long Press: Select Menu Item / Sync Time / Reset Stopwatch.

📦 Libraries Used
ESP8266WiFi / ESP8266HTTPClient (Built-in)

Adafruit GFX & SSD1306: For OLED display handling.

OneButton: For advanced button click detection.

ArduinoOTA: For wireless updates.

📝 Changelog
v2.0 (Latest)

Code cleanup for public release.

Added Daylight Duration to Day Stats.

Optimized UI (Removed cluttered lines).

Generic configuration for GitHub.

v1.0 - v1.9

Initial development and feature additions.
