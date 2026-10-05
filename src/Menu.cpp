#include "Menu.h"
#include <Fonts/FreeMonoBold9pt7b.h>
#include "config.h"

void drawMenu(Adafruit_GFX &d, int menuIndex) {
  const uint16_t ink = 0x0000, paper = 0xFFFF; // GxEPD_BLACK, GxEPD_WHITE
  const char *menuItems[] = {
      "About Watchy", "Vibrate Motor", "Show Accelerometer",
      "Set Time",     "Setup WiFi",    /*"Update Firmware",*/
      "Sync NTP",     "Refresh Screen"};
  int16_t x1, y1;
  uint16_t w, h;

  d.fillScreen(paper);
  d.setFont(&FreeMonoBold9pt7b);
  for (int i = 0; i < MENU_LENGTH; i++) {
    int16_t yPos = MENU_HEIGHT + (MENU_HEIGHT * i);
    d.setCursor(0, yPos);
    if (i == menuIndex) {
      d.getTextBounds(menuItems[i], 0, yPos, &x1, &y1, &w, &h);
      d.fillRect(x1 - 1, y1 - 10, 200, h + 15, ink);
      d.setTextColor(paper);
    } else {
      d.setTextColor(ink);
    }
    d.println(menuItems[i]);
  }
}
