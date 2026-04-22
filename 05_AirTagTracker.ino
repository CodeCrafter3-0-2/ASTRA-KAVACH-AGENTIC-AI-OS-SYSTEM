// ════════════════════════════════════════════════════════════════
// 05 — AirTag Tracker (privacy tool)
// Detects Apple Find My beacons (AirTag signature in mfg data)
// Mfg data: 0x4C (Apple) 0x00 0x12 — Find My protocol
// ════════════════════════════════════════════════════════════════
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <TFT_eSPI.h>
#include "common.h"

TFT_eSPI tft = TFT_eSPI(128, 128);
BLEScan* bleScan;

struct Tag {
  String mac;
  int rssi;
  uint32_t lastSeen;
};
Tag tags[10];
int tagCount = 0;

class TagCB : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice d) override {
    if(!d.haveManufacturerData()) return;
    auto m = d.getManufacturerData();
    if(m.size() < 3) return;
    uint8_t* p = (uint8_t*)m.data();
    // Apple Find My: 0x4C 0x00 0x12
    if(p[0] == 0x4C && p[1] == 0x00 && p[2] == 0x12){
      String mac = d.getAddress().toString().c_str();
      uint32_t now = millis();
      // dedupe
      for(int i=0; i<tagCount; i++){
        if(tags[i].mac == mac){
          tags[i].rssi = d.getRSSI();
          tags[i].lastSeen = now;
          return;
        }
      }
      if(tagCount < 10){
        tags[tagCount].mac = mac;
        tags[tagCount].rssi = d.getRSSI();
        tags[tagCount].lastSeen = now;
        tagCount++;
      }
    }
  }
};
TagCB cb;

void render(){
  tft.fillScreen(C_BG);
  drawTitle(tft, "AIRTAG TRACKER");
  if(tagCount == 0){
    tft.setTextColor(C_GREEN, C_BG); tft.setTextSize(2);
    tft.setCursor(22, 50); tft.print("CLEAR");
    tft.setTextSize(1); tft.setTextColor(C_GOLDD, C_BG);
    tft.setCursor(12, 75); tft.print("No AirTags near");
  } else {
    tft.setTextColor(C_RED, C_BG); tft.setTextSize(2);
    tft.setCursor(2, 18); tft.printf("%d FOUND", tagCount);
    tft.setTextSize(1);
    for(int i=0; i<tagCount && i<5; i++){
      int y = 50 + i*13;
      tft.setTextColor(rssiColor(tags[i].rssi), C_BG);
      tft.setCursor(2, y); tft.printf("%d", tags[i].rssi);
      tft.setTextColor(C_CYAN, C_BG);
      tft.setCursor(28, y);
      String mac = tags[i].mac; if(mac.length()>14) mac = mac.substring(0,14);
      tft.print(mac);
    }
  }
  drawFooter(tft, "Scanning...");
}

void setup(){
  Serial.begin(115200);
  tftInit(tft); buzzInit(); ledInit(); buttonsInit();
  BLEDevice::init("AstraAirTag");
  bleScan = BLEDevice::getScan();
  bleScan->setAdvertisedDeviceCallbacks(&cb);
  bleScan->setActiveScan(true);
  bleScan->setInterval(100);
  bleScan->setWindow(99);
  render();
  buzz(1500, 60);
}

uint32_t lastScan=0, lastRender=0, lastCleanup=0;
int lastDisplayedCount = 0;
void loop(){
  uint32_t now = millis();
  if(now - lastScan > 2000){
    lastScan = now;
    bleScan->start(2, false);
    bleScan->clearResults();
  }
  // cleanup tags not seen in 30s
  if(now - lastCleanup > 5000){
    lastCleanup = now;
    for(int i=0; i<tagCount; ){
      if(now - tags[i].lastSeen > 30000){
        for(int j=i; j<tagCount-1; j++) tags[j] = tags[j+1];
        tagCount--;
      } else i++;
    }
  }
  if(now - lastRender > 1000){
    lastRender = now;
    if(tagCount != lastDisplayedCount){
      if(tagCount > lastDisplayedCount){
        // new tag — beep
        beepAlert();
        ledOn(); delay(200); ledOff();
      }
      lastDisplayedCount = tagCount;
    }
    render();
  }
  delay(50);
}
