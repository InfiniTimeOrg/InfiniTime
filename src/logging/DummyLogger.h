#pragma once
#include "Logger.h"

namespace Pinetime {
  namespace Logging {
    class DummyLogger : public Logger {
    public:
      void Init() override {
      }
      void Resume() override {
      }
    };
  }
}
