// ════════════════════════════════════════════════════════════════
// 06 — RSSI Meter
// Select a WiFi network from list, then see live RSSI graph
// UP/DOWN: select network   SEL: start monitor   LEFT: back to list
// ════════════════════════════════════════════════════════════════
#include <WiFi.h>
#include <TFT_eSPI.h>
#include "common.h"

TFT_eSPI tft = TFT_eSPI(128, 128);

struct AP { String ssid; int rssi; };
AP nets[20];
int netCount = 0, cursor = 0, topRow = 0;
bool monitoring = false;
String target;
int history[120];
int hIdx = 0;

void doScan(){
  tft.fillScreen(C_BG); drawTitle(tft, "RSSI METER");
  tft.setTextColor(C_GOLD, C_BG); tft.setCursor(10, 50); tft.print("Scanning...");
  WiFi.mode(WIFI_STA); WiFi.disconnect(true); delay(100);
  int n = WiFi.scanNetworks(); netCount = (n>20)?20:n;
  for(int i=0; i<netCount; i++){
    nets[i].ssid = WiFi.SSID(i);
    nets[i].rssi = WiFi.RSSI(i);
  }
  WiFi.scanDelete();
  beepOK();
}

void renderList(){
  tft.fillRect(0, 14, 128, 104, C_BG);
  for(int i=0; i<8 && (topRow+i) < netCount; i++){
    int idx = topRow + i; int y = 16 + i*12;
    if(idx == cursor) tft.fillRect(0, y-1, 128, 11, C_HDR);
    tft.setTextColor(rssiColor(nets[idx].rssi), idx==cursor?C_HDR:C_BG);
    tft.setCursor(2, y); tft.printf("%d", nets[idx].rssi);
    tft.setTextColor(C_CYAN, idx==cursor?C_HDR:C_BG);
    tft.setCursor(32, y);
    String s = nets[idx].ssid; if(s.length()>14) s = s.substring(0,14);
    tft.print(s);
  }
  drawFooter(tft, "SEL:monitor");
}

int getRssi(const String& ssid){
  int n = WiFi.scanNetworks(false, false, false, 120);
  int r = -100;
  for(int i=0; i<n; i++){
    if(WiFi.SSID(i) == ssid){ r = WiFi.RSSI(i); break; }
  }
  WiFi.scanDelete();
  return r;
}

void renderMonitor(int cur){
  tft.fillRect(0, 14, 128, 104, C_BG);
  tft.setTextColor(C_CYAN, C_BG); tft.setCursor(2, 16);
  String s = target; if(s.length()>18) s = s.substring(0,18);
  tft.print(s);
  tft.setTextSize(3); tft.setTextColor(rssiColor(cur), C_BG);
  tft.setCursor(6, 30); tft.printf("%d", cur);
  tft.setTextSize(1); tft.setTextColor(C_GOLDD, C_BG);
  tft.setCursor(85, 45); tft.print("dBm");
  // graph area y=70..115
  tft.drawLine(0, 115, 128, 115, C_CYAND);
  for(int i=0; i<120; i++){
    int v = history[i];
    if(v == 0) continue;
    // map -100..-30 → 115..70
    int y = map(v, -100, -30, 115, 70);
    y = constrain(y, 70, 115);
    tft.drawPixel(i+4, y, rssiColor(v));
  }
  drawFooter(tft, "LEFT:back");
}

void setup(){
  Serial.begin(115200);
  tftInit(tft); buzzInit(); ledInit(); buttonsInit();
  doScan(); renderList();
}

bool lUp=false, lDown=false, lSel=false, lLeft=false;
uint32_t lastSample = 0;

void loop(){
  if(!monitoring){
    if(buttonPressed(BTN_UP, lUp))   if(cursor>0){ cursor--; if(cursor<topRow) topRow=cursor; renderList(); }
    if(buttonPressed(BTN_DOWN, lDown)) if(cursor<netCount-1){ cursor++; if(cursor>=topRow+8) topRow=cursor-7; renderList(); }
    if(buttonPressed(BTN_SEL, lSel) && netCount>0){
      target = nets[cursor].ssid;
      monitoring = true;
      for(int i=0;i<120;i++) history[i]=0;
      hIdx = 0;
      tft.fillScreen(C_BG); drawTitle(tft, "RSSI MONITOR");
      beepOK();
    }
  } else {
    if(buttonPressed(BTN_LEFT, lLeft)){
      monitoring = false;
      renderList();
      return;
    }
    if(millis() - lastSample > 800){
      lastSample = millis();
      int r = getRssi(target);
      history[hIdx % 120] = r;
      hIdx++;
      renderMonitor(r);
    }
  }
  delay(20);
}
