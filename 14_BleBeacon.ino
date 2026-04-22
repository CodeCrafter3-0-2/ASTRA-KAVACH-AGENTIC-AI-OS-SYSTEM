// ════════════════════════════════════════════════════════════════
// 14 — BLE Beacon Broadcaster
// Broadcasts as iBeacon or Eddystone URL. LEFT/RIGHT: switch type
// SEL: start/stop advertising
// ════════════════════════════════════════════════════════════════
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLEBeacon.h>
#include <BLEAdvertising.h>
#include <TFT_eSPI.h>
#include "common.h"

TFT_eSPI tft = TFT_eSPI(128, 128);
BLEAdvertising* advertising;

enum BType { B_IBEACON, B_EDDYSTONE };
BType btype = B_IBEACON;
bool advertOn = false;

// iBeacon config
String ibeaconUuid = "e2c56db5-dffb-48d2-b060-d0f5a71096e0";
uint16_t ibMajor = 0x0001;
uint16_t ibMinor = 0x0001;
int8_t   ibTxPower = -58;

// Eddystone URL
String eddyUrl = "https://anthropic.com";

void startIBeacon(){
  BLEBeacon b;
  b.setManufacturerId(0x004C);
  b.setProximityUUID(BLEUUID(ibeaconUuid));
  b.setMajor(ibMajor);
  b.setMinor(ibMinor);
  b.setSignalPower(ibTxPower);
  BLEAdvertisementData adv;
  adv.setFlags(0x04);
  std::string payload = "";
  payload += (char)26;      // len
  payload += (char)0xFF;    // mfg data
  std::string data = b.getData();
  payload += data;
  adv.addData(payload);
  advertising->setAdvertisementData(adv);
}

// Eddystone URL scheme: 0x00=http://www. 0x01=https://www. 0x02=http:// 0x03=https://
uint8_t urlScheme(const String& u, String& rest){
  if(u.startsWith("https://www.")){ rest = u.substring(12); return 0x01; }
  if(u.startsWith("http://www.")){  rest = u.substring(11); return 0x00; }
  if(u.startsWith("https://")){     rest = u.substring(8);  return 0x03; }
  if(u.startsWith("http://")){      rest = u.substring(7);  return 0x02; }
  rest = u; return 0x03;
}

void startEddystone(){
  String body; uint8_t sch = urlScheme(eddyUrl, body);
  BLEAdvertisementData adv;
  adv.setFlags(0x06);
  std::string serviceData;
  serviceData += (char)0x10;            // frame type: URL
  serviceData += (char)0xF6;            // tx power
  serviceData += (char)sch;
  for(size_t i=0; i<body.length() && i<17; i++) serviceData += body[i];
  // assemble full advertising packet manually
  std::string full;
  full += (char)0x03; full += (char)0x03; full += (char)0xAA; full += (char)0xFE;
  full += (char)(serviceData.length()+3);
  full += (char)0x16; full += (char)0xAA; full += (char)0xFE;
  full += serviceData;
  adv.addData(full);
  advertising->setAdvertisementData(adv);
}

void startAdvert(){
  BLEDevice::deinit(false);
  delay(100);
  BLEDevice::init("AstraBcn");
  advertising = BLEDevice::getAdvertising();
  if(btype == B_IBEACON) startIBeacon();
  else startEddystone();
  advertising->start();
  advertOn = true;
}

void stopAdvert(){
  if(advertOn) advertising->stop();
  advertOn = false;
}

void render(){
  tft.fillRect(0, 14, 128, 104, C_BG);
  tft.setTextColor(C_GOLD, C_BG);
  tft.setCursor(2, 20); tft.print("Type:");
  tft.setTextColor(C_CYAN, C_BG);
  tft.setCursor(38, 20); tft.print(btype == B_IBEACON ? "iBeacon" : "Eddystone");
  tft.setTextColor(C_GOLD, C_BG);
  tft.setCursor(2, 40); tft.print("Status:");
  tft.setTextColor(advertOn ? C_GREEN : C_RED, C_BG);
  tft.setCursor(48, 40); tft.print(advertOn ? "ADVERTISING" : "OFF");
  if(btype == B_IBEACON){
    tft.setTextColor(C_GOLDD, C_BG);
    tft.setCursor(2, 60); tft.print("UUID:"); tft.setCursor(2, 72);
    tft.print(ibeaconUuid.substring(0,22));
    tft.setCursor(2, 88); tft.printf("Mj:%d Mn:%d", ibMajor, ibMinor);
  } else {
    tft.setTextColor(C_GOLDD, C_BG);
    tft.setCursor(2, 60); tft.print("URL:");
    tft.setCursor(2, 72); tft.print(eddyUrl.substring(0, 20));
  }
  drawFooter(tft, "L/R:type SEL:tog");
}

void setup(){
  Serial.begin(115200);
  tftInit(tft); buzzInit(); ledInit(); buttonsInit();
  drawTitle(tft, "BLE BEACON");
  BLEDevice::init("AstraBcn");
  advertising = BLEDevice::getAdvertising();
  render();
  beepOK();
}

bool lLeft=false, lRight=false, lSel=false;
void loop(){
  if(buttonPressed(BTN_LEFT, lLeft) || buttonPressed(BTN_RIGHT, lRight)){
    if(advertOn) stopAdvert();
    btype = (btype == B_IBEACON) ? B_EDDYSTONE : B_IBEACON;
    render(); beepOK();
  }
  if(buttonPressed(BTN_SEL, lSel)){
    if(advertOn){ stopAdvert(); ledOff(); }
    else        { startAdvert(); ledOn(); }
    render(); beepOK();
  }
  delay(50);
}
