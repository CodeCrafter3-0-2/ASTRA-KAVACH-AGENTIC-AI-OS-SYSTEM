// ── ASTRA KAVACH — TFT_eSPI User_Setup.h (v4 + WHITE-SCREEN DEBUG) ────
// Copy this file to: Arduino/libraries/TFT_eSPI/User_Setup.h
//
// WIRING (v4):
//   GND      → GND       (Black)
//   VCC      → 3.3V ONLY (Red)  ← NOT 5V unless module explicitly supports it
//   CS       → GPIO 27   (Blue)
//   DC/RS    → GPIO 26   (Teal)
//   RST      → GPIO 5    (Yellow)
//   SDA/MOSI → GPIO 23   (Purple)
//   SCL/SCK  → GPIO 18   (Orange)
//   LED/BL   → GPIO 32   (Green)

#define ST7735_DRIVER
#define TFT_WIDTH  128
#define TFT_HEIGHT 128
#define TFT_MOSI   23
#define TFT_SCLK   18
#define TFT_CS     27
#define TFT_DC     26
#define TFT_RST     5
#define TFT_BL     32

// ══════════════════════════════════════════════════════════════════════
// ⚠️  WHITE SCREEN TROUBLESHOOTING — TAB VARIANT
// ══════════════════════════════════════════════════════════════════════
// ST7735 panels ship with different "tab" colors on the ribbon cable
// Each needs a DIFFERENT init sequence. If display is white/garbled,
// try these ONE AT A TIME:
//   1. Uncomment ONE variant below
//   2. Comment the others
//   3. Re-compile + flash
//   4. If still white, try next variant
//
// NOTE: physically look at the ribbon cable of your display — some
// have a colored tape/sticker indicating which variant it is.
// ══════════════════════════════════════════════════════════════════════

#define ST7735_GREENTAB       // ← default (try FIRST)
// #define ST7735_GREENTAB2   // ← try if above is white
// #define ST7735_GREENTAB3   // ← try if above is white
// #define ST7735_GREENTAB128 // ← try if above is white (128x128 specific)
// #define ST7735_GREENTAB160x80 // ← unusual panels
// #define ST7735_BLACKTAB    // ← try if above is white
// #define ST7735_REDTAB      // ← try if all GREEN variants fail
// #define ST7735_REDTAB160x80

// ── Color inversion (if colors look wrong or screen is uniformly lit) ──
// Uncomment ONE of these if needed:
// #define TFT_INVERSION_ON
// #define TFT_INVERSION_OFF

// ══════════════════════════════════════════════════════════════════════
// ⚠️  SPI FREQUENCY — IF STILL WHITE, DROP THIS
// ══════════════════════════════════════════════════════════════════════
// Long/dupont jumper wires can corrupt 27MHz SPI. If display shows
// garbled or white screen, drop to 10-16 MHz.
// Change ONE active line:

#define SPI_FREQUENCY       27000000   // default 27 MHz
// #define SPI_FREQUENCY    16000000   // fallback 1: 16 MHz
// #define SPI_FREQUENCY    10000000   // fallback 2: 10 MHz (most reliable)
// #define SPI_FREQUENCY     8000000   // fallback 3: 8 MHz (long wires)

#define SPI_READ_FREQUENCY  20000000

// Font loading
#define LOAD_GLCD
#define LOAD_FONT2
#define SMOOTH_FONT
