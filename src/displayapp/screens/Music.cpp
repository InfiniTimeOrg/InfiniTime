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
#include "Music.h"
#include "Symbols.h"
#include <cstdint>
#include "../DisplayApp.h"
#include "components/ble/MusicService.h"
#include "displayapp/icons/music/disc.cpp"
#include "displayapp/icons/music/disc_f_1.cpp"
#include "displayapp/icons/music/disc_f_2.cpp"
#include "lv_i18n/lv_i18n.h"

using namespace Pinetime::Applications::Screens;

static void event_handler(lv_obj_t* obj, lv_event_t event) {
  Music* screen = static_cast<Music*>(obj->user_data);
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

/**
 * Music control watchapp
 *
 * TODO: Investigate Apple Media Service and AVRCPv1.6 support for seamless integration
 */
Music::Music(Pinetime::Applications::DisplayApp* app, Pinetime::Controllers::MusicService& music) : Screen(app), musicService(music) {
  lv_obj_t* label;

  btnVolDown = lv_btn_create(lv_scr_act(), nullptr);
  btnVolDown->user_data = this;
  lv_obj_set_event_cb(btnVolDown, event_handler);
  lv_obj_set_size(btnVolDown, 65, 75);
  lv_obj_align(btnVolDown, nullptr, LV_ALIGN_IN_BOTTOM_LEFT, 15, -10);
  lv_obj_set_style_local_radius(btnVolDown, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 20);
  lv_obj_set_style_local_bg_color(btnVolDown, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_AQUA);
  lv_obj_set_style_local_bg_opa(btnVolDown, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_20);
  label = lv_label_create(btnVolDown, nullptr);
  lv_label_set_text(label, Symbols::volumDown);
  lv_obj_set_hidden(btnVolDown, !displayVolumeButtons);

  btnVolUp = lv_btn_create(lv_scr_act(), nullptr);
  btnVolUp->user_data = this;
  lv_obj_set_event_cb(btnVolUp, event_handler);
  lv_obj_set_size(btnVolUp, 65, 75);
  lv_obj_align(btnVolUp, nullptr, LV_ALIGN_IN_BOTTOM_RIGHT, -15, -10);
  lv_obj_set_style_local_radius(btnVolUp, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 20);
  lv_obj_set_style_local_bg_color(btnVolUp, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_AQUA);
  lv_obj_set_style_local_bg_opa(btnVolUp, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_20);
  label = lv_label_create(btnVolUp, nullptr);
  lv_label_set_text(label, Symbols::volumUp);
  lv_obj_set_hidden(btnVolUp, !displayVolumeButtons);

  btnPrev = lv_btn_create(lv_scr_act(), nullptr);
  btnPrev->user_data = this;
  lv_obj_set_event_cb(btnPrev, event_handler);
  lv_obj_set_size(btnPrev, 65, 75);
  lv_obj_align(btnPrev, nullptr, LV_ALIGN_IN_BOTTOM_LEFT, 15, -10);
  lv_obj_set_style_local_radius(btnPrev, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 20);
  lv_obj_set_style_local_bg_color(btnPrev, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_AQUA);
  lv_obj_set_style_local_bg_opa(btnPrev, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_20);
  label = lv_label_create(btnPrev, nullptr);
  lv_label_set_text(label, Symbols::stepBackward);

  btnNext = lv_btn_create(lv_scr_act(), nullptr);
  btnNext->user_data = this;
  lv_obj_set_event_cb(btnNext, event_handler);
  lv_obj_set_size(btnNext, 65, 75);
  lv_obj_align(btnNext, nullptr, LV_ALIGN_IN_BOTTOM_RIGHT, -15, -10);
  lv_obj_set_style_local_radius(btnNext, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 20);
  lv_obj_set_style_local_bg_color(btnNext, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_AQUA);
  lv_obj_set_style_local_bg_opa(btnNext, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_20);
  label = lv_label_create(btnNext, nullptr);
  lv_label_set_text(label, Symbols::stepForward);

  btnPlayPause = lv_btn_create(lv_scr_act(), nullptr);
  btnPlayPause->user_data = this;
  lv_obj_set_event_cb(btnPlayPause, event_handler);
  lv_obj_set_size(btnPlayPause, 65, 75);
  lv_obj_align(btnPlayPause, nullptr, LV_ALIGN_IN_BOTTOM_MID, 0, -10);
  lv_obj_set_style_local_radius(btnPlayPause, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 20);
  lv_obj_set_style_local_bg_color(btnPlayPause, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_AQUA);
  lv_obj_set_style_local_bg_opa(btnPlayPause, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_20);
  txtPlayPause = lv_label_create(btnPlayPause, nullptr);
  lv_label_set_text(txtPlayPause, Symbols::play);

  txtTrackDuration = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_long_mode(txtTrackDuration, LV_LABEL_LONG_SROLL);
  lv_obj_align(txtTrackDuration, nullptr, LV_ALIGN_IN_TOP_LEFT, 12, 20);
  lv_label_set_text(txtTrackDuration, "--:--/--:--");
  lv_label_set_align(txtTrackDuration, LV_ALIGN_IN_LEFT_MID);
  lv_obj_set_width(txtTrackDuration, LV_HOR_RES);

  constexpr uint8_t FONT_HEIGHT = 12;
  constexpr uint8_t LINE_PAD = 15;
  constexpr int8_t MIDDLE_OFFSET = -25;
  txtArtist = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_long_mode(txtArtist, LV_LABEL_LONG_SROLL_CIRC);
  lv_label_set_anim_speed(txtArtist, 1);
  lv_obj_align(txtArtist, nullptr, LV_ALIGN_IN_LEFT_MID, 12, MIDDLE_OFFSET + 1 * FONT_HEIGHT);
  lv_label_set_align(txtArtist, LV_ALIGN_IN_LEFT_MID);
  lv_obj_set_width(txtArtist, LV_HOR_RES - 12);
  lv_label_set_text(txtArtist, _("music_artist_name"));

  txtTrack = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_long_mode(txtTrack, LV_LABEL_LONG_SROLL_CIRC);
  lv_label_set_anim_speed(txtTrack, 1);
  lv_obj_align(txtTrack, nullptr, LV_ALIGN_IN_LEFT_MID, 12, MIDDLE_OFFSET + 2 * FONT_HEIGHT + LINE_PAD);

  lv_label_set_align(txtTrack, LV_ALIGN_IN_LEFT_MID);
  lv_obj_set_width(txtTrack, LV_HOR_RES - 12);
  lv_label_set_text(txtTrack, _("music_track_name"));

  /** Init animation */
  imgDisc = lv_img_create(lv_scr_act(), nullptr);
  lv_img_set_src_arr(imgDisc, &disc);
  lv_obj_align(imgDisc, nullptr, LV_ALIGN_IN_TOP_RIGHT, -15, 15);

  imgDiscAnim = lv_img_create(lv_scr_act(), nullptr);
  lv_img_set_src_arr(imgDiscAnim, &disc_f_1);
  lv_obj_align(imgDiscAnim, nullptr, LV_ALIGN_IN_TOP_RIGHT, -15 - 32, 15);

  frameB = false;

  musicService.event(Controllers::MusicService::EVENT_MUSIC_OPEN);
}

Music::~Music() {
  lv_obj_clean(lv_scr_act());
}

bool Music::Refresh() {
  if (artist != musicService.getArtist()) {
    artist = musicService.getArtist();
    currentLength = 0;
    lv_label_set_text(txtArtist, artist.data());
  }

  if (track != musicService.getTrack()) {
    track = musicService.getTrack();
    currentLength = 0;
    lv_label_set_text(txtTrack, track.data());
  }

  if (album != musicService.getAlbum()) {
    album = musicService.getAlbum();
    currentLength = 0;
  }

  if (playing != musicService.isPlaying()) {
    playing = musicService.isPlaying();
  }

  // Because we increment this ourselves,
  // we can't compare with the old data directly
  // have to update it when there's actually new data
  // just to avoid unnecessary draws that make UI choppy
  if (lastLength != musicService.getProgress()) {
    currentLength = musicService.getProgress();
    lastLength = currentLength;
    UpdateLength();
  }

  if (totalLength != musicService.getTrackLength()) {
    totalLength = musicService.getTrackLength();
    UpdateLength();
  }

  if (playing == Pinetime::Controllers::MusicService::MusicStatus::Playing) {
    lv_label_set_text(txtPlayPause, Symbols::pause);
    if (xTaskGetTickCount() - 1024 >= lastIncrement) {

      if (frameB) {
        lv_img_set_src(imgDiscAnim, &disc_f_1);
      } else {
        lv_img_set_src(imgDiscAnim, &disc_f_2);
      }
      frameB = !frameB;

      if (currentLength < totalLength) {
        currentLength +=
          static_cast<int>((static_cast<float>(xTaskGetTickCount() - lastIncrement) / 1024.0f) * musicService.getPlaybackSpeed());
      } else {
        // Let's assume the getTrack finished, paused when the timer ends
        //  and there's no new getTrack being sent to us
        // TODO: ideally this would be configurable
        playing = false;
      }
      lastIncrement = xTaskGetTickCount();

      UpdateLength();
    }
  } else {
    lv_label_set_text(txtPlayPause, Symbols::play);
  }

  return running;
}

void Music::UpdateLength() {
  if (totalLength > (99 * 60 * 60)) {
    lv_label_set_text(txtTrackDuration, "Inf/Inf");
  } else if (totalLength > (99 * 60)) {
    char timer[12];
    sprintf(timer,
            "%02d:%02d/%02d:%02d",
            (currentLength / (60 * 60)) % 100,
            ((currentLength % (60 * 60)) / 60) % 100,
            (totalLength / (60 * 60)) % 100,
            ((totalLength % (60 * 60)) / 60) % 100);
    lv_label_set_text(txtTrackDuration, timer);
  } else {
    char timer[12];
    sprintf(timer,
            "%02d:%02d/%02d:%02d",
            (currentLength / 60) % 100,
            (currentLength % 60) % 100,
            (totalLength / 60) % 100,
            (totalLength % 60) % 100);
    lv_label_set_text(txtTrackDuration, timer);
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
      if (playing == Pinetime::Controllers::MusicService::MusicStatus::Playing) {
        musicService.event(Controllers::MusicService::EVENT_MUSIC_PAUSE);

        // Let's assume it stops playing instantly
        playing = Controllers::MusicService::NotPlaying;
      } else {
        musicService.event(Controllers::MusicService::EVENT_MUSIC_PLAY);

        // Let's assume it starts playing instantly
        // TODO: In the future should check for BT connection for better UX
        playing = Controllers::MusicService::Playing;
      }
    } else if (obj == btnNext) {
      musicService.event(Controllers::MusicService::EVENT_MUSIC_NEXT);
    }
  }
}

bool Music::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  switch (event) {
    case TouchEvents::SwipeUp: {
      displayVolumeButtons = true;
      lv_obj_set_hidden(btnVolDown, !displayVolumeButtons);
      lv_obj_set_hidden(btnVolUp, !displayVolumeButtons);

      lv_obj_set_hidden(btnNext, displayVolumeButtons);
      lv_obj_set_hidden(btnPrev, displayVolumeButtons);
      return true;
    }
    case TouchEvents::SwipeDown: {
      displayVolumeButtons = false;
      lv_obj_set_hidden(btnNext, displayVolumeButtons);
      lv_obj_set_hidden(btnPrev, displayVolumeButtons);

      lv_obj_set_hidden(btnVolDown, !displayVolumeButtons);
      lv_obj_set_hidden(btnVolUp, !displayVolumeButtons);
      return true;
    }
    case TouchEvents::SwipeLeft: {
      musicService.event(Controllers::MusicService::EVENT_MUSIC_NEXT);
      return true;
    }
    case TouchEvents::SwipeRight: {
      musicService.event(Controllers::MusicService::EVENT_MUSIC_PREV);
      return true;
    }
    default: {
      return true;
    }
  }
}
