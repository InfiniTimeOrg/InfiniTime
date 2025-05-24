#pragma once
#include <lvgl/lvgl.h>

namespace Pinetime::Applications::Widgets {
  class PopupMessage {
  public:
    PopupMessage() = default;
    void Create();
    void SetHidden(bool hidden);
    bool IsHidden() const;

  private:
    lv_obj_t* popup = nullptr;
    bool isHidden = true;
  };
}
