// ════════════════════════════════════════════════════════════════
// 01 — WiFi Scanner
// Lists all nearby WiFi networks. Scroll with UP/DOWN. Rescan: SEL
// ════════════════════════════════════════════════════════════════
#include <WiFi.h>
#include <TFT_eSPI.h>
#include "common.h"

TFT_eSPI tft = TFT_eSPI(128, 128);

struct AP {
  String ssid;
  int32_t rssi;
  uint8_t ch;
  wifi_auth_mode_t enc;
};
AP nets[30];
int netCount = 0;
int cursor = 0;
int topRow = 0;

const char* encStr(wifi_auth_mode_t e){
  switch(e){
    case WIFI_AUTH_OPEN: return "OPEN";
    case WIFI_AUTH_WEP:  return "WEP";
    case WIFI_AUTH_WPA_PSK: return "WPA";
    case WIFI_AUTH_WPA2_PSK: return "WPA2";
    case WIFI_AUTH_WPA_WPA2_PSK: return "WPA/2";
    case WIFI_AUTH_WPA2_ENTERPRISE: return "EAP";
#if defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 2)
    case WIFI_AUTH_WPA3_PSK: return "WPA3";
    case WIFI_AUTH_WPA2_WPA3_PSK: return "WPA2/3";
#endif
    default: return "?";
  }
}

void doScan(){
  tft.fillScreen(C_BG);
  drawTitle(tft, "WIFI SCANNER");
  tft.setTextColor(C_GOLD, C_BG); tft.setCursor(10, 50); tft.print("Scanning...");
  buzz(1000, 40);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(true);
  delay(100);
  int n = WiFi.scanNetworks(false, false);
  netCount = (n > 30) ? 30 : n;
  for(int i=0; i<netCount; i++){
    nets[i].ssid = WiFi.SSID(i);
    nets[i].rssi = WiFi.RSSI(i);
    nets[i].ch   = WiFi.channel(i);
    nets[i].enc  = WiFi.encryptionType(i);
  }
  WiFi.scanDelete();
  cursor = 0; topRow = 0;
  beepOK();
}

void render(){
  tft.fillRect(0, 14, 128, 104, C_BG);
  if(netCount == 0){
    tft.setTextColor(C_RED, C_BG); tft.setCursor(10, 60); tft.print("No networks");
    return;
  }
  tft.setTextSize(1);
  for(int i=0; i<8 && (topRow+i) < netCount; i++){
    int idx = topRow + i;
    int y = 16 + i*12;
    if(idx == cursor){
      tft.fillRect(0, y-1, 128, 11, C_HDR);
    }
    tft.setTextColor(rssiColor(nets[idx].rssi), idx==cursor ? C_HDR : C_BG);
    tft.setCursor(2, y); tft.printf("%ddBm", nets[idx].rssi);
    tft.setTextColor(C_CYAN, idx==cursor ? C_HDR : C_BG);
    tft.setCursor(36, y);
    String s = nets[idx].ssid; if(s.length()>10) s = s.substring(0,10);
    tft.print(s);
    tft.setTextColor(C_GOLDD, idx==cursor ? C_HDR : C_BG);
    tft.setCursor(95, y); tft.printf("c%d", nets[idx].ch);
  }
  char ftr[20]; snprintf(ftr, 20, "%d nets  UP/DN:SCR SEL:RSCN", netCount);
  drawFooter(tft, ftr);
}

void setup(){
  Serial.begin(115200);
  tftInit(tft);
  buzzInit(); ledInit();
  buttonsInit();
  drawTitle(tft, "WIFI SCANNER");
  doScan();
  render();
}

bool lUp=false, lDown=false, lSel=false;
void loop(){
  if(buttonPressed(BTN_UP, lUp)){
    if(cursor > 0){ cursor--; if(cursor < topRow) topRow = cursor; render(); }
  }
  if(buttonPressed(BTN_DOWN, lDown)){
    if(cursor < netCount-1){ cursor++; if(cursor >= topRow+8) topRow = cursor-7; render(); }
  }
  if(buttonPressed(BTN_SEL, lSel)){
    doScan(); render();
  }
  delay(20);
}
