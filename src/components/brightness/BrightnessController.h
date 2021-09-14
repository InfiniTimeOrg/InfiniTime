#pragma once

#include <cstdint>

namespace Pinetime {
  namespace Controllers {
    class BrightnessController {
    public:
      enum class Levels { Off, Low, Medium, High };
      void Init();

      void Set(Levels level);
      Levels Level() const;
      void Lower();
      void Higher();
      void Step();

      void Backup();
      void Restore();

      const char* GetIcon();
      const char* ToString();

    private:
      Levels level = Levels::High;
      Levels backupLevel = Levels::High;
    };
  }
}
