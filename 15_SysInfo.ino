// ════════════════════════════════════════════════════════════════
// 15 — System Info
// Displays ESP32 chip info, heap, uptime, MAC, temp, voltage est.
// Useful for debugging / health check. Auto-refreshes every 2s.
// ════════════════════════════════════════════════════════════════
#include <WiFi.h>
#include <esp_system.h>
#include <esp_chip_info.h>
#include <TFT_eSPI.h>
#include "common.h"

TFT_eSPI tft = TFT_eSPI(128, 128);

String getMac(){
  uint8_t m[6]; esp_read_mac(m, ESP_MAC_WIFI_STA);
  return macFmt(m);
}

void render(){
  tft.fillRect(0, 14, 128, 104, C_BG);
  esp_chip_info_t info; esp_chip_info(&info);
  uint32_t freeHeap = ESP.getFreeHeap();
  uint32_t totalHeap = ESP.getHeapSize();
  uint32_t flash = ESP.getFlashChipSize();
  uint32_t up = millis()/1000;
  int upH = up/3600, upM = (up/60)%60, upS = up%60;
  int heapPct = (freeHeap * 100) / totalHeap;

  tft.setTextColor(C_GOLD, C_BG); tft.setTextSize(1);
  tft.setCursor(2, 16); tft.print("MAC:");
  tft.setTextColor(C_CYAN, C_BG);
  tft.setCursor(2, 26); tft.print(getMac());

  tft.setTextColor(C_GOLD, C_BG); tft.setCursor(2, 40); tft.print("Chip:");
  tft.setTextColor(C_CYAN, C_BG);
  tft.setCursor(34, 40);
  const char* m = (info.model==CHIP_ESP32)?"ESP32":(info.model==CHIP_ESP32S2)?"S2":(info.model==CHIP_ESP32S3)?"S3":(info.model==CHIP_ESP32C3)?"C3":"???";
  tft.printf("%s r%d", m, info.revision);

  tft.setTextColor(C_GOLD, C_BG); tft.setCursor(2, 52); tft.print("Cores:");
  tft.setTextColor(C_CYAN, C_BG); tft.setCursor(42, 52); tft.printf("%d", info.cores);
  tft.setCursor(60, 52); tft.printf("CPU:%dMHz", ESP.getCpuFreqMHz());

  tft.setTextColor(C_GOLD, C_BG); tft.setCursor(2, 64); tft.print("Flash:");
  tft.setTextColor(C_CYAN, C_BG); tft.setCursor(38, 64); tft.printf("%dMB", flash/1024/1024);

  tft.setTextColor(C_GOLD, C_BG); tft.setCursor(2, 76); tft.print("Heap:");
  tft.setTextColor(heapPct > 30 ? C_GREEN : heapPct > 15 ? C_YLW : C_RED, C_BG);
  tft.setCursor(32, 76); tft.printf("%lu B (%d%%)", freeHeap, heapPct);

  tft.setTextColor(C_GOLD, C_BG); tft.setCursor(2, 88); tft.print("SDK:");
  tft.setTextColor(C_CYAN, C_BG); tft.setCursor(28, 88);
  String sdk = ESP.getSdkVersion(); if(sdk.length()>16) sdk = sdk.substring(0,16);
  tft.print(sdk);

  tft.setTextColor(C_GOLD, C_BG); tft.setCursor(2, 100); tft.print("Up:");
  tft.setTextColor(C_CYAN, C_BG); tft.setCursor(24, 100);
  tft.printf("%02d:%02d:%02d", upH, upM, upS);

  drawFooter(tft, "SEL:reboot");
}

void setup(){
  Serial.begin(115200);
  tftInit(tft); buzzInit(); ledInit(); buttonsInit();
  drawTitle(tft, "SYS INFO");
  render();
  beepOK();
}

bool lSel=false;
uint32_t lastRender = 0;
void loop(){
  if(millis() - lastRender > 2000){ lastRender = millis(); render(); }
  if(buttonPressed(BTN_SEL, lSel)){
    tft.fillScreen(C_RED);
    tft.setTextColor(C_WHT, C_RED); tft.setTextSize(2);
    tft.setCursor(10, 50); tft.print("REBOOT!");
    buzz(400, 200);
    delay(600);
    ESP.restart();
  }
  delay(50);
}
