#pragma once

namespace Pinetime {
  namespace Logging {
    class Logger {
    public:
      virtual void Init() = 0;
      virtual void Resume() = 0;
    };
  }
}