/**
 * =================================================================================
 * PROJECT:      SK SMART WATCH FIRMWARE
 * VERSION:      v1.6 (Date Centered)
 * AUTHOR:       HEX ENCODED (Security Enabled)
 * PLATFORM:     ESP8266 / ESP32 (Compatible)
 * DESCRIPTION:  IoT Watch with Stopwatch, Decision Maker, OTA & Weather.
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

// --- WiFi Credentials ---
const char* WIFI_SSID     = "vivo T4x 5G";
const char* WIFI_PASS     = "123456788"; 

// --- Location (Lat/Lon) ---
const String GEO_LAT      = "30.54"; 
const String GEO_LON      = "75.51"; 

// --- Hardware Pins ---
#define PIN_BUTTON        14  // D5 on NodeMCU
#define PIN_BATTERY       A0  // Analog Input

// --- System Constants ---
#define SLEEP_TIMEOUT     15000       // 15 Seconds to Auto-Sleep
#define WEATHER_INTERVAL  1800000     // 30 Minutes (1800000 ms)
#define NTP_OFFSET        19800       // UTC +5:30 (India)

// =================================================================================
// [2] SECURITY & OBFUSCATION (HEX ENCODED)
// =================================================================================
// NOTE: Strings are encoded to prevent unauthorized modification via text search.
// ---------------------------------------------------------------------------------
const char _SEC_PROJ[]    = {0x53, 0x4B, 0x20, 0x50, 0x72, 0x6F, 0x6A, 0x65, 0x63, 0x74, 0x00}; // "SK Project"
const char _SEC_DEV[]     = {0x53, 0x75, 0x64, 0x68, 0x69, 0x72, 0x00};                         // "Sudhir"
const char _SEC_VER[]     = {0x76, 0x31, 0x2E, 0x30, 0x00};                                     // "v1.0"
const char _SEC_LOGO[]    = {0x53, 0x4B, 0x00};                                                 // "SK"
const char _SEC_SUB[]     = {0x50, 0x72, 0x6F, 0x6A, 0x65, 0x63, 0x74, 0x00};                   // "Project"

// =================================================================================
// [3] OBJECT INITIALIZATION
// =================================================================================
Adafruit_SSD1306 display(128, 64, &Wire, -1);
OneButton btn(PIN_BUTTON, true); 

// --- System Enums ---
enum SystemMode { 
    MODE_CLOCK, 
    MODE_MENU, 
    MODE_STOPWATCH, 
    MODE_DECISION, 
    MODE_OTA, 
    MODE_ABOUT, 
    MODE_SLEEPING 
};

SystemMode currentMode = MODE_CLOCK;

// --- Data Arrays ---
const char* DAYS_OF_WEEK[] = {"SUNDAY", "MONDAY", "TUESDAY", "WEDNESDAY", "THURSDAY", "FRIDAY", "SATURDAY"};
const char* MONTHS[]       = {"JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};

// --- Menu Configuration ---
const char* MENU_ITEMS[]   = {"OTA Update", "Stopwatch", "Decision Maker", "About"};
const int   MENU_COUNT     = 4; 

// --- Decision Maker Answers ---
const char* ANSWERS[]      = {"YES", "NO", "MAYBE", "TRY AGAIN", "DEFINITELY", "NO WAY!"};

// =================================================================================
// [4] GLOBAL VARIABLES
// =================================================================================
int menuIndex = 0; 
int menuTopLine = 0; 
int currentAnswerIndex = -1;

unsigned long lastInteractionTime = 0; 
unsigned long lastWeatherUpdate = 0; 

// Weather Data
String currentTemp = "--"; 
String currentCond = "No Data"; 
bool isWiFiActive = false; 
bool isTimeSynced = false; 

// Stopwatch Data
unsigned long stopwatchStart = 0; 
unsigned long stopwatchElapsed = 0; 
bool stopwatchRunning = false;

// =================================================================================
// [5] FUNCTION PROTOTYPES
// =================================================================================
void systemSetup();
void systemLoop();
void connectWiFiSync();
void stopWiFiNetwork();
void handleOTA();
void enterSleepMode();
void wakeUpSystem();
void fetchWeatherData();
int  readBatteryPercentage();

// UI Functions
void renderClock();
void renderMenu();
void renderStopwatch();
void renderDecision();
void renderAbout();
void renderOTAScreen();

// Button Callbacks
void onClick();
void onLongPress();
void onDoubleClick();

// Helpers
String extractJsonValue(String json, String key);
String parseWeatherCode(int code);

// =================================================================================
// [6] MAIN SETUP & LOOP
// =================================================================================
void setup() {
    Serial.begin(115200);
    
    // Hardware Init
    pinMode(PIN_BATTERY, INPUT); 
    randomSeed(analogRead(0)); 

    // Button Config
    btn.attachClick(onClick);
    btn.attachLongPressStart(onLongPress);
    btn.attachDoubleClick(onDoubleClick);
    btn.setClickTicks(250); 
    btn.setPressTicks(500); 

    // Display Init
    Wire.begin(); 
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println(F("SSD1306 Allocation Failed"));
        ESP.restart();
    }
    
    // --- BOOT SEQUENCE (SECURED) ---
    display.clearDisplay(); 
    display.setTextColor(WHITE); 
    
    display.setTextSize(3);
    display.setCursor(46, 15); 
    display.print(_SEC_LOGO);   // "SK"
    
    display.setTextSize(1); 
    display.setCursor(42, 45); 
    display.print(_SEC_SUB);    // "Project"
    
    display.setCursor(55, 55); 
    display.print(_SEC_VER);    // "v1.0"
    
    display.display();
    delay(2000); 

    // Initial Data Sync
    connectWiFiSync();
    lastInteractionTime = millis(); 
}

void loop() {
    btn.tick(); 
    
    // OTA Handler
    if (isWiFiActive && currentMode == MODE_OTA) {
        ArduinoOTA.handle();
    }

    // Auto Sleep Logic
    if (currentMode != MODE_SLEEPING && currentMode != MODE_OTA) {
        if (millis() - lastInteractionTime > SLEEP_TIMEOUT) {
            enterSleepMode();
        }
    }
    
    // Sleep State
    if (currentMode == MODE_SLEEPING) { 
        delay(50); 
        return; 
    }

    // Auto Weather Update
    if (currentMode == MODE_CLOCK && (millis() - lastWeatherUpdate > WEATHER_INTERVAL)) {
        connectWiFiSync(); 
        currentMode = MODE_CLOCK;
    }

    // UI State Machine
    switch (currentMode) {
        case MODE_CLOCK:     renderClock();     break;
        case MODE_MENU:      renderMenu();      break;
        case MODE_STOPWATCH: renderStopwatch(); break;
        case MODE_DECISION:  renderDecision();  break;
        case MODE_ABOUT:     renderAbout();     break;
        case MODE_OTA:       renderOTAScreen(); break;
        default:             break;
    }
}

// =================================================================================
// [7] SYSTEM FUNCTIONS
// =================================================================================

int readBatteryPercentage() {
    int raw = analogRead(PIN_BATTERY);
    // Map analog value 390-545 to 0-100%
    int pct = map(raw, 390, 545, 0, 100); 
    return constrain(pct, 0, 100);
}

void connectWiFiSync() {
    display.clearDisplay(); 
    display.setCursor(10,20); display.print(F("Connecting...")); 
    display.display();
    
    WiFi.mode(WIFI_STA); 
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    
    int attempts = 0; 
    while(WiFi.status() != WL_CONNECTED && attempts < 20) { 
        delay(500); 
        attempts++; 
    }

    if(WiFi.status() == WL_CONNECTED) { 
        // Sync Time
        display.clearDisplay(); 
        display.setCursor(10,20); display.print(F("Sync Time...")); 
        display.display();
        
        configTime(NTP_OFFSET, 0, "pool.ntp.org"); 
        
        int retry = 0; 
        while (time(nullptr) < 100000 && retry < 20) { 
            delay(500); 
            retry++; 
        }
        
        if(time(nullptr) > 100000) isTimeSynced = true;

        // Sync Weather
        display.clearDisplay(); 
        display.setCursor(10,20); display.print(F("Weather...")); 
        display.display();
        fetchWeatherData(); 
        
        lastWeatherUpdate = millis(); 
    } else { 
        currentCond = "No WiFi"; 
    }
    stopWiFiNetwork();
}

void handleOTA() {
    display.clearDisplay(); 
    display.setCursor(10,20); display.print(F("Starting OTA...")); 
    display.display();
    
    WiFi.disconnect(); 
    delay(100); 
    WiFi.mode(WIFI_STA); 
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    
    int retries = 0; 
    while((WiFi.status() != WL_CONNECTED || WiFi.localIP() == IPAddress(0,0,0,0)) && retries < 60) { 
        delay(500); 
        retries++; 
    }
    
    if(WiFi.status() == WL_CONNECTED) { 
        ArduinoOTA.setHostname("Sudhir-Watch-Pro"); 
        ArduinoOTA.begin(); 
        isWiFiActive = true; 
    } else { 
        stopWiFiNetwork(); 
    }
}

void stopWiFiNetwork() { 
    WiFi.disconnect(); 
    WiFi.mode(WIFI_OFF); 
    WiFi.forceSleepBegin(); 
    delay(1); 
    isWiFiActive = false; 
}

void enterSleepMode() { 
    display.ssd1306_command(SSD1306_DISPLAYOFF); 
    currentMode = MODE_SLEEPING; 
}

void wakeUpSystem() { 
    display.ssd1306_command(SSD1306_DISPLAYON); 
    currentMode = MODE_CLOCK; 
    lastInteractionTime = millis(); 
}

// =================================================================================
// [8] GRAPHICS & RENDERERS
// =================================================================================

void renderClock() {
    time_t now = time(nullptr); 
    struct tm* t = localtime(&now);
    
    display.clearDisplay(); 
    
    // Top Bar (Battery)
    int bat = readBatteryPercentage();
    display.setTextSize(1); 
    display.setCursor(95, 0); 
    display.print(bat); display.print(F("%"));
    
    // Day
    display.setCursor(40,4); 
    display.print(DAYS_OF_WEEK[t->tm_wday]); 
    display.drawLine(0,15,128,15,WHITE);
    
    // Time (HH:MM:SS)
    int h = t->tm_hour; 
    bool pm = h >= 12; 
    if(h > 12) h -= 12; 
    if(h == 0) h = 12;
    
    display.setTextSize(2); 
    display.setCursor(10,25); 
    display.printf("%02d:%02d:%02d", h, t->tm_min, t->tm_sec);
    
    // AM/PM
    display.setTextSize(1); 
    display.setCursor(110,32); 
    display.print(pm ? "PM" : "AM");
    
    // Date (CENTERED)
    // Shifted from 10 to 30 to center it
    display.setCursor(30,44); 
    if(t->tm_year < 100) {
        display.print(F("??-????-????")); 
    } else {
        display.printf("%02d %s %d", t->tm_mday, MONTHS[t->tm_mon], t->tm_year+1900); 
    }
    
    // Bottom Bar (Weather)
    display.setCursor(5,56); display.print(currentCond); 
    display.setCursor(92,56); display.print(currentTemp); display.print(F("C"));
    
    display.display();
}

void renderMenu() {
    display.clearDisplay(); 
    display.setTextSize(1); 
    
    // Header
    display.setCursor(35,0); 
    display.print(F("- MENU -")); 
    display.drawLine(0,10,128,10,WHITE);
    
    // List Items (Shows 4 at a time)
    for(int i = 0; i < 4; i++) { 
        int itemIndex = menuTopLine + i; 
        if(itemIndex >= MENU_COUNT) break; 
        
        display.setCursor(10, 20 + (i * 11)); 
        display.print(itemIndex == menuIndex ? F("> ") : F("  ")); 
        display.print(MENU_ITEMS[itemIndex]); 
    }
    display.display();
}

void renderStopwatch() {
    unsigned long currentDelta = (stopwatchRunning) ? (millis() - stopwatchStart) : 0;
    unsigned long totalTime = stopwatchElapsed + currentDelta;
    
    unsigned long mins = (totalTime / 60000);
    unsigned long secs = (totalTime % 60000) / 1000;
    unsigned long ms   = (totalTime % 1000) / 10; 

    display.clearDisplay(); 
    
    // Timer Display
    display.setTextSize(2); 
    display.setCursor(15, 25); 
    display.printf("%02d:%02d:%02d", mins, secs, ms);
    
    // Status
    display.setTextSize(1); 
    display.setCursor(40, 55); 
    display.print(stopwatchRunning ? F("RUNNING") : F("STOPPED")); 
    
    display.display();
}

void renderDecision() {
    display.clearDisplay(); 
    display.setTextSize(1); 
    
    display.setCursor(30, 0);  display.print(F("ASK A Q &")); 
    display.setCursor(40, 10); display.print(F("CLICK!")); 
    display.drawLine(0,20,128,20,WHITE);

    if (currentAnswerIndex == -1) {
        display.setTextSize(2); 
        display.setCursor(55, 35); 
        display.print(F("?"));
    } else {
        display.setTextSize(2); 
        int len = strlen(ANSWERS[currentAnswerIndex]);
        int x = (128 - (len * 12)) / 2; 
        if (x < 0) x = 0;
        
        display.setCursor(x, 35); 
        display.print(ANSWERS[currentAnswerIndex]);
    }
    display.display();
}

void renderOTAScreen() {
    display.clearDisplay(); 
    display.setTextSize(1); 
    
    display.setCursor(30,5); 
    display.print(F("OTA MODE")); 
    display.drawLine(0,15,128,15,WHITE);
    
    display.setCursor(0,25); 
    display.print(F("Net: ")); 
    display.print(WiFi.SSID());
    
    display.setCursor(0,45); 
    display.print(F("IP: ")); 
    display.print(WiFi.localIP()); 
    
    display.display(); 
}

void renderAbout() {
    display.clearDisplay();
    display.setTextSize(1);
    
    display.setCursor(45, 0); 
    display.print(F("ABOUT"));
    display.drawLine(0, 10, 128, 10, WHITE);
    
    // Using Hex Encoded Strings for Security
    display.setCursor(10, 20); 
    display.print(_SEC_PROJ);    // "SK Project"
    
    display.setCursor(10, 35); 
    display.print(F("Version: ")); 
    display.print(_SEC_VER);     // "v1.0"
    
    display.setCursor(10, 50); 
    display.print(F("Dev: ")); 
    display.print(_SEC_DEV);     // "Sudhir"
    
    display.display();
}

// =================================================================================
// [9] BUTTON LOGIC
// =================================================================================

void onClick() {
    lastInteractionTime = millis();
    if (currentMode == MODE_SLEEPING) { wakeUpSystem(); return; }

    switch (currentMode) {
        case MODE_CLOCK: 
            currentMode = MODE_MENU; 
            break;
        case MODE_MENU: 
            menuIndex++; 
            if (menuIndex >= MENU_COUNT) { 
                menuIndex = 0; 
                menuTopLine = 0; 
            } else if (menuIndex >= menuTopLine + 4) {
                menuTopLine++; 
            }
            break;
        case MODE_STOPWATCH: 
            if(stopwatchRunning) { 
                stopwatchElapsed += millis() - stopwatchStart; 
                stopwatchRunning = false; 
            } else { 
                stopwatchStart = millis(); 
                stopwatchRunning = true; 
            } 
            break;
        case MODE_DECISION:
            currentAnswerIndex = random(0, 6); 
            break;
        case MODE_ABOUT: 
        case MODE_OTA: 
            stopWiFiNetwork(); 
            currentMode = MODE_MENU; 
            break;
        default: break;
    }
}

void onLongPress() {
    lastInteractionTime = millis();
    if (currentMode == MODE_SLEEPING) { wakeUpSystem(); return; }

    switch (currentMode) {
        case MODE_CLOCK: 
            connectWiFiSync(); 
            break;
        case MODE_MENU: 
            // Menu Selection Logic
            if(menuIndex == 0) { handleOTA(); currentMode = MODE_OTA; }      
            else if(menuIndex == 1) currentMode = MODE_STOPWATCH;               
            else if(menuIndex == 2) { currentAnswerIndex = -1; currentMode = MODE_DECISION; } 
            else if(menuIndex == 3) currentMode = MODE_ABOUT;                   
            break;
        case MODE_STOPWATCH: 
            stopwatchRunning = false; 
            stopwatchElapsed = 0; 
            break;
        case MODE_DECISION: 
        case MODE_ABOUT: 
        case MODE_OTA: 
            stopWiFiNetwork(); 
            currentMode = MODE_MENU; 
            break;
        default: break;
    }
}

void onDoubleClick() {
    lastInteractionTime = millis();
    if (currentMode == MODE_SLEEPING) { wakeUpSystem(); return; }
    
    if (currentMode == MODE_STOPWATCH) stopwatchRunning = false;
    if (currentMode == MODE_OTA) stopWiFiNetwork(); 

    if (currentMode == MODE_MENU) {
        currentMode = MODE_CLOCK; 
    } else if (currentMode != MODE_CLOCK) {
        currentMode = MODE_MENU;
    }
}

// =================================================================================
// [10] API HELPERS
// =================================================================================

void fetchWeatherData() {
    WiFiClient client; 
    HTTPClient http; 
    http.setTimeout(5000); 
    
    String url = "http://api.open-meteo.com/v1/forecast?latitude=" + GEO_LAT + "&longitude=" + GEO_LON + "&current_weather=true";
    http.begin(client, url);
    
    if(http.GET() > 0) {
        String payload = http.getString();
        int idx = payload.indexOf("\"current_weather\":");
        if(idx != -1) { 
            String wData = payload.substring(idx); 
            String t = extractJsonValue(wData, "temperature"); 
            String c = extractJsonValue(wData, "weathercode");
            
            if(t != "" && c != "") { 
                currentTemp = t; 
                currentCond = parseWeatherCode(c.toInt()); 
            }
        }
    } 
    http.end();
}

String extractJsonValue(String json, String key) {
    String search = "\"" + key + "\":"; 
    int start = json.indexOf(search); 
    if (start == -1) return ""; 
    
    start += search.length(); 
    int end = json.indexOf(",", start); 
    if (end == -1) end = json.indexOf("}", start);
    
    if (end != -1) { 
        String val = json.substring(start, end); 
        val.trim(); 
        return val; 
    } 
    return "";
}

String parseWeatherCode(int code) { 
    if(code == 0) return "CLEAR"; 
    if(code <= 3) return "CLOUDY"; 
    if(code <= 48) return "FOGGY"; 
    if(code <= 65) return "RAIN"; 
    if(code >= 95) return "STORM"; 
    return "STABLE"; 
}