// ════════════════════════════════════════════════════════════════
// 03 — Deauth Detector (lightweight, WiFi only, no BLE)
// Listens for 802.11 deauth/disassoc frames on hopping channels
// Alert on TFT + buzzer + LED when detected
// ════════════════════════════════════════════════════════════════
#include <WiFi.h>
#include <esp_wifi.h>
#include <TFT_eSPI.h>
#include "common.h"

TFT_eSPI tft = TFT_eSPI(128, 128);

volatile uint32_t deauthCnt = 0;
volatile uint32_t lastDeauthMs = 0;
volatile int8_t   lastRssi = 0;
volatile uint8_t  lastMac[6] = {0};
volatile uint8_t  lastCh = 0;
uint8_t currCh = 1;
uint32_t totalDeauths = 0;

static void IRAM_ATTR snifferCB(void* buf, wifi_promiscuous_pkt_type_t type){
  if(type != WIFI_PKT_MGMT) return;
  auto* pkt = (wifi_promiscuous_pkt_t*)buf;
  uint16_t len = pkt->rx_ctrl.sig_len;
  if(len < 24) return;
  uint8_t fc = pkt->payload[0];
  if(fc == 0xC0 || fc == 0xA0){   // deauth or disassoc
    deauthCnt++;
    totalDeauths++;
    lastDeauthMs = millis();
    lastRssi = pkt->rx_ctrl.rssi;
    memcpy((void*)lastMac, pkt->payload + 10, 6);
    lastCh = currCh;
  }
}

void renderMain(){
  tft.fillScreen(C_BG);
  drawTitle(tft, "DEAUTH DETECTOR");
  tft.setTextColor(C_GOLD, C_BG); tft.setTextSize(1);
  tft.setCursor(2, 20); tft.print("Channel:");
  tft.setCursor(2, 40); tft.print("Total:");
  tft.setCursor(2, 60); tft.print("Status:");
  drawFooter(tft, "Hopping 1-13");
}

void updateStatus(){
  tft.fillRect(60, 18, 70, 60, C_BG);
  tft.setTextColor(C_CYAN, C_BG); tft.setCursor(60, 20); tft.printf("%d", currCh);
  tft.setTextColor(C_GOLDD, C_BG); tft.setCursor(60, 40); tft.printf("%lu", totalDeauths);
  if(millis() - lastDeauthMs < 3000 && lastDeauthMs > 0){
    tft.setTextColor(C_RED, C_BG); tft.setCursor(60, 60); tft.print("ATTACK!");
  } else {
    tft.setTextColor(C_GREEN, C_BG); tft.setCursor(60, 60); tft.print("CLEAN");
  }
}

void showAlert(){
  tft.fillScreen(C_RED);
  tft.setTextColor(C_WHT, C_RED); tft.setTextSize(2);
  tft.setCursor(10, 20); tft.print("DEAUTH!");
  tft.setTextSize(1);
  tft.setCursor(2, 55); tft.printf("RSSI: %d dBm", lastRssi);
  tft.setCursor(2, 70); tft.print("MAC:");
  uint8_t m[6]; for(int i=0;i<6;i++) m[i]=lastMac[i];
  tft.setCursor(2, 82); tft.print(macFmt(m));
  tft.setCursor(2, 100); tft.printf("Channel: %d", lastCh);
  ledOn();
  for(int f=1400; f<=2800; f+=200) buzz(f, 35);
  for(int f=2800; f>=1400; f-=200) buzz(f, 35);
  delay(3000);
  ledOff();
  renderMain();
}

void setup(){
  Serial.begin(115200);
  tftInit(tft); buzzInit(); ledInit(); buttonsInit();
  WiFi.mode(WIFI_STA); WiFi.disconnect(true); delay(100);
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_promiscuous_rx_cb(snifferCB);
  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);
  currCh = 1;
  renderMain();
  buzz(1000, 50); buzz(1500, 50); buzz(2000, 100);
}

uint32_t lastHop = 0;
uint32_t lastUi = 0;
uint32_t lastAlertShown = 0;

void loop(){
  uint32_t now = millis();
  if(now - lastHop > 500){
    lastHop = now;
    currCh = (currCh % 13) + 1;
    esp_wifi_set_channel(currCh, WIFI_SECOND_CHAN_NONE);
  }
  if(now - lastUi > 300){ lastUi = now; updateStatus(); }
  if(deauthCnt > 0 && now - lastAlertShown > 5000){
    deauthCnt = 0;
    lastAlertShown = now;
    showAlert();
  }
  delay(10);
}
