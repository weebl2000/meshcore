#include "BaseUITask.h"
#include <Arduino.h>
#include <helpers/ArduinoHelpers.h>

void BaseUITask::loop() {
  if (_display->isOn()) {
    if (millis_passed(_next_refresh)) {
      _display->startFrame();
      long interval = renderCurrScreen();
      _display->endFrame();

      _next_refresh = millis() + interval;
    }
    if (millis_passed(_auto_off)) {
      _display->turnOff();
    }
  }
}