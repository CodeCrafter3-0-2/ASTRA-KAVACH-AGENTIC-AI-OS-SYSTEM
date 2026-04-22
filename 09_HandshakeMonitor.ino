// ════════════════════════════════════════════════════════════════
// 09 — WPA Handshake Monitor
// Passively detects EAPOL (WPA 4-way handshake) frames on sniffer
// Shows count + last detected MAC. Does NOT capture or decrypt.
// ════════════════════════════════════════════════════════════════
#include <WiFi.h>
#include <esp_wifi.h>
#include <TFT_eSPI.h>
#include "common.h"

TFT_eSPI tft = TFT_eSPI(128, 128);

volatile uint32_t eapolCnt = 0;
volatile uint32_t dataCnt  = 0;
volatile uint32_t lastEapolTs = 0;
volatile uint8_t  lastMac[6] = {0};
uint8_t currCh = 1;

static void IRAM_ATTR sniffCB(void* buf, wifi_promiscuous_pkt_type_t type){
  if(type != WIFI_PKT_DATA) return;
  auto* pkt = (wifi_promiscuous_pkt_t*)buf;
  uint16_t len = pkt->rx_ctrl.sig_len;
  if(len < 36) return;
  dataCnt++;
  // EAPOL LLC signature: 88 8E at offset 32 after 802.11 header + LLC/SNAP
  if(pkt->payload[32] == 0x88 && pkt->payload[33] == 0x8E){
    eapolCnt++;
    lastEapolTs = millis();
    memcpy((void*)lastMac, pkt->payload + 10, 6);
  }
}

void render(){
  tft.fillRect(0, 14, 128, 104, C_BG);
  tft.setTextColor(C_GOLD, C_BG); tft.setTextSize(1);
  tft.setCursor(2, 20); tft.print("EAPOL:");
  tft.setCursor(2, 35); tft.print("DATA:");
  tft.setCursor(2, 50); tft.print("CHAN:");
  tft.setCursor(2, 70); tft.print("Last MAC:");
  tft.setTextColor(C_CYAN, C_BG);
  tft.setCursor(50, 20); tft.printf("%lu", eapolCnt);
  tft.setCursor(50, 35); tft.printf("%lu", dataCnt);
  tft.setCursor(50, 50); tft.printf("%d", currCh);
  if(lastEapolTs){
    uint8_t m[6]; for(int i=0;i<6;i++) m[i]=lastMac[i];
    tft.setCursor(2, 83); tft.print(macFmt(m));
    uint32_t age = (millis()-lastEapolTs)/1000;
    tft.setTextColor(C_GOLDD, C_BG);
    tft.setCursor(2, 96); tft.printf("%lus ago", age);
  }
  if(millis() - lastEapolTs < 2000 && lastEapolTs > 0){
    tft.fillRect(90, 70, 36, 28, C_RED);
    tft.setTextColor(C_WHT, C_RED); tft.setCursor(94, 83); tft.print("LIVE");
  }
  drawFooter(tft, "SEL:reset");
}

void setup(){
  Serial.begin(115200);
  tftInit(tft); buzzInit(); ledInit(); buttonsInit();
  drawTitle(tft, "HANDSHAKE MON");
  WiFi.mode(WIFI_STA); WiFi.disconnect(true); delay(100);
  // enable MGMT + DATA for EAPOL detection
  esp_wifi_set_promiscuous(true);
  wifi_promiscuous_filter_t f = { .filter_mask = WIFI_PROMIS_FILTER_MASK_DATA | WIFI_PROMIS_FILTER_MASK_MGMT };
  esp_wifi_set_promiscuous_filter(&f);
  esp_wifi_set_promiscuous_rx_cb(sniffCB);
  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);
  render();
  beepOK();
}

uint32_t lastHop=0, lastRender=0;
uint32_t lastAlert = 0;
bool lSel=false;
void loop(){
  uint32_t now = millis();
  if(now - lastHop > 500){
    lastHop = now;
    currCh = (currCh % 13) + 1;
    esp_wifi_set_channel(currCh, WIFI_SECOND_CHAN_NONE);
  }
  if(now - lastRender > 400){ lastRender = now; render(); }
  // alert on new EAPOL
  if(lastEapolTs > lastAlert + 2000){
    lastAlert = lastEapolTs;
    if(eapolCnt > 0){ ledOn(); buzz(2000, 100); delay(100); ledOff(); }
  }
  if(buttonPressed(BTN_SEL, lSel)){
    eapolCnt = 0; dataCnt = 0; lastEapolTs = 0;
    render(); beepOK();
  }
  delay(30);
}
