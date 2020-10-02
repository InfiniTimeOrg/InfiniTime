#include <libs/lvgl/lvgl.h>
#include "Music.h"

using namespace Pinetime::Applications::Screens;
extern lv_font_t jetbrains_mono_extrabold_compressed;
extern lv_font_t jetbrains_mono_bold_20;

static void event_handler(lv_obj_t * obj, lv_event_t event)
{
  Music* screen = static_cast<Music *>(obj->user_data);
  screen->OnObjectEvent(obj, event);
}

Music::Music(Pinetime::Applications::DisplayApp *app, Pinetime::Controllers::MusicService &music) : Screen(app), musicService(music) {
    lv_obj_t * label;

    btnVolDown = lv_btn_create(lv_scr_act(), NULL);
    btnVolDown->user_data = this;
    lv_obj_set_event_cb(btnVolDown, event_handler);
    lv_obj_align(btnVolDown, NULL, LV_ALIGN_IN_TOP_LEFT, 10, 10);
    label = lv_label_create(btnVolDown, NULL);
    lv_label_set_text(label, "v-");

    btnVolUp = lv_btn_create(lv_scr_act(), NULL);
    btnVolUp->user_data = this;
    lv_obj_set_event_cb(btnVolUp, event_handler);
    lv_obj_align(btnVolUp, NULL, LV_ALIGN_IN_TOP_RIGHT, -10, 10);
    label = lv_label_create(btnVolUp, NULL);
    lv_label_set_text(label, "v+");

    btnPrev = lv_btn_create(lv_scr_act(), NULL);
    btnPrev->user_data = this;
    lv_obj_set_event_cb(btnPrev, event_handler);
    lv_obj_set_size(btnPrev, LV_HOR_RES / 4, LV_VER_RES / 4);
    lv_obj_align(btnPrev, NULL, LV_ALIGN_IN_BOTTOM_LEFT, 10,-10);
    label = lv_label_create(btnPrev, NULL);
    lv_label_set_text(label, "<<");

    btnPlayPause = lv_btn_create(lv_scr_act(), NULL);
    btnPlayPause->user_data = this;
    lv_obj_set_event_cb(btnPlayPause, event_handler);
    lv_obj_set_size(btnPlayPause, LV_HOR_RES / 4, LV_VER_RES / 4);
    lv_obj_align(btnPlayPause, NULL, LV_ALIGN_IN_BOTTOM_MID, 0,-10);
    txtPlayPause = lv_label_create(btnPlayPause, NULL);
    lv_label_set_text(txtPlayPause, ">");

    btnNext = lv_btn_create(lv_scr_act(), NULL);
    btnNext->user_data = this;
    lv_obj_set_event_cb(btnNext, event_handler);
    lv_obj_set_size(btnNext, LV_HOR_RES / 4, LV_VER_RES / 4);
    lv_obj_align(btnNext, NULL, LV_ALIGN_IN_BOTTOM_RIGHT, -10,-10);
    label = lv_label_create(btnNext, NULL);
    lv_label_set_text(label, ">>");

    txtArtist = lv_label_create(lv_scr_act(), NULL);
    lv_label_set_long_mode(txtArtist, LV_LABEL_LONG_SROLL);
    lv_obj_align(txtArtist, NULL, LV_ALIGN_IN_LEFT_MID, 0,-20);
    lv_label_set_text(txtArtist, "Artist Name");
    lv_label_set_align(txtArtist, LV_LABEL_ALIGN_CENTER);
    lv_obj_set_width(txtArtist, LV_HOR_RES);

    txtTrack = lv_label_create(lv_scr_act(), NULL);
    lv_label_set_long_mode(txtTrack, LV_LABEL_LONG_DOT);
    lv_obj_align(txtTrack, NULL, LV_ALIGN_IN_LEFT_MID, 0,20);
    lv_label_set_text(txtTrack, "This is a very long track name");
    lv_label_set_align(txtTrack, LV_LABEL_ALIGN_CENTER);
    lv_obj_set_width(txtTrack, LV_HOR_RES);

    musicService.event(Controllers::MusicService::EVENT_MUSIC_OPEN);
}

Music::~Music() {
  lv_obj_clean(lv_scr_act());
}

bool Music::OnButtonPushed() {
  running = false;
  return true;
}

bool Music::Refresh() {

    if (m_artist != musicService.artist()) {
        m_artist = musicService.artist();
        lv_label_set_text(txtArtist, m_artist.data());
    }
    if (m_track != musicService.track()) {
        m_track = musicService.track();
        lv_label_set_text(txtTrack, m_track.data());
    }
    if (m_album != musicService.album()) {
        m_album = musicService.album();
    }
    if (m_status != musicService.status()) {
        m_status = musicService.status();
    }
    if (m_status == Pinetime::Controllers::MusicService::STATUS_MUSIC_PLAYING) {
        lv_label_set_text(txtPlayPause, "||");
    } else {
        lv_label_set_text(txtPlayPause, ">");
    }

  return running;
}

void Music::OnObjectEvent(lv_obj_t* obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED) {
        if (obj == btnVolDown) {
            musicService.event(Controllers::MusicService::EVENT_MUSIC_VOLDOWN);
        } else if (obj == btnVolUp) {
            musicService.event(Controllers::MusicService::EVENT_MUSIC_VOLUP);
        } else if (obj == btnPrev) {
            musicService.event(Controllers::MusicService::EVENT_MUSIC_PREV);
        } else if (obj == btnPlayPause) {
            if (m_status == Pinetime::Controllers::MusicService::STATUS_MUSIC_PLAYING) {
                musicService.event(Controllers::MusicService::EVENT_MUSIC_PAUSE);
            } else {
                musicService.event(Controllers::MusicService::EVENT_MUSIC_PLAY);
            }
        } else if (obj == btnNext) {
            musicService.event(Controllers::MusicService::EVENT_MUSIC_NEXT);
        }
    }
}
