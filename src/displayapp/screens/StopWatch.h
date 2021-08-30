#pragma once

#include "Screen.h"
#include "components/datetime/DateTimeController.h"
#include "../LittleVgl.h"

#include "FreeRTOS.h"
#include "portmacro_cmsis.h"

#include <array>
#include "systemtask/SystemTask.h"

namespace Pinetime::Applications::Screens {

  enum class States { Init, Running, Halted };

  struct TimeSeparated_t {
    int mins;
    int secs;
    int hundredths;
  };

  // A simple buffer to hold the latest two laps
  template <int N> struct LapTextBuffer_t {
    LapTextBuffer_t() : buffer {}, currentSize {}, capacity {N}, head {-1} {
    }

    void addLaps(const TimeSeparated_t& timeVal) {
      head++;
      head %= capacity;
      buffer[head] = timeVal;

      if (currentSize < capacity) {
        currentSize++;
      }
    }

    void clearBuffer() {
      buffer = {};
      currentSize = 0;
      head = -1;
    }

    TimeSeparated_t* operator[](std::size_t idx) {
      // Sanity check for out-of-bounds
      if (idx >= 0 && idx < capacity) {
        if (idx < currentSize) {
          // This transformation is to ensure that head is always pointing to index 0.
          const auto transformed_idx = (head - idx) % capacity;
          return (&buffer[transformed_idx]);
        }
      }
      return nullptr;
    }

  private:
    std::array<TimeSeparated_t, N> buffer;
    uint8_t currentSize;
    uint8_t capacity;
    int8_t head;
  };

  class StopWatch : public Screen {
  public:
    StopWatch(DisplayApp* app, System::SystemTask& systemTask);
    ~StopWatch() override;
    void Refresh() override;

    void playPauseBtnEventHandler(lv_event_t event);
    void stopLapBtnEventHandler(lv_event_t event);
    bool OnButtonPushed() override;

    void reset();
    void start();
    void pause();

  private:
    Pinetime::System::SystemTask& systemTask;
    TickType_t timeElapsed;
    States currentState;
    TickType_t startTime;
    TickType_t oldTimeElapsed;
    TimeSeparated_t currentTimeSeparated; // Holds Mins, Secs, millisecs
    LapTextBuffer_t<2> lapBuffer;
    int lapNr = 0;
    lv_obj_t *time, *msecTime, *btnPlayPause, *btnStopLap, *txtPlayPause, *txtStopLap;
    lv_obj_t *lapOneText, *lapTwoText;

    lv_task_t* taskRefresh;
  };
}
