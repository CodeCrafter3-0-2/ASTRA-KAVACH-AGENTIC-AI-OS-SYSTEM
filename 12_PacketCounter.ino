// ════════════════════════════════════════════════════════════════
// 12 — Packet-per-Second Counter
// Shows live PPS on current channel. UP/DOWN: change channel manual.
// LEFT/RIGHT: toggle auto-hop. Useful to gauge RF activity / sniffer health.
// ════════════════════════════════════════════════════════════════
#include <WiFi.h>
#include <esp_wifi.h>
#include <TFT_eSPI.h>
#include "common.h"

TFT_eSPI tft = TFT_eSPI(128, 128);

volatile uint32_t pktCnt = 0;
volatile uint32_t totalPkts = 0;
uint32_t pps = 0;
uint8_t currCh = 1;
bool autoHop = false;

static void IRAM_ATTR sniffCB(void* buf, wifi_promiscuous_pkt_type_t type){
  pktCnt++; totalPkts++;
}

void render(){
  tft.fillRect(0, 14, 128, 104, C_BG);
  // big PPS
  tft.setTextSize(4);
  uint16_t col = pps > 50 ? C_GREEN : pps > 10 ? C_YLW : pps > 0 ? C_ORG : C_RED;
  tft.setTextColor(col, C_BG);
  char buf[8]; snprintf(buf, 8, "%lu", pps);
  int bw = strlen(buf) * 24;
  tft.setCursor((128-bw)/2, 25);
  tft.print(buf);
  tft.setTextSize(1); tft.setTextColor(C_GOLDD, C_BG);
  tft.setCursor(50, 65); tft.print("pkt/s");

  tft.setTextColor(C_CYAN, C_BG);
  tft.setCursor(2, 82); tft.printf("Ch: %d", currCh);
  tft.setCursor(55, 82); tft.print(autoHop ? "[AUTO]" : "[LOCK]");
  tft.setTextColor(C_GOLDD, C_BG);
  tft.setCursor(2, 96); tft.printf("Total: %lu", totalPkts);

  drawFooter(tft, "UP/DN:ch LR:auto");
}

void setup(){
  Serial.begin(115200);
  tftInit(tft); buzzInit(); ledInit(); buttonsInit();
  drawTitle(tft, "PPS COUNTER");
  WiFi.mode(WIFI_STA); WiFi.disconnect(true); delay(100);
  esp_wifi_set_promiscuous(true);
  wifi_promiscuous_filter_t f = { .filter_mask = WIFI_PROMIS_FILTER_MASK_ALL };
  esp_wifi_set_promiscuous_filter(&f);
  esp_wifi_set_promiscuous_rx_cb(sniffCB);
  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);
  render();
  beepOK();
}

bool lUp=false, lDown=false, lLeft=false, lRight=false;
uint32_t lastTick=0, lastHop=0;
void loop(){
  uint32_t now = millis();
  if(now - lastTick > 1000){
    lastTick = now;
    pps = pktCnt;
    pktCnt = 0;
    render();
  }
  if(autoHop && now - lastHop > 1000){
    lastHop = now;
    currCh = (currCh % 13) + 1;
    esp_wifi_set_channel(currCh, WIFI_SECOND_CHAN_NONE);
  }
  if(buttonPressed(BTN_UP, lUp))     { if(currCh<13){ currCh++; esp_wifi_set_channel(currCh,WIFI_SECOND_CHAN_NONE); render(); } }
  if(buttonPressed(BTN_DOWN, lDown)) { if(currCh>1){ currCh--; esp_wifi_set_channel(currCh,WIFI_SECOND_CHAN_NONE); render(); } }
  if(buttonPressed(BTN_LEFT, lLeft) || buttonPressed(BTN_RIGHT, lRight)) {
    autoHop = !autoHop; render(); beepOK();
  }
  delay(20);
}
