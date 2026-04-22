// ════════════════════════════════════════════════════════════════
// 13 — Clock / Stopwatch / Timer
// LEFT/RIGHT: switch mode (CLOCK / STOPWATCH / TIMER)
// CLOCK: UP/DN to set hours, SEL hold 2s to switch to setting mins
// STOPWATCH: SEL start/stop, UP reset
// TIMER: UP/DN adjust, SEL start, RIGHT reset
// (No RTC — clock counts from user-set time, resets on reboot)
// ════════════════════════════════════════════════════════════════
#include <TFT_eSPI.h>
#include "common.h"

TFT_eSPI tft = TFT_eSPI(128, 128);

enum Mode { M_CLOCK, M_SW, M_TIMER };
Mode mode = M_CLOCK;
const char* modeName[] = {"CLOCK", "STOPWATCH", "TIMER"};

// Clock
int clkH = 12, clkM = 0, clkS = 0;
uint32_t clkTick = 0;
bool setMins = false;

// Stopwatch
uint32_t swStart = 0, swAcc = 0;
bool swRun = false;

// Timer
int timerSet = 60;   // seconds
int timerRemain = 0;
uint32_t timerStart = 0;
bool timerRun = false;
bool timerDone = false;

void drawBig(int a, int b, int c){
  tft.fillRect(0, 40, 128, 50, C_BG);
  tft.setTextSize(3);
  tft.setTextColor(C_CYAN, C_BG);
  char buf[12]; snprintf(buf, 12, "%02d:%02d:%02d", a, b, c);
  // ~6*3=18 px/char, "HH:MM:SS" = 8 chars = 144px, too wide. Use size 2.
  tft.setTextSize(2);
  int w = 8 * 12;
  tft.setCursor((128-w)/2, 50);
  tft.print(buf);
  tft.setTextSize(1);
}

void render(){
  tft.fillRect(0, 14, 128, 104, C_BG);
  tft.setTextColor(C_GOLD, C_BG);
  tft.setCursor(2, 20); tft.print(modeName[mode]);
  if(mode == M_CLOCK){
    drawBig(clkH, clkM, clkS);
    tft.setTextColor(C_GOLDD, C_BG);
    tft.setCursor(2, 100); tft.print(setMins ? "Set minutes" : "Set hours");
  } else if(mode == M_SW){
    uint32_t ms = swAcc + (swRun ? (millis()-swStart) : 0);
    int h = ms/3600000;
    int m = (ms/60000)%60;
    int s = (ms/1000)%60;
    drawBig(h, m, s);
    tft.setTextColor(swRun ? C_GREEN : C_GOLDD, C_BG);
    tft.setCursor(2, 100); tft.print(swRun ? "RUNNING" : "STOPPED");
  } else {
    int rem = timerRemain;
    if(timerRun) rem = timerRemain - (millis()-timerStart)/1000;
    if(rem < 0) rem = 0;
    int h = rem/3600, m = (rem/60)%60, s = rem%60;
    drawBig(h, m, s);
    tft.setTextColor(timerDone ? C_RED : (timerRun ? C_GREEN : C_GOLDD), C_BG);
    tft.setCursor(2, 100); tft.print(timerDone ? "ALARM!" : (timerRun ? "RUNNING" : "SET"));
  }
  drawFooter(tft, "L/R:mode UP/DN");
}

void setup(){
  Serial.begin(115200);
  tftInit(tft); buzzInit(); ledInit(); buttonsInit();
  drawTitle(tft, "UTIL");
  render();
  beepOK();
}

bool lUp=false, lDown=false, lLeft=false, lRight=false, lSel=false;
uint32_t selPressStart = 0;
void loop(){
  // clock tick
  if(millis() - clkTick >= 1000){
    clkTick += 1000;
    clkS++;
    if(clkS>=60){ clkS=0; clkM++; if(clkM>=60){ clkM=0; clkH++; if(clkH>=24) clkH=0; } }
  }
  // timer tick
  if(timerRun && !timerDone){
    int rem = timerRemain - (int)((millis()-timerStart)/1000);
    if(rem <= 0){
      timerRun = false; timerDone = true;
      for(int i=0;i<5;i++){ buzz(2200, 150); delay(80); ledOn(); delay(50); ledOff(); }
    }
  }
  // navigation
  if(buttonPressed(BTN_LEFT, lLeft))  { mode = (Mode)((mode+2)%3); render(); }
  if(buttonPressed(BTN_RIGHT, lRight)){
    if(mode == M_TIMER){ timerDone=false; timerRun=false; timerRemain=timerSet; }
    else mode = (Mode)((mode+1)%3);
    render();
  }
  if(buttonPressed(BTN_UP, lUp)){
    if(mode==M_CLOCK){ if(setMins) clkM = (clkM+1)%60; else clkH = (clkH+1)%24; render(); }
    else if(mode==M_SW){ swRun=false; swAcc=0; render(); }
    else { timerSet += 10; if(timerSet>3600) timerSet=3600; timerRemain=timerSet; render(); }
  }
  if(buttonPressed(BTN_DOWN, lDown)){
    if(mode==M_CLOCK){ if(setMins) clkM = (clkM+59)%60; else clkH = (clkH+23)%24; render(); }
    else if(mode==M_TIMER){ timerSet -= 10; if(timerSet<10) timerSet=10; timerRemain=timerSet; render(); }
  }
  // SEL — short = action, hold = toggle h/m in clock
  bool nowSel = digitalRead(BTN_SEL) == LOW;
  if(nowSel && !lSel){ selPressStart = millis(); lSel = true; }
  if(!nowSel && lSel){
    uint32_t dur = millis() - selPressStart;
    lSel = false;
    if(dur > 1500 && mode == M_CLOCK){ setMins = !setMins; render(); }
    else if(dur > 30){
      if(mode == M_SW){
        if(swRun){ swAcc += millis()-swStart; swRun=false; }
        else     { swStart = millis(); swRun=true; }
        render();
      } else if(mode == M_TIMER){
        if(!timerRun && !timerDone){ timerStart = millis(); timerRun = true; }
        render();
      }
    }
  }
  // 1Hz refresh for running clock/stopwatch/timer
  static uint32_t lastRefresh=0;
  if(millis()-lastRefresh > 500){ lastRefresh=millis(); render(); }
  delay(20);
}
