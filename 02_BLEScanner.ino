// ════════════════════════════════════════════════════════════════
// 02 — BLE Scanner
// Lists all BLE devices nearby (name, MAC, RSSI). SEL: rescan
// ════════════════════════════════════════════════════════════════
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <TFT_eSPI.h>
#include "common.h"

TFT_eSPI tft = TFT_eSPI(128, 128);
BLEScan* bleScan;

struct Dev {
  String name;
  String mac;
  int rssi;
};
Dev devs[30];
int devCount = 0;
int cursor = 0, topRow = 0;

class ScanCB : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice d) override {
    if(devCount >= 30) return;
    String mac = d.getAddress().toString().c_str();
    // dedupe
    for(int i=0; i<devCount; i++) if(devs[i].mac == mac){ devs[i].rssi = d.getRSSI(); return; }
    devs[devCount].mac = mac;
    devs[devCount].rssi = d.getRSSI();
    devs[devCount].name = d.haveName() ? d.getName().c_str() : "(no name)";
    devCount++;
  }
};
ScanCB cb;

void doScan(){
  tft.fillScreen(C_BG);
  drawTitle(tft, "BLE SCANNER");
  tft.setTextColor(C_GOLD, C_BG); tft.setCursor(10, 50); tft.print("Scanning 5s...");
  buzz(1000, 40);
  devCount = 0;
  bleScan->start(5, false);
  bleScan->clearResults();
  cursor = 0; topRow = 0;
  beepOK();
}

void render(){
  tft.fillRect(0, 14, 128, 104, C_BG);
  if(devCount == 0){
    tft.setTextColor(C_RED, C_BG); tft.setCursor(10, 60); tft.print("No BLE devs");
    return;
  }
  for(int i=0; i<8 && (topRow+i) < devCount; i++){
    int idx = topRow + i;
    int y = 16 + i*12;
    if(idx == cursor) tft.fillRect(0, y-1, 128, 11, C_HDR);
    tft.setTextColor(rssiColor(devs[idx].rssi), idx==cursor?C_HDR:C_BG);
    tft.setCursor(2, y); tft.printf("%d", devs[idx].rssi);
    tft.setTextColor(C_CYAN, idx==cursor?C_HDR:C_BG);
    tft.setCursor(28, y);
    String n = devs[idx].name; if(n.length()>16) n = n.substring(0,16);
    tft.print(n);
  }
  char ftr[20]; snprintf(ftr, 20, "%d devs  SEL:rescan", devCount);
  drawFooter(tft, ftr);
}

void setup(){
  Serial.begin(115200);
  tftInit(tft); buzzInit(); ledInit(); buttonsInit();
  BLEDevice::init("AstraScan");
  bleScan = BLEDevice::getScan();
  bleScan->setAdvertisedDeviceCallbacks(&cb);
  bleScan->setActiveScan(true);
  bleScan->setInterval(100);
  bleScan->setWindow(99);
  doScan(); render();
}

bool lUp=false, lDown=false, lSel=false;
void loop(){
  if(buttonPressed(BTN_UP, lUp)){
    if(cursor > 0){ cursor--; if(cursor < topRow) topRow = cursor; render(); }
  }
  if(buttonPressed(BTN_DOWN, lDown)){
    if(cursor < devCount-1){ cursor++; if(cursor >= topRow+8) topRow = cursor-7; render(); }
  }
  if(buttonPressed(BTN_SEL, lSel)){ doScan(); render(); }
  delay(20);
}
