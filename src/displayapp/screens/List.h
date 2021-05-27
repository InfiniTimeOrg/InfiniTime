#pragma once

#include <lvgl/lvgl.h>
#include <array>
#include "Screen.h"

#define MAXLISTITEMS 4

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class List : public Screen {
      public:
        struct Item {
          const char* icon;
          const char* name;
          void* data;
        };
        
        virtual void OnButtonEvent(lv_obj_t* object, lv_event_t event) = 0;
      
      protected:
        explicit List(Pinetime::Applications::DisplayApp* app, uint8_t screenID, uint8_t numScreens, std::array<Item, MAXLISTITEMS>&
        items);
        
        lv_obj_t* buttons[MAXLISTITEMS];
        lv_obj_t* container1;
        
        void createButtonNr(int i, std::array<Item, MAXLISTITEMS>&
        items);
      
      private:
        
        lv_point_t pageIndicatorBasePoints[2];
        lv_point_t pageIndicatorPoints[2];
        lv_obj_t* pageIndicatorBase;
        lv_obj_t* pageIndicator;
      };
    }
  }
}
