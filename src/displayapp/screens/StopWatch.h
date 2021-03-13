#pragma once

#include "Screen.h"
#include "components/datetime/DateTimeController.h"
#include "../LittleVgl.h"

#include "FreeRTOS.h"
#include "portmacro_cmsis.h"

#include <array>

namespace Pinetime::Applications::Screens {

  enum class States { INIT, RUNNING, HALTED };

  enum class Events { PLAY, PAUSE, STOP };

  struct TimeSeparated_t {
    int mins;
    int secs;
    int msecs;
  };

  // A simple buffer to hold the latest two laps
  template <int N> struct LapTextBuffer_t {
    LapTextBuffer_t() : _arr {}, currentSz {}, capacity {N}, head {-1} {
    }

    void addLaps(const TimeSeparated_t& timeVal) {
      head++;
      head %= capacity;
      _arr[head] = timeVal;

      if (currentSz < capacity) {
        currentSz++;
      }
    }

    void clearBuffer() {
      _arr = {};
      currentSz = 0;
      head = -1;
    }

    TimeSeparated_t* operator[](std::size_t idx) {
      // Sanity check for out-of-bounds
      if (idx >= 0 && idx < capacity) {
        if (idx < currentSz) {
          // This transformation is to ensure that head is always pointing to index 0.
          const auto transformed_idx = (head - idx) % capacity;
          return (&_arr[transformed_idx]);
        }
      }
      return nullptr;
    }

  private:
    std::array<TimeSeparated_t, N> _arr;
    uint8_t currentSz;
    uint8_t capacity;
    int8_t head;
  };

  class StopWatch : public Screen {
  public:
    StopWatch(DisplayApp* app, const Pinetime::Controllers::DateTime& dateTime);
    ~StopWatch() override;
    bool Refresh() override;
    bool OnButtonPushed() override;
    void playPauseBtnEventHandler(lv_event_t event);
    void stopLapBtnEventHandler(lv_event_t event);

  private:
    const Pinetime::Controllers::DateTime& dateTime;
    bool running;
    States currentState;
    Events currentEvent;
    TickType_t startTime;
    TickType_t oldTimeElapsed;
    TimeSeparated_t currentTimeSeparated; // Holds Mins, Secs, millisecs
    LapTextBuffer_t<2> lapBuffer;
    int lapNr;
    bool lapPressed;
    lv_obj_t *time, *msecTime, *btnPlayPause, *btnStopLap, *txtPlayPause, *txtStopLap;
    lv_obj_t *lapOneText, *lapTwoText;
  };
}