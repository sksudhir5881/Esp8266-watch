/**
 * =================================================================================
 * PROJECT:      SK SMART WATCH FIRMWARE (Day Length Added)
 * VERSION:      v5.9 (Clean UI - No Lines, Day Duration Added)
 * PLATFORM:     ESP8266 / ESP32
 * =================================================================================
 */

// --- LIBRARIES ---
#include <ESP8266WiFi.h>      
#include <ESP8266HTTPClient.h> 
#include <WiFiClient.h>
#include <time.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoOTA.h>
#include "OneButton.h" 

// =================================================================================
// [1] CONFIGURATION & SETTINGS
// =================================================================================

const char* WIFI_SSID     = "vivo T4x 5G";
const char* WIFI_PASS     = "123456788"; 
const String GEO_LAT      = "30.54"; 
const String GEO_LON      = "75.51"; 

#define PIN_BUTTON        14  // D5 on NodeMCU
#define PIN_BATTERY       A0  // Analog Input
#define SLEEP_TIMEOUT     20000       
#define WEATHER_INTERVAL  3600000     
#define NTP_OFFSET        19800       

// =================================================================================
// [2] GRAPHICS (ICONS)
// =================================================================================

const unsigned char PROGMEM ICON_SUN_SMALL[] = { 0x24, 0x00, 0xBD, 0x3C, 0x3C, 0xBD, 0x00, 0x24 };
const unsigned char PROGMEM MOON_PHASES[8][8] = {
  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, {0x00, 0x00, 0x00, 0x06, 0x06, 0x00, 0x00, 0x00}, 
  {0x04, 0x0C, 0x0C, 0x0E, 0x0E, 0x0C, 0x0C, 0x04}, {0x1C, 0x3E, 0x3E, 0x3F, 0x3F, 0x3E, 0x3E, 0x1C}, 
  {0x3C, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x3C}, {0x38, 0x7C, 0x7C, 0xFC, 0xFC, 0x7C, 0x7C, 0x38}, 
  {0x20, 0x30, 0x30, 0x70, 0x70, 0x30, 0x30, 0x20}, {0x00, 0x00, 0x00, 0x60, 0x60, 0x00, 0x00, 0x00}  
};

// --- MENU ICONS (16x16) ---
const unsigned char PROGMEM MENU_ICON_STOPWATCH[] = { 0x03, 0xC0, 0x04, 0x20, 0x08, 0x10, 0x13, 0xC8, 0x24, 0x24, 0x48, 0x12, 0x40, 0x02, 0x80, 0x01, 0x80, 0x01, 0x81, 0x81, 0x80, 0x01, 0x40, 0x02, 0x40, 0x02, 0x20, 0x04, 0x10, 0x08, 0x0F, 0xF0 };
const unsigned char PROGMEM MENU_ICON_WIFI_METER[]= { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x05, 0x00, 0x05, 0x00, 0x15, 0x00, 0x15, 0x00, 0x55, 0x00, 0x55, 0x01, 0x55, 0x01, 0x55, 0x00, 0x00, 0x00, 0x00 };
const unsigned char PROGMEM MENU_ICON_FLASHLIGHT[]= { 0x03, 0xC0, 0x0C, 0x30, 0x10, 0x08, 0x27, 0xE4, 0x27, 0xE4, 0x27, 0xE4, 0x27, 0xE4, 0x20, 0x04, 0x20, 0x04, 0x20, 0x04, 0x10, 0x08, 0x10, 0x08, 0x08, 0x10, 0x08, 0x10, 0x07, 0xE0, 0x03, 0xC0 };
const unsigned char PROGMEM MENU_ICON_OTA[]       = { 0x00, 0x00, 0x03, 0xC0, 0x0C, 0x30, 0x30, 0x0C, 0x40, 0x02, 0x03, 0xC0, 0x0C, 0x30, 0x10, 0x08, 0x03, 0xC0, 0x04, 0x20, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x03, 0xC0 };
const unsigned char PROGMEM MENU_ICON_ABOUT[]     = { 0x00, 0x00, 0x03, 0xC0, 0x03, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x03, 0xC0, 0x03, 0xC0, 0x03, 0xC0, 0x03, 0xC0, 0x03, 0xC0, 0x03, 0xC0, 0x03, 0xC0, 0x00, 0x00, 0x00, 0x00 };
const unsigned char PROGMEM MENU_ICON_EXIT[]      = { 0x00, 0x00, 0xC0, 0x03, 0xE0, 0x07, 0x70, 0x0E, 0x38, 0x1C, 0x1C, 0x38, 0x0E, 0x70, 0x07, 0xE0, 0x07, 0xE0, 0x0E, 0x70, 0x1C, 0x38, 0x38, 0x1C, 0x70, 0x0E, 0xE0, 0x07, 0xC0, 0x03, 0x00, 0x00 };
const unsigned char PROGMEM MENU_ICON_DICE[]      = { 0x00, 0x00, 0x3F, 0xFC, 0x20, 0x04, 0x20, 0x04, 0x20, 0x04, 0x29, 0x44, 0x20, 0x04, 0x20, 0x04, 0x20, 0x04, 0x22, 0x84, 0x20, 0x04, 0x20, 0x04, 0x20, 0x04, 0x3F, 0xFC, 0x00, 0x00, 0x00, 0x00 };
const unsigned char PROGMEM MENU_ICON_COIN[]      = { 0x03, 0xC0, 0x0C, 0x30, 0x10, 0x08, 0x20, 0x04, 0x21, 0x84, 0x21, 0x84, 0x21, 0x84, 0x27, 0xE4, 0x21, 0x84, 0x21, 0x84, 0x21, 0x84, 0x20, 0x04, 0x10, 0x08, 0x0C, 0x30, 0x03, 0xC0, 0x00, 0x00 };
const unsigned char PROGMEM MENU_ICON_JULIAN[]    = { 0x00, 0x00, 0x1F, 0xF8, 0x10, 0x08, 0x17, 0xE8, 0x14, 0x28, 0x14, 0x28, 0x17, 0xE8, 0x10, 0x08, 0x10, 0x08, 0x10, 0x08, 0x10, 0x08, 0x10, 0x08, 0x10, 0x08, 0x10, 0x08, 0x1F, 0xF8, 0x00, 0x00 };
const unsigned char PROGMEM MENU_ICON_DAYSTATS[]  = { 0x00, 0x00, 0x18, 0x18, 0x3C, 0x3C, 0x7E, 0x7E, 0xFF, 0xFF, 0xFF, 0xFF, 0x7E, 0x7E, 0x3C, 0x3C, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

const unsigned char* MENU_ICONS_ARRAY[] = {
  MENU_ICON_STOPWATCH, MENU_ICON_WIFI_METER, MENU_ICON_FLASHLIGHT, 
  MENU_ICON_DICE, MENU_ICON_COIN, MENU_ICON_JULIAN,
  MENU_ICON_DAYSTATS, 
  MENU_ICON_OTA, MENU_ICON_ABOUT, MENU_ICON_EXIT
};

// =================================================================================
// [3] OBJECT INITIALIZATION
// =================================================================================
Adafruit_SSD1306 display(128, 64, &Wire, -1);
OneButton btn(PIN_BUTTON, true); 

enum SystemMode { 
    MODE_CLOCK, MODE_MENU, MODE_STOPWATCH, MODE_WIFI_METER, MODE_FLASHLIGHT, 
    MODE_DICE, MODE_COIN, MODE_JULIAN,
    MODE_DAYSTATS, 
    MODE_OTA, MODE_ABOUT, MODE_SLEEPING 
};
SystemMode currentMode = MODE_CLOCK;

const char* DAYS_OF_WEEK[] = {"SUNDAY", "MONDAY", "TUESDAY", "WEDNESDAY", "THURSDAY", "FRIDAY", "SATURDAY"};
const char* MONTHS[]       = {"JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};

const char* MENU_ITEMS[]   = {
  "Stopwatch", "WiFi Meter", "Flashlight", 
  "Dice Roll", "Coin Toss", "Julian Date",
  "Last 3 Days", 
  "OTA Update", "About", "Exit"
};
const int   MENU_COUNT     = 10; 

// =================================================================================
// [4] GLOBAL VARIABLES
// =================================================================================
int menuIndex = 0; 
unsigned long lastInteractionTime = 0; 
unsigned long lastWeatherUpdate = 0; 

String currentTemp = "--"; 
String currentCond = "No Data"; 

// 3 DAYS HISTORY VARS (Index 0 = Day-2, Index 1 = Yesterday, Index 2 = Today)
String histSunrise[3]  = {"--:--", "--:--", "--:--"};
String histSunset[3]   = {"--:--", "--:--", "--:--"};
String histMin[3]      = {"--", "--", "--"};
String histMax[3]      = {"--", "--", "--"};
String histDate[3]     = {"--", "--", "--"};
String histDuration[3] = {"--", "--", "--"}; // New: Day Length

int dayStatsPage = 2; // Start showing Today (Index 2)

int isDayTime = 1; 
bool isWiFiActive = false; 
bool isTimeSynced = false; 

// Vars
int coinState = 0; 
int finalCoinSide = 0; 
int diceState = 0; 
int diceValue = 1;   
unsigned long stopwatchStart = 0; 
unsigned long stopwatchElapsed = 0; 
bool stopwatchRunning = false;

// =================================================================================
// [5] FUNCTION PROTOTYPES
// =================================================================================
void systemSetup();
void connectWiFiSync();
void stopWiFiNetwork();
void handleOTA();
void enterSleepMode();
void wakeUpSystem();
void fetchWeatherData();
int  readBatteryPercentage();
void renderClock();
void renderMenu(); 
void renderStopwatch();
void renderWiFiMeter(); 
void renderFlashlight();
void renderDice();     
void renderCoinToss();   
void renderJulian();   
void renderDayStats(); 
void renderAbout();
void renderOTAScreen();
void onClick();
void onLongPress();
void onDoubleClick();
String extractJsonValue(String json, String key);
String extractArrayElement(String json, String key, int index);
String convertTime12H(String rawTime);
String formatDateSimple(String rawDate);
String formatDuration(float seconds);
String parseWeatherCode(int code);
int calculateMoonPhase(time_t now);

// =================================================================================
// [6] MAIN SETUP & LOOP
// =================================================================================
void setup() {
    Serial.begin(115200);
    pinMode(PIN_BATTERY, INPUT); 
    randomSeed(analogRead(0)); 

    WiFi.mode(WIFI_OFF);
    WiFi.forceSleepBegin();
    delay(1);

    btn.attachClick(onClick);
    btn.attachLongPressStart(onLongPress);
    btn.attachDoubleClick(onDoubleClick);
    btn.setClickTicks(250); 
    btn.setPressTicks(500); 

    Wire.begin(); 
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println(F("SSD1306 Allocation Failed"));
        ESP.restart();
    }
    
    display.ssd1306_command(SSD1306_SETCONTRAST);
    display.ssd1306_command(100); 

    display.clearDisplay(); 
    display.setTextColor(WHITE); 
    display.setTextSize(3); display.setCursor(46, 15); display.print(F("SK"));   
    display.setTextSize(1); display.setCursor(42, 45); display.print(F("Pro Watch"));    
    display.setCursor(55, 55); display.print(F("v5.9"));    
    display.display();
    delay(2000); 

    connectWiFiSync();
    lastInteractionTime = millis(); 
}

void loop() {
    btn.tick(); 
    
    if (isWiFiActive && currentMode == MODE_OTA) ArduinoOTA.handle();

    bool preventSleep = (currentMode == MODE_WIFI_METER || currentMode == MODE_FLASHLIGHT || 
                         (currentMode == MODE_STOPWATCH && stopwatchRunning));
    
    if (currentMode != MODE_SLEEPING && currentMode != MODE_OTA && !preventSleep) {
        if (millis() - lastInteractionTime > SLEEP_TIMEOUT) enterSleepMode();
    }
    
    if (currentMode == MODE_SLEEPING) { 
        delay(10); 
        return; 
    }

    if (currentMode == MODE_CLOCK && (millis() - lastWeatherUpdate > WEATHER_INTERVAL)) {
        connectWiFiSync(); 
        currentMode = MODE_CLOCK;
    }

    switch (currentMode) {
        case MODE_CLOCK:     renderClock();     break;
        case MODE_MENU:      renderMenu();      break;
        case MODE_STOPWATCH: renderStopwatch(); break;
        case MODE_WIFI_METER:renderWiFiMeter(); break; 
        case MODE_FLASHLIGHT:renderFlashlight();break;
        case MODE_DICE:      renderDice();      break; 
        case MODE_COIN:      renderCoinToss();  break; 
        case MODE_JULIAN:    renderJulian();    break; 
        case MODE_DAYSTATS:  renderDayStats();  break; 
        case MODE_ABOUT:     renderAbout();     break;
        case MODE_OTA:       renderOTAScreen(); break;
        default:             break;
    }
    delay(10); 
}

// =================================================================================
// [7] SYSTEM FUNCTIONS
// =================================================================================

int readBatteryPercentage() {
    int raw = analogRead(PIN_BATTERY);
    int pct = map(raw, 390, 545, 0, 100); 
    return constrain(pct, 0, 100);
}

void connectWiFiSync() {
    display.clearDisplay(); display.setCursor(10,20); display.print(F("Connecting...")); display.display();
    WiFi.forceSleepWake(); delay(1);
    WiFi.mode(WIFI_STA); WiFi.begin(WIFI_SSID, WIFI_PASS);
    int attempts = 0; 
    while(WiFi.status() != WL_CONNECTED && attempts < 20) { delay(500); attempts++; }
    if(WiFi.status() == WL_CONNECTED) { 
        display.clearDisplay(); display.setCursor(10,20); display.print(F("Sync Time...")); display.display();
        configTime(NTP_OFFSET, 0, "pool.ntp.org"); 
        int retry = 0; while (time(nullptr) < 100000 && retry < 20) { delay(500); retry++; }
        if(time(nullptr) > 100000) isTimeSynced = true;
        display.clearDisplay(); display.setCursor(10,20); display.print(F("Weather...")); display.display();
        fetchWeatherData(); 
        lastWeatherUpdate = millis(); 
    }
    if(currentMode != MODE_WIFI_METER) stopWiFiNetwork();
}

void handleOTA() {
    display.clearDisplay(); display.setCursor(10,20); display.print(F("Starting OTA...")); display.display();
    WiFi.forceSleepWake(); delay(1);
    WiFi.disconnect(); delay(100); 
    WiFi.mode(WIFI_STA); WiFi.begin(WIFI_SSID, WIFI_PASS);
    int retries = 0; while((WiFi.status() != WL_CONNECTED || WiFi.localIP() == IPAddress(0,0,0,0)) && retries < 60) { delay(500); retries++; }
    if(WiFi.status() == WL_CONNECTED) { 
        ArduinoOTA.setHostname("Sudhir-Watch-Pro"); ArduinoOTA.begin(); isWiFiActive = true; 
    } else { stopWiFiNetwork(); }
}

void stopWiFiNetwork() { 
    WiFi.disconnect(); WiFi.mode(WIFI_OFF); WiFi.forceSleepBegin(); delay(1); isWiFiActive = false; 
}

void enterSleepMode() { 
    display.ssd1306_command(SSD1306_DISPLAYOFF); stopWiFiNetwork(); currentMode = MODE_SLEEPING; 
}

void wakeUpSystem() { 
    display.ssd1306_command(SSD1306_DISPLAYON); currentMode = MODE_CLOCK; lastInteractionTime = millis(); 
}

int calculateMoonPhase(time_t now) {
    struct tm* t = localtime(&now);
    int y = t->tm_year + 1900; int m = t->tm_mon + 1; int d = t->tm_mday;
    double c = 0, e = 0, jd = 0; int b = 0;
    if (m < 3) { y--; m += 12; } ++m;
    c = 365.25 * y; e = 30.6 * m;
    jd = c + e + d - 694039.09; jd /= 29.53058;
    b = (int)jd; jd -= b; b = round(jd * 8); 
    if (b >= 8) b = 0; return b;
}

// =================================================================================
// [8] GRAPHICS & RENDERERS
// =================================================================================

void renderClock() {
    time_t now = time(nullptr); struct tm* t = localtime(&now);
    display.clearDisplay(); 
    int bat = readBatteryPercentage();
    display.setTextSize(1); display.setCursor(95, 0); display.print(bat); display.print(F("%"));
    if(isDayTime == 1) { display.drawBitmap(5, 4, ICON_SUN_SMALL, 8, 8, WHITE); } 
    else { int phase = calculateMoonPhase(now); display.drawBitmap(5, 4, MOON_PHASES[phase], 8, 8, WHITE); }
    String dayName = DAYS_OF_WEEK[t->tm_wday];
    int textWidth = dayName.length() * 6; int centerX = (128 - textWidth) / 2;
    display.setCursor(centerX, 4); display.print(dayName); 
    display.drawLine(0,15,128,15,WHITE);
    int h = t->tm_hour; bool pm = h >= 12; if(h > 12) h -= 12; if(h == 0) h = 12;
    display.setTextSize(2); display.setCursor(10,25); display.printf("%02d:%02d:%02d", h, t->tm_min, t->tm_sec);
    display.setTextSize(1); display.setCursor(110,32); display.print(pm ? "PM" : "AM");
    display.setCursor(30,44); 
    if(t->tm_year < 100) display.print(F("??-????-????")); else display.printf("%02d %s %d", t->tm_mday, MONTHS[t->tm_mon], t->tm_year+1900); 
    display.setCursor(5,56); display.print(currentCond); 
    display.setCursor(92,56); display.print(currentTemp); display.print(F("C"));
    display.display();
}

void renderMenu() {
    display.clearDisplay(); display.setTextSize(1); display.setCursor(45,0); display.print(F("MENU")); 
    display.drawLine(0, 10, 128, 10, WHITE);
    display.fillTriangle(10, 30, 18, 25, 18, 35, WHITE);  
    display.fillTriangle(118, 30, 110, 25, 110, 35, WHITE); 
    display.drawBitmap(56, 20, MENU_ICONS_ARRAY[menuIndex], 16, 16, WHITE);
    String label = MENU_ITEMS[menuIndex]; int w = label.length() * 6; int x = (128 - w) / 2;
    display.setCursor(x, 45); display.print(label);
    for(int i=0; i<MENU_COUNT; i++) {
        if(i == menuIndex) display.fillRect(20 + (i*8), 58, 3, 3, WHITE); else display.drawPixel(21 + (i*8), 59, WHITE);
    }
    display.display();
}

void renderDayStats() {
    display.clearDisplay();
    display.setTextSize(1); 
    
    // Header based on page index
    display.setCursor(0, 0); 
    if(dayStatsPage == 2) display.print(F("TODAY "));
    else if(dayStatsPage == 1) display.print(F("YESTERDAY "));
    else display.print(F("2 DAYS AGO "));
    
    display.print(histDate[dayStatsPage]); // Date

    // NOTE: Removed the long horizontal lines as requested
    
    // Sunrise & Sunset
    display.setCursor(5, 12); display.print(F("Rise: ")); display.print(histSunrise[dayStatsPage]);
    display.setCursor(5, 22); display.print(F("Set : ")); display.print(histSunset[dayStatsPage]);

    // Temp Range
    display.setCursor(5, 34); display.print(F("Min : ")); display.print(histMin[dayStatsPage]); display.print(F(" C"));
    display.setCursor(5, 44); display.print(F("Max : ")); display.print(histMax[dayStatsPage]); display.print(F(" C"));

    // NEW: Day Length (Duration)
    display.setCursor(5, 56); display.print(F("Len : ")); display.print(histDuration[dayStatsPage]);

    // Indicator dots
    for(int i=0; i<3; i++) {
       if(i == dayStatsPage) display.fillCircle(120, 20 + (i*10), 2, WHITE);
       else display.drawCircle(120, 20 + (i*10), 2, WHITE);
    }

    display.display();
}

void renderDice() {
    display.clearDisplay();
    display.setTextSize(1); display.setCursor(40, 0); display.print(F("DICE")); display.drawLine(0,10,128,10,WHITE);
    
    if(diceState == 0) {
        display.drawRect(50, 20, 28, 28, WHITE);
        display.setTextSize(2); display.setCursor(57, 27); display.print(diceValue);
        display.setTextSize(1); display.setCursor(25, 55); display.print(F("Click to Roll"));
    } else if(diceState == 1) {
        for(int i=0; i<10; i++) {
            display.fillRect(50, 20, 28, 28, BLACK); 
            display.drawRect(50, 20, 28, 28, WHITE); 
            display.setTextSize(2); display.setCursor(57, 27); 
            display.print(random(1, 7)); 
            display.display();
            delay(50 + (i*15)); 
        }
        diceValue = random(1, 7); 
        diceState = 2; 
    } else {
        display.drawRect(50, 20, 28, 28, WHITE);
        display.setTextSize(2); display.setCursor(57, 27); display.print(diceValue);
        display.setTextSize(1); display.setCursor(35, 55); display.print(F("Result!"));
    }
    display.display();
}

void renderCoinToss() {
    display.clearDisplay();
    display.setTextSize(1); display.setCursor(35, 0); display.print(F("COIN TOSS")); display.drawLine(0, 10, 128, 10, WHITE);
    
    if(coinState == 0) { 
        display.setCursor(35, 30); display.print(F("CLICK TO")); 
        display.setCursor(45, 45); display.print(F("FLIP")); 
    } 
    else if(coinState == 1) { 
        for(int i=0; i<12; i++) {
            display.fillRect(0, 20, 128, 44, BLACK); 
            display.setTextSize(2); 
            if(i % 2 == 0) { display.setCursor(35, 30); display.print(F("HEADS")); }
            else { display.setCursor(35, 30); display.print(F("TAILS")); }
            display.display();
            delay(100 + (i*10)); 
        }
        finalCoinSide = random(0, 2); 
        coinState = 2; 
    } 
    else { 
        display.setTextSize(2); 
        if(finalCoinSide == 0) { display.setCursor(35, 30); display.print(F("HEADS")); } 
        else { display.setCursor(35, 30); display.print(F("TAILS")); } 
    }
    display.display();
}

void renderJulian() {
    time_t now = time(nullptr); struct tm* t = localtime(&now);
    display.clearDisplay();
    display.setTextSize(1); display.setCursor(30, 0); display.print(F("JULIAN DATE")); display.drawLine(0, 10, 128, 10, WHITE);
    display.setTextSize(2); display.setCursor(45, 25); display.print(t->tm_yday + 1); 
    display.setTextSize(1); display.setCursor(35, 50); display.print(F("Day of Year"));
    display.display();
}

void renderWiFiMeter() {
    display.clearDisplay();
    display.setTextSize(1); display.setCursor(0,0); display.print(F("WIFI SIGNAL")); display.drawLine(0,10,128,10, WHITE);
    if (WiFi.status() == WL_CONNECTED) {
        long rssi = WiFi.RSSI(); int quality = 2 * (rssi + 100); if(quality>100)quality=100; if(quality<0)quality=0;
        String ssid = WiFi.SSID(); if(ssid.length() > 14) ssid = ssid.substring(0,14);
        display.setCursor(0, 20); display.print("Net: "); display.print(ssid);
        display.setCursor(0, 32); display.print("Sig: "); display.print(rssi); display.print(" dBm");
        display.setCursor(80, 32); display.print(quality); display.print("%");
        display.drawRect(10, 48, 108, 10, WHITE); int fillW = map(quality, 0, 100, 0, 104);
        display.fillRect(12, 50, fillW, 6, WHITE); 
    } else { display.setCursor(20, 30); display.print(F("Not Connected")); }
    display.display();
}

void renderFlashlight() { display.fillScreen(WHITE); display.display(); }

void renderStopwatch() {
    unsigned long currentDelta = (stopwatchRunning) ? (millis() - stopwatchStart) : 0;
    unsigned long totalTime = stopwatchElapsed + currentDelta;
    unsigned long mins = (totalTime / 60000); unsigned long secs = (totalTime % 60000) / 1000; unsigned long ms = (totalTime % 1000) / 10; 
    display.clearDisplay(); display.setTextSize(2); display.setCursor(15, 25); 
    display.printf("%02d:%02d:%02d", mins, secs, ms);
    display.setTextSize(1); display.setCursor(40, 55); display.print(stopwatchRunning ? F("RUNNING") : F("STOPPED")); 
    display.display();
}

void renderOTAScreen() {
    display.clearDisplay(); display.setTextSize(1); display.setCursor(30,5); display.print(F("OTA MODE")); display.drawLine(0,15,128,15,WHITE);
    display.setCursor(0,25); display.print(F("Net: ")); display.print(WiFi.SSID()); display.setCursor(0,45); display.print(F("IP: ")); display.print(WiFi.localIP()); display.display(); 
}

void renderAbout() {
    display.clearDisplay(); display.setTextSize(1); display.setCursor(45, 0); display.print(F("ABOUT")); display.drawLine(0, 10, 128, 10, WHITE);
    display.setCursor(10, 20); display.print(F("SK Project")); display.setCursor(10, 35); display.print(F("Version: v5.9")); display.setCursor(10, 50); display.print(F("Dev: Sudhir")); display.display();
}

// =================================================================================
// [9] BUTTON LOGIC
// =================================================================================

void onClick() {
    lastInteractionTime = millis();
    if (currentMode == MODE_SLEEPING) { wakeUpSystem(); return; }

    switch (currentMode) {
        case MODE_CLOCK: currentMode = MODE_MENU; break;
        case MODE_MENU: menuIndex++; if (menuIndex >= MENU_COUNT) menuIndex = 0; break;
        case MODE_STOPWATCH: if(stopwatchRunning) { stopwatchElapsed += millis() - stopwatchStart; stopwatchRunning = false; } else { stopwatchStart = millis(); stopwatchRunning = true; } break;
        
        case MODE_COIN: 
            if(coinState == 0) coinState = 1; 
            else coinState = 0; 
            break;
            
        case MODE_DICE: 
            if(diceState == 0) diceState = 1; 
            else diceState = 0; 
            break; 

        case MODE_DAYSTATS: 
            // Cycle pages: 2->1->0->2 (Today -> Yesterday -> 2 Days Ago)
            dayStatsPage--; 
            if(dayStatsPage < 0) dayStatsPage = 2; 
            break;

        case MODE_WIFI_METER: break; 
        case MODE_FLASHLIGHT: currentMode = MODE_MENU; break;
        case MODE_ABOUT: 
        case MODE_OTA: stopWiFiNetwork(); currentMode = MODE_MENU; break;
        default: break;
    }
}

void onDoubleClick() {
    lastInteractionTime = millis();
    if (currentMode == MODE_SLEEPING) { wakeUpSystem(); return; }
    if(currentMode == MODE_MENU) { menuIndex--; if(menuIndex < 0) menuIndex = MENU_COUNT - 1; return; }
    if (currentMode == MODE_CLOCK) { handleOTA(); currentMode = MODE_OTA; return; }
    if (currentMode == MODE_STOPWATCH) stopwatchRunning = false;
    if (currentMode == MODE_OTA) stopWiFiNetwork(); 
    if (currentMode != MODE_CLOCK) currentMode = MODE_MENU; 
}

void onLongPress() {
    lastInteractionTime = millis();
    if (currentMode == MODE_SLEEPING) { wakeUpSystem(); return; }

    switch (currentMode) {
        case MODE_CLOCK: connectWiFiSync(); break;
        case MODE_MENU: 
            if(menuIndex == 0) currentMode = MODE_STOPWATCH;       
            else if(menuIndex == 1) { currentMode = MODE_WIFI_METER; if(WiFi.status() != WL_CONNECTED) connectWiFiSync(); }
            else if(menuIndex == 2) currentMode = MODE_FLASHLIGHT; 
            else if(menuIndex == 3) currentMode = MODE_DICE; 
            else if(menuIndex == 4) currentMode = MODE_COIN; 
            else if(menuIndex == 5) currentMode = MODE_JULIAN; 
            else if(menuIndex == 6) { currentMode = MODE_DAYSTATS; dayStatsPage = 2; } // Reset to Today
            else if(menuIndex == 7) { handleOTA(); currentMode = MODE_OTA; } 
            else if(menuIndex == 8) currentMode = MODE_ABOUT;      
            else if(menuIndex == 9) currentMode = MODE_CLOCK; 
            break;
        case MODE_STOPWATCH: stopwatchRunning = false; stopwatchElapsed = 0; break;
        case MODE_DICE:
        case MODE_COIN:
        case MODE_JULIAN:
        case MODE_DAYSTATS:
        case MODE_WIFI_METER: 
        case MODE_FLASHLIGHT: stopWiFiNetwork(); currentMode = MODE_MENU; break;
        case MODE_ABOUT: 
        case MODE_OTA: stopWiFiNetwork(); currentMode = MODE_MENU; break;
        default: break;
    }
}

// =================================================================================
// [10] API HELPERS
// =================================================================================

void fetchWeatherData() {
    WiFiClient client; HTTPClient http; http.setTimeout(5000); 
    // Added 'daylight_duration' to request
    String url = "http://api.open-meteo.com/v1/forecast?latitude=" + GEO_LAT + "&longitude=" + GEO_LON + "&current_weather=true&daily=sunrise,sunset,temperature_2m_max,temperature_2m_min,daylight_duration&past_days=2&forecast_days=1&timezone=auto";
    
    http.begin(client, url);
    if(http.GET() > 0) {
        String payload = http.getString();
        
        // 1. Parse Current Weather
        int idx = payload.indexOf("\"current_weather\":");
        if(idx != -1) { 
            String wData = payload.substring(idx); 
            String t = extractJsonValue(wData, "temperature"); 
            String c = extractJsonValue(wData, "weathercode");
            String d = extractJsonValue(wData, "is_day");
            if(d != "") isDayTime = d.toInt(); 
            if(t != "" && c != "") { currentTemp = t; currentCond = parseWeatherCode(c.toInt()); }
        }

        // 2. Parse 3 Days History (Arrays of size 3)
        for(int i=0; i<3; i++) {
           String rawDate = extractArrayElement(payload, "time", i);
           String rawRise = extractArrayElement(payload, "sunrise", i);
           String rawSet  = extractArrayElement(payload, "sunset", i);
           String rawMax  = extractArrayElement(payload, "temperature_2m_max", i);
           String rawMin  = extractArrayElement(payload, "temperature_2m_min", i);
           String rawDur  = extractArrayElement(payload, "daylight_duration", i); // Get Duration (Seconds)

           if(rawDate != "") histDate[i] = formatDateSimple(rawDate);
           if(rawRise != "") histSunrise[i] = convertTime12H(rawRise);
           if(rawSet != "")  histSunset[i]  = convertTime12H(rawSet);
           if(rawMax != "")  histMax[i] = rawMax;
           if(rawMin != "")  histMin[i] = rawMin;
           if(rawDur != "")  histDuration[i] = formatDuration(rawDur.toFloat());
        }
    } 
    http.end();
}

String extractJsonValue(String json, String key) {
    String search = "\"" + key + "\":"; int start = json.indexOf(search); if (start == -1) return ""; 
    start += search.length(); int end = json.indexOf(",", start); if (end == -1) end = json.indexOf("}", start);
    if (end != -1) { String val = json.substring(start, end); val.trim(); return val; } 
    return "";
}

String extractArrayElement(String json, String key, int index) {
    String search = "\"" + key + "\":["; 
    int start = json.indexOf(search); 
    if (start == -1) return ""; 
    start += search.length();
    
    int currentIdx = 0;
    while(currentIdx < index) {
        int comma = json.indexOf(",", start);
        if(comma == -1) return ""; 
        start = comma + 1;
        currentIdx++;
    }

    int end = json.indexOf(",", start);
    int closeBracket = json.indexOf("]", start);
    
    if (end == -1 || (closeBracket != -1 && closeBracket < end)) {
        end = closeBracket;
    }
    
    if (end != -1) {
        String val = json.substring(start, end);
        val.replace("\"", ""); 
        val.trim();
        return val;
    }
    return "";
}

String convertTime12H(String rawTime) {
    int tIndex = rawTime.indexOf("T");
    if(tIndex == -1) return rawTime; 
    
    String timePart = rawTime.substring(tIndex + 1); 
    int hour = timePart.substring(0, 2).toInt();
    String minute = timePart.substring(3, 5);
    
    String ampm = "AM";
    if (hour >= 12) {
        ampm = "PM";
        if (hour > 12) hour -= 12;
    }
    if (hour == 0) hour = 12;
    
    return String(hour) + ":" + minute + " " + ampm;
}

String formatDateSimple(String rawDate) {
   if(rawDate.length() < 10) return "";
   String mStr = rawDate.substring(5, 7);
   String dStr = rawDate.substring(8, 10);
   int m = mStr.toInt();
   if(m >= 1 && m <= 12) return "(" + dStr + " " + String(MONTHS[m-1]) + ")";
   return "";
}

// NEW Helper: Convert Seconds to "HHh MMm"
String formatDuration(float seconds) {
    int totalMin = seconds / 60;
    int h = totalMin / 60;
    int m = totalMin % 60;
    return String(h) + "h " + String(m) + "m";
}

String parseWeatherCode(int code) { 
    if(code == 0) return "CLEAR"; if(code <= 3) return "CLOUDY"; if(code <= 48) return "FOGGY"; 
    if(code <= 65) return "RAIN"; if(code >= 95) return "STORM"; return "STABLE"; 
}
