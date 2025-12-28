# ⌚ SK Smart Watch v1.6 (Pro Edition)

**Developed by:** Sudhir (SK Project)  
**Version:** v1.6 (Stable)  
**Platform:** ESP8266 (NodeMCU / Wemos D1 Mini)

## 📝 Description
This is a professional DIY Smartwatch project featuring a clean UI, real-time weather updates, stopwatch with milliseconds, and a fun decision-maker tool. The code is secured with Hex encoding for privacy.

## 🔥 Features
- **🕒 Accurate Clock:** Syncs time via Internet (NTP) with centered Date display.
- **☁️ Live Weather:** Real-time Temperature & Condition updates (Open-Meteo API).
- **⏱️ Stopwatch:** Precise timing with format `MM:SS:ms`.
- **🎲 Decision Maker:** Randomly answers your questions (Yes/No/Maybe).
- **📡 OTA Update:** Update firmware wirelessly without connecting cables.
- **🔋 Battery Monitor:** Shows battery percentage on the main screen.
- **🔒 Secured Core:** Name and branding are Hex Encoded.

## 🛠️ Hardware Required
1. **ESP8266 Board** (NodeMCU or Wemos D1 Mini)
2. **OLED Display** (0.96 inch I2C SSD1306)
3. **Push Button** (1x)
4. **LiPo Battery** (3.7V)
5. **Connecting Wires**

## 🔌 Circuit Diagram
Here is the connection layout:

```text
       +-------------------------+
       |   ESP8266 (NodeMCU)     |
       |                         |
       |             3.3V Pin +--+---------> OLED VCC
       |                  GND +--+----+----> OLED GND
       |                      |  |    |
       |          (SCL) D1 Pin+--+----+----> OLED SCL
       |          (SDA) D2 Pin+--+----+----> OLED SDA
       |                      |
       |          (Btn) D5 Pin+--+---------> [ Push Button ]
       |                      |  |                 |
       |                      |  +-----------------+
       |                      |
       |  (Bat Monitor) A0 Pin+--+---------> Battery (+)
       |                  VIN +--+---------> Battery (+)
       |                  GND +--+---------> Battery (-)
       +-------------------------+
## 📚 Libraries Used
Install these libraries from Arduino IDE Library Manager:
- `Adafruit GFX Library`
- `Adafruit SSD1306`
- `OneButton`
- `ArduinoOTA` (Built-in)
- `ESP8266WiFi` (Built-in)

## 🚀 How to Install
1. Open the `.ino` file in Arduino IDE.
2. Install the required libraries.
3. Update your **WiFi SSID & Password** in the code:
   ```cpp
   const char* WIFI_SSID = "YOUR_WIFI_NAME";
   const char* WIFI_PASS = "YOUR_WIFI_PASSWORD";
4.Upload to your ESP8266.
5.Enjoy your Smartwatch!
