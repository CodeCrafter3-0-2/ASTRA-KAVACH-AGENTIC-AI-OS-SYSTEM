// ════════════════════════════════════════════════════════════════
// common.h — Shared definitions for Astra Kavach Toolkit
// Used by all 15 sketches. Copy this file into each sketch folder.
// ════════════════════════════════════════════════════════════════
#ifndef ASTRA_COMMON_H
#define ASTRA_COMMON_H

#include <TFT_eSPI.h>

// ── HARDWARE PINS (v4 wiring) ───────────────────────────
// TFT (via User_Setup.h): MOSI=23 SCK=18 CS=27 DC=26 RST=5 BL=32
#define PIN_BUZZER     4
#define PIN_LED       25
#define PIN_BL        32
#define PIN_RST        5
#define SD_CS         15
#define BTN_UP        33
#define BTN_DOWN      22
#define BTN_LEFT      14
#define BTN_RIGHT     13
#define BTN_SEL       17

// Optional GPS (NEO-6M) — uncomment in sketches that use it
// GPS TX → ESP32 GPIO 16  (ESP32 RX)
// GPS RX → ESP32 GPIO 21  (ESP32 TX)  -- only if configuring GPS
// GPS VCC → 3.3V, GND → GND
#define GPS_RX_PIN 16
#define GPS_TX_PIN 21

// ── COLORS (RGB565) ─────────────────────────────────────
#define C_BG       0x0000
#define C_HDR      0x0104
#define C_PANEL    0x0861
#define C_CYAN     0x07FF
#define C_CYAND    0x03DF
#define C_GOLD     0xFEA0
#define C_GOLDD    0xB4C0
#define C_GREEN    0x07E0
#define C_RED      0xF800
#define C_YLW      0xFFE0
#define C_ORG      0xFD20
#define C_GRAY     0x4208
#define C_WHT      0xFFFF
#define C_MAG      0xF81F

// ── LEDC PWM channels ───────────────────────────────────
#define BUZZ_CH   0
#define LED_CH    1
#define LED_DUTY  200
#define LED_FREQ  1000

// ── BUZZER (LEDC hardware PWM) ──────────────────────────
inline void buzzInit(){
#if defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 3)
  ledcAttach(PIN_BUZZER, 2000, 10);
  ledcWriteTone(PIN_BUZZER, 0);
#else
  ledcSetup(BUZZ_CH, 2000, 10);
  ledcAttachPin(PIN_BUZZER, BUZZ_CH);
  ledcWriteTone(BUZZ_CH, 0);
#endif
}
inline void buzz(int freq, int ms){
  if(freq<=0 || ms<=0) return;
#if defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 3)
  ledcWriteTone(PIN_BUZZER, freq); delay(ms); ledcWriteTone(PIN_BUZZER, 0);
#else
  ledcWriteTone(BUZZ_CH, freq); delay(ms); ledcWriteTone(BUZZ_CH, 0);
#endif
}
inline void beepOK(){ buzz(1200, 60); }
inline void beepErr(){ buzz(400, 200); }
inline void beepAlert(){
  for(int i=0;i<3;i++){ buzz(2200,80); delay(50); }
}

// ── LED (LEDC PWM, no resistor needed) ──────────────────
inline void ledInit(){
#if defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 3)
  ledcAttach(PIN_LED, LED_FREQ, 10); ledcWrite(PIN_LED, 0);
#else
  ledcSetup(LED_CH, LED_FREQ, 10);
  ledcAttachPin(PIN_LED, LED_CH);
  ledcWrite(LED_CH, 0);
#endif
}
inline void ledOn(){
#if defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 3)
  ledcWrite(PIN_LED, LED_DUTY);
#else
  ledcWrite(LED_CH, LED_DUTY);
#endif
}
inline void ledOff(){
#if defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 3)
  ledcWrite(PIN_LED, 0);
#else
  ledcWrite(LED_CH, 0);
#endif
}

// ── BUTTONS (INPUT_PULLUP, press = LOW) ─────────────────
struct Btns { bool up, down, left, right, sel; };
inline void buttonsInit(){
  pinMode(BTN_UP,    INPUT_PULLUP);
  pinMode(BTN_DOWN,  INPUT_PULLUP);
  pinMode(BTN_LEFT,  INPUT_PULLUP);
  pinMode(BTN_RIGHT, INPUT_PULLUP);
  pinMode(BTN_SEL,   INPUT_PULLUP);
}
inline Btns buttonsRead(){
  return {
    digitalRead(BTN_UP)   == LOW,
    digitalRead(BTN_DOWN) == LOW,
    digitalRead(BTN_LEFT) == LOW,
    digitalRead(BTN_RIGHT)== LOW,
    digitalRead(BTN_SEL)  == LOW
  };
}
// edge-detect helper: returns true once per press
inline bool buttonPressed(uint8_t pin, bool& lastState){
  bool now = digitalRead(pin) == LOW;
  if(now && !lastState){ lastState = now; delay(30); return true; }
  lastState = now;
  return false;
}

// ── TFT helpers ─────────────────────────────────────────
inline void tftInit(TFT_eSPI& tft){
  pinMode(PIN_BL, OUTPUT);  digitalWrite(PIN_BL, HIGH);
  pinMode(PIN_RST, OUTPUT);
  digitalWrite(PIN_RST, LOW);  delay(100);
  digitalWrite(PIN_RST, HIGH); delay(100);
  tft.init();
  tft.setRotation(0);
  tft.fillScreen(C_BG);
}
inline void drawTitle(TFT_eSPI& tft, const char* title){
  tft.fillRect(0, 0, 128, 14, C_HDR);
  tft.drawLine(0, 13, 128, 13, C_CYAN);
  tft.setTextColor(C_CYAN, C_HDR);
  tft.setTextSize(1);
  tft.setCursor(3, 3);
  tft.print(title);
}
inline void drawFooter(TFT_eSPI& tft, const char* txt){
  tft.fillRect(0, 118, 128, 10, C_HDR);
  tft.drawLine(0, 117, 128, 117, C_CYAND);
  tft.setTextColor(C_GOLDD, C_HDR);
  tft.setTextSize(1);
  tft.setCursor(3, 119);
  tft.print(txt);
}
inline String macFmt(const uint8_t* mac){
  char buf[18];
  snprintf(buf, 18, "%02X:%02X:%02X:%02X:%02X:%02X",
           mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
  return String(buf);
}
inline uint16_t rssiColor(int rssi){
  if(rssi > -55) return C_GREEN;
  if(rssi > -75) return C_YLW;
  return C_RED;
}

#endif
