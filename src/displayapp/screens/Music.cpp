/*  Copyright (C) 2020 JF, Adam Pigg, Avamander

    This file is part of InfiniTime.

    InfiniTime is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published
    by the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    InfiniTime is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#include "displayapp/screens/Music.h"
#include "displayapp/screens/Symbols.h"
#include <cstdint>
#include "displayapp/DisplayApp.h"
#include "components/ble/MusicService.h"
#include "displayapp/InfiniTimeTheme.h"
#include "components/ble/BleController.h"

using namespace Pinetime::Applications::Screens;

namespace {
  void EventHandler(lv_obj_t* obj, lv_event_t event) {
    auto* screen = static_cast<Music*>(obj->user_data);
    screen->OnObjectEvent(obj, event);
  }

  /**
   * Set the pixel array to display by the image
   * This just calls lv_img_set_src but adds type safety
   *
   * @param img pointer to an image object
   * @param data the image array
   */
  inline void lv_img_set_src_arr(lv_obj_t* img, const lv_img_dsc_t* src_img) {
    lv_img_set_src(img, src_img);
  }
}

/**
 * Music control watchapp
 *
 * TODO: Investigate Apple Media Service and AVRCPv1.6 support for seamless integration
 */
Music::Music(Pinetime::Controllers::MusicService& music, const Controllers::Ble& bleController)
  : musicService(music), bleController {bleController} {

  lv_style_init(&btn_style);
  lv_style_set_radius(&btn_style, LV_STATE_DEFAULT, 20);
  lv_style_set_bg_color(&btn_style, LV_STATE_DEFAULT, Colors::bgAlt);

  btnVolDown = lv_btn_create(lv_scr_act(), nullptr);
  btnVolDown->user_data = this;
  lv_obj_set_event_cb(btnVolDown, EventHandler);
  lv_obj_set_size(btnVolDown, 117, 60);
  lv_obj_align(btnVolDown, nullptr, LV_ALIGN_IN_TOP_LEFT, 0, 0);
  lv_obj_add_style(btnVolDown, LV_STATE_DEFAULT, &btn_style);
  txtVolDown = lv_label_create(btnVolDown, nullptr);
  lv_label_set_text_static(txtVolDown, Symbols::volumDown);

  btnVolUp = lv_btn_create(lv_scr_act(), nullptr);
  btnVolUp->user_data = this;
  lv_obj_set_event_cb(btnVolUp, EventHandler);
  lv_obj_set_size(btnVolUp, 117, 60);
  lv_obj_align(btnVolUp, nullptr, LV_ALIGN_IN_TOP_RIGHT, 0, 0);
  lv_obj_add_style(btnVolUp, LV_STATE_DEFAULT, &btn_style);
  txtVolUp = lv_label_create(btnVolUp, nullptr);
  lv_label_set_text_static(txtVolUp, Symbols::volumUp);

  btnPrev = lv_btn_create(lv_scr_act(), nullptr);
  btnPrev->user_data = this;
  lv_obj_set_event_cb(btnPrev, EventHandler);
  lv_obj_set_size(btnPrev, 76, 76);
  lv_obj_align(btnPrev, nullptr, LV_ALIGN_IN_BOTTOM_LEFT, 3, 0);
  lv_obj_add_style(btnPrev, LV_STATE_DEFAULT, &btn_style);
  txtBtnPrev = lv_label_create(btnPrev, nullptr);
  lv_label_set_text_static(txtBtnPrev, Symbols::stepBackward);

  btnNext = lv_btn_create(lv_scr_act(), nullptr);
  btnNext->user_data = this;
  lv_obj_set_event_cb(btnNext, EventHandler);
  lv_obj_set_size(btnNext, 76, 76);
  lv_obj_align(btnNext, nullptr, LV_ALIGN_IN_BOTTOM_RIGHT, -3, 0);
  lv_obj_add_style(btnNext, LV_STATE_DEFAULT, &btn_style);
  txtBtnNext = lv_label_create(btnNext, nullptr);
  lv_label_set_text_static(txtBtnNext, Symbols::stepForward);

  btnPlayPause = lv_btn_create(lv_scr_act(), nullptr);
  btnPlayPause->user_data = this;
  lv_obj_set_event_cb(btnPlayPause, EventHandler);
  lv_obj_set_size(btnPlayPause, 76, 76);
  lv_obj_align(btnPlayPause, nullptr, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
  lv_obj_add_style(btnPlayPause, LV_STATE_DEFAULT, &btn_style);
  txtPlayPause = lv_label_create(btnPlayPause, nullptr);
  lv_label_set_text_static(txtPlayPause, Symbols::play);

  // I'm using the txtTrack label as the top anchor for the whole lot
  // of song, artist, progress bar and duration text (0:00 and -0:00) so
  // its much easier to move that around and mess with the buttons separately
  constexpr int16_t BASE_Y = -40; // -45 for buttons at the top

  txtTrack = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_long_mode(txtTrack, LV_LABEL_LONG_SROLL_CIRC);
  lv_obj_align(txtTrack, nullptr, LV_ALIGN_IN_LEFT_MID, 0, BASE_Y);
  lv_label_set_align(txtTrack, LV_ALIGN_IN_LEFT_MID);
  lv_obj_set_width(txtTrack, LV_HOR_RES);
  lv_label_set_text_static(txtTrack, "");

  txtArtist = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_long_mode(txtArtist, LV_LABEL_LONG_SROLL_CIRC);
  lv_obj_align(txtArtist, txtTrack, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
  lv_label_set_align(txtArtist, LV_ALIGN_IN_LEFT_MID);
  lv_obj_set_width(txtArtist, LV_HOR_RES);
  lv_label_set_text_static(txtArtist, "");
  lv_obj_set_style_local_text_color(txtArtist, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::lightGray);

  barTrackDuration = lv_bar_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_bg_color(barTrackDuration, LV_BAR_PART_BG, LV_STATE_DEFAULT, Colors::bgAlt);
  lv_obj_set_style_local_bg_color(barTrackDuration, LV_BAR_PART_INDIC, LV_STATE_DEFAULT, Colors::lightGray);
  lv_obj_set_style_local_bg_opa(barTrackDuration, LV_BAR_PART_BG, LV_STATE_DEFAULT, LV_OPA_100);
  lv_obj_set_style_local_radius(barTrackDuration, LV_BAR_PART_BG, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
  lv_obj_set_size(barTrackDuration, 240, 10);
  lv_obj_align(barTrackDuration, txtArtist, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
  lv_bar_set_range(barTrackDuration, 0, 1000);
  lv_bar_set_value(barTrackDuration, 0, LV_ANIM_OFF);

  txtCurrentPosition = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_long_mode(txtCurrentPosition, LV_LABEL_LONG_SROLL);
  lv_obj_align(txtCurrentPosition, barTrackDuration, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
  lv_label_set_text_static(txtCurrentPosition, "--:--");
  lv_label_set_align(txtCurrentPosition, LV_ALIGN_IN_LEFT_MID);
  lv_obj_set_width(txtCurrentPosition, LV_HOR_RES);
  lv_obj_set_style_local_text_color(txtCurrentPosition, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::lightGray);

  txtTrackDuration = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_long_mode(txtTrackDuration, LV_LABEL_LONG_SROLL);
  lv_obj_align(txtTrackDuration, barTrackDuration, LV_ALIGN_OUT_BOTTOM_RIGHT, -13, 0);
  lv_label_set_text_static(txtTrackDuration, "--:--");
  lv_label_set_align(txtTrackDuration, LV_ALIGN_IN_RIGHT_MID);
  lv_obj_set_width(txtTrackDuration, LV_HOR_RES);
  lv_obj_set_style_local_text_color(txtTrackDuration, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::lightGray);

  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
}

Music::~Music() {
  lv_task_del(taskRefresh);
  lv_style_reset(&btn_style);
  lv_obj_clean(lv_scr_act());
}

void Music::Refresh() {
  bleState = bleController.IsConnected();

  if (bleState.IsUpdated()) {
    const bool connected = bleState.Get();

    if (!connected) {
      SetDisconnectedUI();
      return;
    }
    musicService.event(Controllers::MusicService::EVENT_MUSIC_OPEN);
    SetConnectedUI();
    RefreshTrackInfo();
    return;
  }

  if (bleState.Get()) {
    RefreshTrackInfo();
  }
}

void Music::SetDisconnectedUI() {
  lv_label_set_text_static(txtArtist, "Disconnected");
  lv_label_set_text_static(txtTrack, "");

  for (auto* btn : GetButtons()) {
    lv_obj_set_style_local_bg_color(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Colors::bgDark);
  }

  for (auto* lbl : GetButtonLabels()) {
    lv_obj_set_style_local_text_color(lbl, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::bgAlt);
  }

  lv_label_set_text_static(txtCurrentPosition, "--:--");
  lv_label_set_text_static(txtTrackDuration, "--:--");
  lv_bar_set_range(barTrackDuration, 0, 1000);
  lv_bar_set_value(barTrackDuration, 0, LV_ANIM_OFF);

  artist = "";
  track = "";
}

void Music::SetConnectedUI() {
  for (auto* btn : GetButtons()) {
    lv_obj_set_style_local_bg_color(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Colors::bgAlt);
  }

  for (auto* lbl : GetButtonLabels()) {
    lv_obj_set_style_local_text_color(lbl, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  }
}

void Music::RefreshTrackInfo() {
  if (playing != musicService.isPlaying()) {
    playing = musicService.isPlaying();
    lv_label_set_text_static(txtPlayPause, playing ? Symbols::pause : Symbols::play);
  }

  artist = musicService.getArtist();
  if (artist.IsUpdated()) {
    lv_label_set_text(txtArtist, artist.Get().data());
  }

  track = musicService.getTrack();
  if (track.IsUpdated()) {
    lv_label_set_text(txtTrack, track.Get().data());
  }

  if (currentPosition != musicService.getProgress()) {
    currentPosition = musicService.getProgress();
    UpdateLength();
  }

  if (totalLength != musicService.getTrackLength()) {
    totalLength = musicService.getTrackLength();
    UpdateLength();
  }
}

void Music::UpdateLength() {
  int remaining = std::max(totalLength - currentPosition, 0);
  int range = std::min(totalLength > 0 ? totalLength : 1, static_cast<int>(std::numeric_limits<int16_t>::max()));

  if (totalLength > (99 * 60 * 60)) {
    lv_label_set_text_static(txtCurrentPosition, "Inf");
    lv_label_set_text_static(txtTrackDuration, "Inf");
  } else if (totalLength > (99 * 60)) {
    lv_label_set_text_fmt(txtCurrentPosition, "%d:%02d", (currentPosition / (60 * 60)) % 100, ((currentPosition % (60 * 60)) / 60) % 100);
    lv_label_set_text_fmt(txtTrackDuration, "-%d:%02d", (remaining / (60 * 60)) % 100, ((remaining % (60 * 60)) / 60) % 100);
    lv_bar_set_range(barTrackDuration, 0, range);
    lv_bar_set_value(barTrackDuration, std::min(currentPosition, range), LV_ANIM_OFF);
  } else {
    lv_label_set_text_fmt(txtCurrentPosition, "%d:%02d", (currentPosition / 60) % 100, (currentPosition % 60) % 100);
    lv_label_set_text_fmt(txtTrackDuration, "-%d:%02d", (remaining / 60) % 100, (remaining % 60) % 100);
    lv_bar_set_range(barTrackDuration, 0, range);
    lv_bar_set_value(barTrackDuration, std::min(currentPosition, range), LV_ANIM_OFF);
  }
}

void Music::OnObjectEvent(lv_obj_t* obj, lv_event_t event) {
  if (event == LV_EVENT_CLICKED) {
    if (obj == btnVolDown) {
      musicService.event(Controllers::MusicService::EVENT_MUSIC_VOLDOWN);
    } else if (obj == btnVolUp) {
      musicService.event(Controllers::MusicService::EVENT_MUSIC_VOLUP);
    } else if (obj == btnPrev) {
      musicService.event(Controllers::MusicService::EVENT_MUSIC_PREV);
    } else if (obj == btnPlayPause) {
      if (playing) {
        musicService.event(Controllers::MusicService::EVENT_MUSIC_PAUSE);
      } else {
        musicService.event(Controllers::MusicService::EVENT_MUSIC_PLAY);
      }
      // Let's assume it stops/starts playing instantly
      // TODO: In the future should check for BT connection for better UX
      playing = !playing;
    } else if (obj == btnNext) {
      musicService.event(Controllers::MusicService::EVENT_MUSIC_NEXT);
    }
  }
}

bool Music::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  switch (event) {
    case TouchEvents::SwipeLeft: {
      musicService.event(Controllers::MusicService::EVENT_MUSIC_NEXT);
      return true;
    }
    case TouchEvents::SwipeRight: {
      musicService.event(Controllers::MusicService::EVENT_MUSIC_PREV);
      return true;
    }
    default: {
      return false;
    }
  }
}
