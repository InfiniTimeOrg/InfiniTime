#pragma once

#include <cstdint>
#include <chrono>
#include <Components/Gfx/Gfx.h>
#include "Screen.h"
#include <bits/unique_ptr.h>
#include <libs/lvgl/src/lv_core/lv_style.h>
#include <libs/lvgl/src/lv_core/lv_obj.h>
#include <Components/Battery/BatteryController.h>
#include <Components/Ble/BleController.h>
#include "../../Version.h"
#include <Components/Ble/MusicService.h>
#include <string>

namespace Pinetime {
  namespace Applications {
    namespace Screens {

      class Music : public Screen{
        public:
          Music(DisplayApp* app, Pinetime::Controllers::MusicService &music);
          ~Music() override;

          bool Refresh() override;
          bool OnButtonPushed() override;

          void OnObjectEvent(lv_obj_t* obj, lv_event_t event);

        private:
          lv_obj_t * btnPrev;
          lv_obj_t * btnPlayPause;
          lv_obj_t * btnNext;
          lv_obj_t * btnVolDown;
          lv_obj_t * btnVolUp;
          lv_obj_t * txtArtist;
          lv_obj_t * txtTrack;
          lv_obj_t * txtPlayPause;

          bool running = true;
          Pinetime::Controllers::MusicService &musicService;
          std::string m_artist;
          std::string m_album;
          std::string m_track;
          unsigned char m_status;
      };
    }
  }
}
