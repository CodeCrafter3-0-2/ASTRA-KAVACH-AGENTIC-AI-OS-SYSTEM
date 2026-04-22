// ════════════════════════════════════════════════════════════════
// 04 — Flipper Zero Hunter
// Ultra-sensitive dedicated detection of Flipper Zero via BLE
// Matches: Flipper OUI (80:E1:26), name "Flipper*", manufacturer bytes
// ════════════════════════════════════════════════════════════════
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <TFT_eSPI.h>
#include "common.h"

TFT_eSPI tft = TFT_eSPI(128, 128);
BLEScan* bleScan;

volatile bool flipperSeen = false;
String  flipperMac = "";
String  flipperName = "";
int     flipperRssi = 0;
uint32_t flipperTs = 0;
uint32_t totalHits = 0;

class HunterCB : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice d) override {
    bool isFlipper = false;
    BLEAddress a = d.getAddress();
    uint8_t* r = (uint8_t*)a.getNative();
    // Flipper Zero OUI: 80:E1:26 (reversed in native format)
    if(r[5]==0x80 && r[4]==0xE1 && r[3]==0x26) isFlipper = true;
    if(d.haveName()){
      String n = d.getName().c_str();
      if(n.indexOf("Flipper") >= 0 || n.indexOf("xRemote") >= 0) isFlipper = true;
    }
    if(isFlipper){
      flipperSeen = true;
      flipperMac  = a.toString().c_str();
      flipperName = d.haveName() ? d.getName().c_str() : "(unnamed)";
      flipperRssi = d.getRSSI();
      flipperTs   = millis();
      totalHits++;
    }
  }
};
HunterCB cb;

void renderIdle(){
  tft.fillScreen(C_BG);
  drawTitle(tft, "FLIPPER HUNTER");
  tft.setTextColor(C_CYAN, C_BG);
  tft.setTextSize(2);
  tft.setCursor(6, 40); tft.print("SEARCHING");
  tft.setTextSize(1);
  tft.setTextColor(C_GOLDD, C_BG);
  tft.setCursor(10, 70); tft.print("Scanning BLE...");
  tft.setCursor(10, 85); tft.printf("Total: %lu hits", totalHits);
  drawFooter(tft, "Active BLE scan");
}

void renderHit(){
  tft.fillScreen(C_MAG);
  tft.setTextColor(C_WHT, C_MAG); tft.setTextSize(2);
  tft.setCursor(8, 8); tft.print("FLIPPER");
  tft.setCursor(18, 28); tft.print("FOUND!");
  tft.setTextSize(1);
  tft.setCursor(2, 55); tft.print("MAC:"); tft.setCursor(2, 65); tft.print(flipperMac);
  tft.setCursor(2, 80); tft.print("Name:"); tft.setCursor(2, 90); tft.print(flipperName);
  tft.setCursor(2, 105); tft.printf("RSSI: %d dBm", flipperRssi);
  ledOn();
  for(int f=1400; f<=2800; f+=200) buzz(f, 35);
  for(int f=2800; f>=1400; f-=200) buzz(f, 35);
}

void setup(){
  Serial.begin(115200);
  tftInit(tft); buzzInit(); ledInit(); buttonsInit();
  BLEDevice::init("AstraHunt");
  bleScan = BLEDevice::getScan();
  bleScan->setAdvertisedDeviceCallbacks(&cb);
  bleScan->setActiveScan(true);
  bleScan->setInterval(100);
  bleScan->setWindow(99);
  renderIdle();
  buzz(1200, 40); buzz(1800, 80);
}

uint32_t lastScan = 0;
uint32_t lastShown = 0;
void loop(){
  if(millis() - lastScan > 1500){
    lastScan = millis();
    bleScan->start(1, false);
    bleScan->clearResults();
  }
  if(flipperSeen){
    flipperSeen = false;
    renderHit();
    lastShown = millis();
  }
  if(lastShown && millis() - lastShown > 5000){
    ledOff();
    lastShown = 0;
    renderIdle();
  }
  delay(50);
}
