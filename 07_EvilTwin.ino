// ════════════════════════════════════════════════════════════════
// 07 — Evil Twin Detector
// Scans WiFi every 10s. If same SSID appears with 2+ different BSSIDs
// (different MACs) — one is likely an impostor / Evil Twin. ALERT.
// ════════════════════════════════════════════════════════════════
#include <WiFi.h>
#include <TFT_eSPI.h>
#include "common.h"

TFT_eSPI tft = TFT_eSPI(128, 128);

struct SsidGroup {
  String ssid;
  int count;
  String macs[4];   // up to 4 BSSIDs per SSID
};
SsidGroup groups[20];
int groupCount = 0;
int suspiciousCount = 0;
uint32_t lastScanTs = 0;

void analyze(){
  groupCount = 0; suspiciousCount = 0;
  int n = WiFi.scanNetworks(false, false);
  for(int i=0; i<n; i++){
    String s = WiFi.SSID(i);
    String m = WiFi.BSSIDstr(i);
    if(s.length() == 0) continue;
    int gi = -1;
    for(int g=0; g<groupCount; g++) if(groups[g].ssid == s){ gi = g; break; }
    if(gi < 0){
      if(groupCount >= 20) continue;
      gi = groupCount++;
      groups[gi].ssid = s; groups[gi].count = 0;
    }
    if(groups[gi].count < 4){
      bool dup = false;
      for(int k=0; k<groups[gi].count; k++) if(groups[gi].macs[k] == m){ dup=true; break; }
      if(!dup){
        groups[gi].macs[groups[gi].count++] = m;
      }
    }
  }
  WiFi.scanDelete();
  for(int g=0; g<groupCount; g++) if(groups[g].count >= 2) suspiciousCount++;
}

void renderClean(){
  tft.fillScreen(C_BG);
  drawTitle(tft, "EVIL TWIN");
  tft.setTextColor(C_GREEN, C_BG); tft.setTextSize(2);
  tft.setCursor(16, 40); tft.print("CLEAN");
  tft.setTextSize(1); tft.setTextColor(C_GOLDD, C_BG);
  tft.setCursor(14, 75); tft.printf("%d unique SSIDs", groupCount);
  drawFooter(tft, "10s auto-rescan");
}

void renderSuspicious(){
  tft.fillScreen(C_RED);
  tft.setTextColor(C_WHT, C_RED); tft.setTextSize(2);
  tft.setCursor(2, 5); tft.print("EVIL TWIN");
  tft.setTextSize(1); tft.setCursor(12, 25); tft.printf("%d SUSPECT", suspiciousCount);
  int line = 40;
  for(int g=0; g<groupCount && line < 115; g++){
    if(groups[g].count < 2) continue;
    tft.setTextColor(C_YLW, C_RED);
    String s = groups[g].ssid; if(s.length()>16) s = s.substring(0,16);
    tft.setCursor(2, line); tft.printf("%s x%d", s.c_str(), groups[g].count);
    line += 10;
  }
  ledOn();
  beepAlert();
  delay(2000);
  ledOff();
}

void setup(){
  Serial.begin(115200);
  tftInit(tft); buzzInit(); ledInit(); buttonsInit();
  WiFi.mode(WIFI_STA); WiFi.disconnect(true); delay(100);
  tft.fillScreen(C_BG); drawTitle(tft, "EVIL TWIN");
  tft.setTextColor(C_GOLD, C_BG); tft.setCursor(10, 50); tft.print("Initializing...");
  analyze();
  if(suspiciousCount > 0) renderSuspicious(); else renderClean();
  lastScanTs = millis();
}

bool lSel=false;
void loop(){
  if(buttonPressed(BTN_SEL, lSel) || millis() - lastScanTs > 10000){
    lastScanTs = millis();
    analyze();
    if(suspiciousCount > 0) renderSuspicious(); else renderClean();
  }
  delay(100);
}
