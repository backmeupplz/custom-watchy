#include <Watchy.h>
#include "settings.h"
#include "timers.h"
#include "ui.h"
#include "weather.h"

#define APP_IDLE_MS 30000 // leave the timer app after this long without a press

RTC_DATA_ATTR uint32_t lastWeatherAttempt;
RTC_DATA_ATTR uint8_t stepDay;
RTC_DATA_ATTR uint8_t lastHms[3] = {0, 5, 0}; // picker starts at the last used duration

enum Button { NONE, MENU, BACK, UP, DOWN };
static const uint8_t BUTTON_PINS[] = {MENU_BTN_PIN, BACK_BTN_PIN, UP_BTN_PIN, DOWN_BTN_PIN};

static Button readButton() {
  for (uint8_t i = 0; i < 4; i++)
    if (digitalRead(BUTTON_PINS[i]) == ACTIVE_LOW) return Button(MENU + i);
  return NONE;
}

// The CPU light-sleeps while the display refreshes; in apps, also wake on buttons
// and remember the press so quick taps during a refresh aren't lost.
static Button latched = NONE;
static void appBusyCallback(const void *) {
  for (uint8_t pin : BUTTON_PINS)
    gpio_wakeup_enable((gpio_num_t)pin, ACTIVE_LOW ? GPIO_INTR_HIGH_LEVEL : GPIO_INTR_LOW_LEVEL);
  WatchyDisplay::busyCallback(nullptr);
  for (uint8_t pin : BUTTON_PINS) gpio_wakeup_disable((gpio_num_t)pin);
  if (latched == NONE) latched = readButton();
}

class Face : public Watchy {
public:
  using Watchy::Watchy;

  uint32_t now() {
    RTC.read(currentTime);
    return makeTime(currentTime);
  }

  void buzz() { vibMotor(120, 20); }

  void onWake() override {
    if (timersFire(now())) buzz();
  }

  time_t nextAlarm() override { return timerNextEnd(); }

  void handleButtonPress() override {
    if (guiState == WATCHFACE_STATE && (esp_sleep_get_ext1_wakeup_status() & DOWN_BTN_MASK)) {
      timerApp();
      return;
    }
    Watchy::handleButtonPress();
  }

  void drawWatchFace() override {
    uint32_t t = makeTime(currentTime);
    if (t - lastWeatherAttempt >= WEATHER_INTERVAL_MIN * 60 || t < lastWeatherAttempt) {
      lastWeatherAttempt = t;
      if (connectWiFi()) {
        int32_t offset;
        if (weatherFetch(settings.lat.c_str(), settings.lon.c_str(), offset)) {
          syncNTP(offset);
          t = now();
          lastWeatherAttempt = t;
        }
        WiFi.mode(WIFI_OFF);
        btStop();
      }
    }

    if (currentTime.Day != stepDay) { // new day
      sensor.resetStepCounter();
      stepDay = currentTime.Day;
    }

    FaceData f = {currentTime.Hour, currentTime.Minute, currentTime.Wday, currentTime.Day, currentTime.Month,
                  sensor.getCounter(), weatherView(t)};
    f.timerCount = timerViews(t, f.timers, FACE_TIMERS);
    drawFace(display, f);
  }

  /* ---------------- timer app ---------------- */

  static void waitRelease() {
    while (readButton() != NONE) delay(10);
  }

  // Up/Down repeat while held (the display refresh paces them); Menu/Back fire once per press.
  Button waitButton(uint32_t &lastPress) {
    Button b = latched != NONE ? latched : readButton();
    latched = NONE;
    if (b == MENU || b == BACK) waitRelease();
    if (b != NONE) lastPress = millis();
    else delay(20);
    return b;
  }

  void timerApp() {
    guiState = APP_STATE;
    for (uint8_t pin : BUTTON_PINS) pinMode(pin, INPUT);
    waitRelease(); // the Down press that opened us
    display.epd2.setBusyCallback(appBusyCallback);

    uint8_t sel = 0;
    uint32_t lastPress = millis(), lastSecond = 0;
    bool dirty = true;
    TimerView views[MAX_TIMERS];
    while (millis() - lastPress < APP_IDLE_MS) {
      uint32_t t = now();
      if (timersFire(t)) {
        buzz();
        dirty = true;
      }
      if (t != lastSecond) { // countdowns tick
        lastSecond = t;
        dirty = dirty || timerNextEnd();
      }
      if (dirty) {
        uint8_t n = timerViews(t, views, MAX_TIMERS);
        drawTimerList(display, views, n, sel);
        refresh();
        dirty = false;
      }

      Button b = waitButton(lastPress);
      if (b == BACK) break;
      if (b == UP) sel = sel ? sel - 1 : timerCount;
      if (b == DOWN) sel = sel < timerCount ? sel + 1 : 0;
      if (b == MENU && sel == 0) {
        if (pickDuration(lastPress)) {
          timerAdd(now(), lastHms[0] * 3600 + lastHms[1] * 60 + lastHms[2]);
          break; // straight back to the face to see it running
        }
      } else if (b == MENU) {
        timerRemove(sel - 1);
        if (sel > timerCount) sel = timerCount;
      }
      dirty = dirty || b != NONE;
    }
    display.epd2.setBusyCallback(WatchyDisplay::busyCallback);
    RTC.read(currentTime);
    showWatchFace();
  }

  // Edits lastHms; true = start, false = cancelled.
  bool pickDuration(uint32_t &lastPress) {
    static const uint8_t LIMIT[] = {24, 60, 60}, STEP[] = {1, 1, 5};
    uint8_t hms[3] = {lastHms[0], lastHms[1], lastHms[2]}, field = 0;
    bool dirty = true;
    while (millis() - lastPress < APP_IDLE_MS) {
      if (dirty) {
        drawTimerPicker(display, hms, field);
        refresh();
        dirty = false;
      }
      Button b = waitButton(lastPress);
      dirty = b != NONE;
      if (b == UP) hms[field] = (hms[field] + STEP[field]) % LIMIT[field];
      if (b == DOWN) hms[field] = (hms[field] + LIMIT[field] - STEP[field]) % LIMIT[field];
      if (b == BACK) {
        if (field == 0) return false;
        field--;
      }
      if (b == MENU) {
        if (field < 2) {
          field++;
        } else if (hms[0] || hms[1] || hms[2]) {
          memcpy(lastHms, hms, 3);
          return true;
        }
      }
    }
    return false;
  }
};

Face watchy(settings);

void setup() {
#ifdef ARDUINO_ESP32S3_DEV
  // V3 keeps time in the ESP32-S3 itself and loses it on reset: start from build time until NTP sync
  struct tm t = {};
  strptime(__DATE__ " " __TIME__, "%b %d %Y %H:%M:%S", &t);
  char buf[20];
  strftime(buf, sizeof buf, "%Y:%m:%d:%H:%M:%S", &t);
  watchy.init(buf);
#else
  watchy.init();
#endif
}
void loop() {}
