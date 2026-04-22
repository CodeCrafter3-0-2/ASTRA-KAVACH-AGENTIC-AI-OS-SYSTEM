// ════════════════════════════════════════════════════════════════
// 08 — Probe Request Logger
// Listens for 802.11 probe requests (devices looking for WiFi nets)
// Shows SSIDs being probed and MAC of requesting device
// ════════════════════════════════════════════════════════════════
#include <WiFi.h>
#include <esp_wifi.h>
#include <TFT_eSPI.h>
#include "common.h"

TFT_eSPI tft = TFT_eSPI(128, 128);

struct Probe {
  String ssid;
  uint8_t mac[6];
  uint32_t ts;
  int count;
};
Probe probes[20];
int probeCount = 0;
uint32_t totalProbes = 0;
uint8_t currCh = 1;
int cursor = 0, topRow = 0;
SemaphoreHandle_t mtx;

static void IRAM_ATTR sniffCB(void* buf, wifi_promiscuous_pkt_type_t type){
  if(type != WIFI_PKT_MGMT) return;
  auto* pkt = (wifi_promiscuous_pkt_t*)buf;
  uint16_t len = pkt->rx_ctrl.sig_len;
  if(len < 26) return;
  if(pkt->payload[0] != 0x40) return;   // probe request
  totalProbes++;
  // SSID tag at offset 24
  if(pkt->payload[24] != 0x00) return;
  uint8_t sl = pkt->payload[25];
  if(sl == 0 || sl > 32 || 26+sl > len) return;
  char ssid[33] = {0};
  memcpy(ssid, pkt->payload + 26, sl);
  if(xSemaphoreTakeFromISR(mtx, NULL) != pdTRUE) return;
  // dedupe
  for(int i=0; i<probeCount; i++){
    if(strcmp(probes[i].ssid.c_str(), ssid) == 0){
      probes[i].ts = millis();
      probes[i].count++;
      xSemaphoreGiveFromISR(mtx, NULL);
      return;
    }
  }
  if(probeCount < 20){
    probes[probeCount].ssid = String(ssid);
    memcpy(probes[probeCount].mac, pkt->payload + 10, 6);
    probes[probeCount].ts = millis();
    probes[probeCount].count = 1;
    probeCount++;
  }
  xSemaphoreGiveFromISR(mtx, NULL);
}

void render(){
  tft.fillRect(0, 14, 128, 104, C_BG);
  if(probeCount == 0){
    tft.setTextColor(C_GOLDD, C_BG); tft.setCursor(10, 60); tft.print("Listening...");
  } else {
    for(int i=0; i<8 && (topRow+i) < probeCount; i++){
      int idx = topRow + i; int y = 16 + i*12;
      if(idx == cursor) tft.fillRect(0, y-1, 128, 11, C_HDR);
      tft.setTextColor(C_GOLD, idx==cursor?C_HDR:C_BG);
      tft.setCursor(2, y); tft.printf("%d", probes[idx].count);
      tft.setTextColor(C_CYAN, idx==cursor?C_HDR:C_BG);
      tft.setCursor(22, y);
      String s = probes[idx].ssid; if(s.length()>16) s = s.substring(0,16);
      tft.print(s);
    }
  }
  char ftr[24]; snprintf(ftr, 24, "Tot:%lu c:%d", totalProbes, currCh);
  drawFooter(tft, ftr);
}

void setup(){
  Serial.begin(115200);
  tftInit(tft); buzzInit(); ledInit(); buttonsInit();
  drawTitle(tft, "PROBE LOGGER");
  mtx = xSemaphoreCreateMutex();
  WiFi.mode(WIFI_STA); WiFi.disconnect(true); delay(100);
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_promiscuous_rx_cb(sniffCB);
  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);
  render();
  beepOK();
}

uint32_t lastHop=0, lastRender=0;
bool lUp=false, lDown=false, lSel=false;
void loop(){
  uint32_t now = millis();
  if(now - lastHop > 500){
    lastHop = now;
    currCh = (currCh % 13) + 1;
    esp_wifi_set_channel(currCh, WIFI_SECOND_CHAN_NONE);
  }
  if(now - lastRender > 800){
    lastRender = now;
    drawTitle(tft, "PROBE LOGGER");
    render();
  }
  if(buttonPressed(BTN_UP, lUp))   if(cursor>0){ cursor--; if(cursor<topRow) topRow=cursor; render(); }
  if(buttonPressed(BTN_DOWN, lDown)) if(cursor<probeCount-1){ cursor++; if(cursor>=topRow+8) topRow=cursor-7; render(); }
  if(buttonPressed(BTN_SEL, lSel)){
    probeCount = 0; totalProbes = 0; cursor = 0; topRow = 0;
    render(); beepOK();
  }
  delay(30);
}
