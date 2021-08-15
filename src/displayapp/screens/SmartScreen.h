#pragma once

#include <lvgl/src/lv_core/lv_obj.h>
#include <cstdint>
#include <memory>
#include "Screen.h"
#include "ScreenList.h"


// This screen is meant for simple interaction
// with Smart Home systems like Homeassistant, Domoticz, FHEM
// and similar. The goal is to send commands via mqtt, but 
// this part will be handled by the companion app.
// How the companion app decide to pass the data further, is up 
// to the app. We only design the screen and communication over
// Bluetooth to the companion app

// @TODO 16.08.2021
// It probably will need some kind of controller to run in background and 
// handle the communication with companion app. It will have to hold 
// the mqtt topics (based on the feedback in discord channel).
// Not sure how setting the topic should be handled, I don't think it
// should be done here, in screen. 

namespace Pinetime {
  namespace Applications {
    namespace Screens {

      class SmartScreen : public Screen {
      public:
        SmartScreen(DisplayApp* app);

        ~SmartScreen() override;

        bool Refresh() override;

      private:
        // This screen is pretty simple
        // Should display 3 values as a string, typical sensor data like temperature or air humidity
        // Additionaly it has 3 buttons to send commands, maybe for switching lights, opening garage door
        // or starting automation/scripts on remote server. 

        bool bButton1State;
        bool bButton2State;
        bool bButton3State;

        lv_obj_t* labelTitle;
        lv_obj_t* labelValue1;
        lv_obj_t* labelValue2;
        lv_obj_t* labelValue3;
       

        // Displayed above the buttons, name of the function
        lv_obj_t* labelButton1;
        lv_obj_t* labelButton2;
        lv_obj_t* labelButton3;

        // Displayed in the button, ON/OFF
        lv_obj_t* textButton1;
        lv_obj_t* textButton2;
        lv_obj_t* textButton3;
        lv_obj_t* button1;
        lv_obj_t* button2;
        lv_obj_t* button3;
        lv_obj_t* backgroundLabel;

        // LVGL doesn't handle c++ member function calls too well (cause hidden "this" parameter), thus we need to use static member function
        static void btn_event_cb(lv_obj_t * btn, lv_event_t event);
        void UpdateValues();

      };
    }
  }
}
