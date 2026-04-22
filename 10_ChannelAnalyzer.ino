// ════════════════════════════════════════════════════════════════
// 10 — Channel Traffic Analyzer
// Spends 1s on each channel 1-13, counts packets, shows bar chart.
// Helps pick least-congested WiFi channel.
// ════════════════════════════════════════════════════════════════
#include <WiFi.h>
#include <esp_wifi.h>
#include <TFT_eSPI.h>
#include "common.h"

TFT_eSPI tft = TFT_eSPI(128, 128);

volatile uint32_t chanCnt[14] = {0};
uint8_t currCh = 1;
uint32_t maxCnt = 1;

static void IRAM_ATTR sniffCB(void* buf, wifi_promiscuous_pkt_type_t type){
  if(currCh >= 1 && currCh <= 13) chanCnt[currCh]++;
}

void render(){
  tft.fillRect(0, 14, 128, 104, C_BG);
  // find max
  uint32_t mx = 1;
  for(int i=1; i<=13; i++) if(chanCnt[i] > mx) mx = chanCnt[i];
  maxCnt = mx;
  // bars at x=2..128, one bar = 9px wide incl gap for 13 channels
  int barW = 9;
  int baseY = 105;
  for(int ch=1; ch<=13; ch++){
    int h = (chanCnt[ch] * 70) / mx;
    if(h < 1 && chanCnt[ch] > 0) h = 1;
    int x = 2 + (ch-1)*barW;
    uint16_t col = (ch == currCh) ? C_GOLD : (h > 40 ? C_RED : h > 15 ? C_YLW : C_GREEN);
    tft.fillRect(x, baseY-h, barW-1, h, col);
    tft.setTextColor(C_GOLDD, C_BG); tft.setTextSize(1);
    tft.setCursor(x, 107); tft.printf("%d", ch);
  }
  tft.setTextColor(C_CYAN, C_BG);
  tft.setCursor(2, 17); tft.printf("Ch %d  max:%lu", currCh, mx);
  drawFooter(tft, "SEL:reset");
}

void setup(){
  Serial.begin(115200);
  tftInit(tft); buzzInit(); ledInit(); buttonsInit();
  drawTitle(tft, "CHANNEL SCAN");
  WiFi.mode(WIFI_STA); WiFi.disconnect(true); delay(100);
  esp_wifi_set_promiscuous(true);
  wifi_promiscuous_filter_t f = { .filter_mask = WIFI_PROMIS_FILTER_MASK_ALL };
  esp_wifi_set_promiscuous_filter(&f);
  esp_wifi_set_promiscuous_rx_cb(sniffCB);
  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);
  currCh = 1;
  render();
  beepOK();
}

uint32_t lastHop=0, lastRender=0;
bool lSel=false;
void loop(){
  uint32_t now = millis();
  if(now - lastHop > 1000){
    lastHop = now;
    currCh = (currCh % 13) + 1;
    esp_wifi_set_channel(currCh, WIFI_SECOND_CHAN_NONE);
  }
  if(now - lastRender > 500){ lastRender = now; render(); }
  if(buttonPressed(BTN_SEL, lSel)){
    for(int i=0;i<14;i++) chanCnt[i]=0;
    render(); beepOK();
  }
  delay(30);
}
