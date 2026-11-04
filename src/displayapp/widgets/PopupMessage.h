#pragma once
#include <lvgl/lvgl.h>

namespace Pinetime::Applications::Widgets {
  class PopupMessage {
  public:
    PopupMessage() = default;
    void Create();
    void SetHidden(bool hidden);
    bool IsHidden() const;
    // public function only to be used by lvgl handler
    void HandleDelete(lv_obj_t* object, lv_event_t event);

  private:
    lv_obj_t* popup = nullptr;
    bool isHidden = true;
  };
}
