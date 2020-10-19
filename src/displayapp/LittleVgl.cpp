/*  Copyright (C) 2020 JF, Avamander

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
#include <FreeRTOS.h>
#include <projdefs.h>
#include <task.h>
#include <libs/lvgl/src/lv_core/lv_obj.h>
#include <hal/nrf_rtc.h>
#include <libraries/log/nrf_log.h>

#include <libs/lvgl/src/lv_themes/lv_theme.h>
//#include <libs/lvgl/src/lv_themes/lv_theme_night.h>

#include "LittleVgl.h"

using namespace Pinetime::Components;

extern "C" {
LV_FONT_DECLARE(jetbrains_mono_extrabold_compressed)
LV_FONT_DECLARE(jetbrains_mono_bold_20)
}

lv_style_t* LabelBigStyle = nullptr;

static void disp_flush(lv_disp_drv_t* disp_drv, const lv_area_t* area, lv_color_t* color_p) {
  auto* lvgl = static_cast<LittleVgl*>(disp_drv->user_data);
  lvgl->FlushDisplay(area, color_p);
}

bool touchpad_read(lv_indev_drv_t* indev_drv, lv_indev_data_t* data) {
  auto* lvgl = static_cast<LittleVgl*>(indev_drv->user_data);
  return lvgl->GetTouchPadInfo(data);
}

LittleVgl::LittleVgl(Pinetime::Drivers::St7789& lcd, Pinetime::Drivers::Cst816S& touchPanel) : lcd{lcd}, touchPanel{touchPanel},
                                                                                               previousClick{0, 0} {
  lv_init();
  InitTheme();
  InitDisplay();
  InitTouchpad();
}

void LittleVgl::InitDisplay() {
  lv_disp_buf_init(&disp_buf_2, buf2_1, buf2_2, LV_HOR_RES_MAX * 4);   /*Initialize the display buffer*/
  lv_disp_drv_init(&disp_drv);                    /*Basic initialization*/
  
  /*Set up the functions to access to your display*/
  
  /*Set the resolution of the display*/
  disp_drv.hor_res = 240;
  disp_drv.ver_res = 240;
  
  /*Used to copy the buffer's content to the display*/
  disp_drv.flush_cb = disp_flush;
  /*Set a display buffer*/
  disp_drv.buffer = &disp_buf_2;
  disp_drv.user_data = this;
  
  /*Finally register the driver*/
  lv_disp_drv_register(&disp_drv);
}

void LittleVgl::InitTouchpad() {
  lv_indev_drv_t indev_drv;
  
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = touchpad_read;
  indev_drv.user_data = this;
  lv_indev_drv_register(&indev_drv);
}

void LittleVgl::SetFullRefresh(FullRefreshDirections direction) {
  if (scrollDirection == FullRefreshDirections::None) {
    scrollDirection = direction;
    if (scrollDirection == FullRefreshDirections::Down) {
      lv_disp_set_direction(lv_disp_get_default(), 1);
    }
  }
}

void LittleVgl::FlushDisplay(const lv_area_t* area, lv_color_t* color_p) {
  ulTaskNotifyTake(pdTRUE, 500);
  // Notification is still needed (even if there is a mutex on SPI)
  // because the DataCommand pin cannot be set/clear during a transfer.
  
  // TODO refactore and remove duplicated code
  
  uint16_t x, y, y1, y2, width, height = 0;
  if (scrollDirection == LittleVgl::FullRefreshDirections::Down) {
    if (area->y2 == visibleNbLines - 1) {
      writeOffset = ((writeOffset + totalNbLines) - visibleNbLines) % totalNbLines;
    }
    x = area->x1;
    width = (area->x2 - area->x1) + 1;
    
    y1 = (area->y1 + writeOffset) % totalNbLines;
    y2 = (area->y2 + writeOffset) % totalNbLines;
    y = y1;
    height = (y2 - y1) + 1;
    
    if (area->y2 < visibleNbLines - 1) {
      uint16_t toScroll = 0;
      if (area->y1 == 0) {
        toScroll = height * 2;
        scrollDirection = FullRefreshDirections::None;
        lv_disp_set_direction(lv_disp_get_default(), 0);
      } else {
        toScroll = height;
      }
      
      if (scrollOffset >= toScroll) {
        scrollOffset -= toScroll;
      } else {
        toScroll -= scrollOffset;
        scrollOffset = (totalNbLines) - toScroll;
      }
      
      lcd.VerticalScrollDefinition(0, 320, 0);
      lcd.VerticalScrollStartAddress(scrollOffset);
    }
    
    lcd.BeginDrawBuffer(x, y, width, height);
    lcd.NextDrawBuffer(reinterpret_cast<const uint8_t*>(color_p), width * height * 2);
    
  } else if (scrollDirection == FullRefreshDirections::Up) {
    if (area->y1 == 0) {
      writeOffset = (writeOffset + visibleNbLines) % totalNbLines;
    }
    
    x = area->x1;
    width = (area->x2 - area->x1) + 1;
    
    y1 = (area->y1 + writeOffset) % totalNbLines;
    y2 = (area->y2 + writeOffset) % totalNbLines;
    y = y1;
    height = (y2 - y1) + 1;
    
    if (area->y1 > 0) {
      if (area->y2 == visibleNbLines - 1) {
        scrollOffset += (height * 2);
        scrollDirection = FullRefreshDirections::None;
        lv_disp_set_direction(lv_disp_get_default(), 0);
      } else {
        scrollOffset += height;
      }
      scrollOffset = scrollOffset % totalNbLines;
      lcd.VerticalScrollDefinition(0, 320, 0);
      lcd.VerticalScrollStartAddress(scrollOffset);
    }
    
    lcd.BeginDrawBuffer(x, y, width, height);
    lcd.NextDrawBuffer(reinterpret_cast<const uint8_t*>(color_p), width * height * 2);
  } else {
    x = area->x1;
    width = (area->x2 - area->x1) + 1;
    y1 = (area->y1 + writeOffset) % totalNbLines;
    y2 = (area->y2 + writeOffset) % totalNbLines;
    y = y1;
    height = (y2 - y1) + 1;
    
    if (y2 < y1) {
      height = (totalNbLines - 1) - y1;
      lcd.BeginDrawBuffer(x, y1, width, height);
      lcd.NextDrawBuffer(reinterpret_cast<const uint8_t*>(color_p), width * height * 2);
      ulTaskNotifyTake(pdTRUE, 500);
      height = y2;
      lcd.BeginDrawBuffer(x, 0, width, height);
      lcd.NextDrawBuffer(reinterpret_cast<const uint8_t*>(color_p), width * height * 2);
    } else {
      lcd.BeginDrawBuffer(x, y, width, height);
      lcd.NextDrawBuffer(reinterpret_cast<const uint8_t*>(color_p), width * height * 2);
    }
  }
  
  /* IMPORTANT!!!
   * Inform the graphics library that you are ready with the flushing*/
  lv_disp_flush_ready(&disp_drv);
}

void LittleVgl::SetNewTapEvent(uint16_t x, uint16_t y) {
  tap_x = x;
  tap_y = y;
  tapped = true;
}

bool LittleVgl::GetTouchPadInfo(lv_indev_data_t* ptr) {
  if (tapped) {
    ptr->point.x = tap_x;
    ptr->point.y = tap_y;
    ptr->state = LV_INDEV_STATE_PR;
    tapped = false;
  } else {
    ptr->state = LV_INDEV_STATE_REL;
  }
  return false;
  /*
  auto info = touchPanel.GetTouchInfo();

  if((previousClick.x != info.x || previousClick.y != info.y) &&
          (info.gesture == Drivers::Cst816S::Gestures::SingleTap)) {
    // TODO For an unknown reason, the first touch is taken twice into account.
    // 'firstTouch' is a quite'n'dirty workaound until I find a better solution
    if(firstTouch) ptr->state = LV_INDEV_STATE_REL;
    else ptr->state = LV_INDEV_STATE_PR;
    firstTouch = false;
    previousClick.x = info.x;
    previousClick.y = info.y;
  }
  else {
    ptr->state = LV_INDEV_STATE_REL;
  }

  ptr->point.x = info.x;
  ptr->point.y = info.y;
  return false;
   */
}

void LittleVgl::InitTheme() {
  uint16_t i;
  auto** style_p = (lv_style_t**) &theme.style;
  for (i = 0; i < LV_THEME_STYLE_COUNT; i++) {
    *style_p = &def;
    style_p++;
  }
  
  InitBaseTheme();
  InitThemeContainer();
  InitThemeButton();
  InitThemeLabel();
  InitThemeLine();
  InitThemeLed();
  InitThemeImage();
  InitThemeBar();
  InitThemeSlider();
  InitThemeSwitch();
  InitThemeMeter();
  InitThemeGauge();
  InitThemeArc();
  InitThemePreload();
  InitThemeChart();
  InitThemeCalendar();
  InitThemeCheckBox();
  InitThemeButtonMatrix();
  InitThemeKnob();
  InitThemeMessageBox();
  InitThemePage();
  InitThemeTextArea();
  InitThemeSpinBox();
  InitThemeList();
  InitThemeDropDownList();
  InitThemeRoller();
  InitThemeTabView();
  InitThemeTileView();
  InitThemeTable();
  InitThemeWindow();
  
  lv_theme_set_act(&theme);
}

void LittleVgl::InitBaseTheme() {
  if (font == nullptr) { font = &jetbrains_mono_bold_20; }
  lv_style_copy(&def, &lv_style_plain); /*Initialize the default style*/
  lv_style_set_text_font(&def, LV_STATE_DEFAULT, font);
  
  lv_style_copy(&bg, &lv_style_plain);
  lv_style_set_value_color(&bg, LV_STATE_DEFAULT, LV_COLOR_BLACK);
  lv_style_set_bg_grad_color(&bg, LV_STATE_DEFAULT, LV_COLOR_BLACK);
  lv_style_set_text_color(&bg, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_style_set_text_font(&bg, LV_STATE_DEFAULT, font);
  lv_style_set_image_recolor(&bg, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  
  lv_style_copy(&scr, &bg);
  lv_style_set_pad_bottom(&scr, LV_STATE_DEFAULT, 0);
  lv_style_set_pad_top(&scr, LV_STATE_DEFAULT, 0);
  lv_style_set_pad_left(&scr, LV_STATE_DEFAULT, 0);
  lv_style_set_pad_right(&scr, LV_STATE_DEFAULT, 0);
  
  lv_style_copy(&sb, &def);
  lv_style_set_value_color(&sb, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 30, 60));
  lv_style_set_bg_grad_color(&sb, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 30, 60));
  lv_style_set_border_width(&sb, LV_STATE_DEFAULT, 0);
  lv_style_set_pad_inner(&sb, LV_STATE_DEFAULT, LV_DPI / 20);
  lv_style_set_pad_left(&sb, LV_STATE_DEFAULT, 0);
  lv_style_set_pad_right(&sb, LV_STATE_DEFAULT, 0);
  lv_style_set_pad_top(&sb, LV_STATE_DEFAULT, 0);
  lv_style_set_pad_bottom(&sb, LV_STATE_DEFAULT, 0);
  sb.body.radius = LV_DPI / 30;
  lv_style_set_image_opa(&sb, LV_STATE_DEFAULT, LV_OPA_COVER);
  
  lv_style_copy(&panel, &bg);
  
  lv_style_set_bg_opa(&panel, LV_STATE_DEFAULT, LV_OPA_COVER);
  
  lv_style_set_value_color(&panel, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 11, 18));
  lv_style_set_bg_color(&panel, LV_STATE_DEFAULT, LV_COLOR_SILVER);
  lv_style_set_bg_grad_color(&panel, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 11, 18));
  lv_style_set_bg_grad_dir(&panel, LV_STATE_DEFAULT, LV_GRAD_DIR_VER);
  
  lv_style_set_radius(&panel, LV_STATE_DEFAULT, LV_DPI / 20);
  lv_style_set_pad_all(&panel, LV_STATE_DEFAULT, LV_DPI / 10);
  
  lv_style_set_border_color(&panel, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 10, 25));
  lv_style_set_border_width(&panel, LV_STATE_DEFAULT, 1);
  lv_style_set_border_opa(&panel, LV_STATE_DEFAULT, LV_OPA_COVER);
  
  lv_style_set_line_color(&panel, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 20, 40));
  lv_style_set_line_width(&panel, LV_STATE_DEFAULT, 1);
  
  
  theme.style.scr = &scr;
  theme.style.bg = &bg;
  theme.style.panel = &def;
}

void LittleVgl::InitThemeContainer() {
  theme.style.cont = &panel;
}

void LittleVgl::InitThemeButton() {
  
  
  lv_style_copy(&btn_rel, &def);
  btn_lv_style_set_value_color(&rel, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 10, 40);
  lv_style_set_bg_grad_color(&btn_rel, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 10, 20));
  lv_style_set_border_color(&btn_rel, LV_STATE_DEFAULT, lv_color_hex3(0x111));
  lv_style_set_border_width(&btn_rel, LV_STATE_DEFAULT, 1);
  lv_style_set_border_opa(&btn_rel, LV_STATE_DEFAULT, LV_OPA_70);
  lv_style_set_pad_left(&btn_rel, LV_STATE_DEFAULT, LV_DPI / 4);
  lv_style_set_pad_right(&btn_rel, LV_STATE_DEFAULT, LV_DPI / 4);
  lv_style_set_pad_top(&btn_rel, LV_STATE_DEFAULT, LV_DPI / 8);
  lv_style_set_pad_bottom(&btn_rel, LV_STATE_DEFAULT, LV_DPI / 8);
  btn_rel.body.shadow.type = LV_SHADOW_BOTTOM;
  lv_style_set_shadow_color(&btn_rel, LV_STATE_DEFAULT, lv_color_hex3(0x111));
  lv_style_set_shadow_width(&btn_rel, LV_STATE_DEFAULT, LV_DPI / 30);
  lv_style_set_text_color(&btn_rel, LV_STATE_DEFAULT, lv_color_hex3(0xeee));
  lv_style_set_image_recolor(&btn_rel, LV_STATE_DEFAULT, lv_color_hex3(0xeee));
  
  lv_style_copy(&btn_pr, &btn_rel);
  lv_style_set_value_color(&btn_pr, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 10, 30));
  lv_style_set_bg_grad_color(&btn_pr, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 10, 10));
  
  lv_style_copy(&btn_tgl_rel, &btn_rel);
  lv_style_set_value_color(&btn_tgl_rel, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 10, 20));
  lv_style_set_bg_grad_color(&btn_tgl_rel, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 10, 40));
  lv_style_set_shadow_width(&btn_tgl_rel, LV_STATE_DEFAULT, LV_DPI / 40);
  lv_style_set_text_color(&btn_tgl_rel, LV_STATE_DEFAULT, lv_color_hex3(0xddd));
  lv_style_set_image_recolor(&btn_tgl_rel, LV_STATE_DEFAULT, lv_color_hex3(0xddd));
  
  lv_style_copy(&btn_tgl_pr, &btn_rel);
  lv_style_set_value_color(&btn_tgl_pr, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 10, 10));
  lv_style_set_bg_grad_color(&btn_tgl_pr, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 10, 30));
  lv_style_set_shadow_width(&btn_tgl_pr, LV_STATE_DEFAULT, LV_DPI / 30);
  lv_style_set_text_color(&btn_tgl_pr, LV_STATE_DEFAULT, lv_color_hex3(0xddd));
  lv_style_set_image_recolor(&btn_tgl_pr, LV_STATE_DEFAULT, lv_color_hex3(0xddd));
  
  lv_style_copy(&btn_ina, &btn_rel);
  lv_style_set_value_color(&btn_ina, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 10, 20));
  lv_style_set_bg_grad_color(&btn_ina, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 10, 20));
  lv_style_set_shadow_width(&btn_ina, LV_STATE_DEFAULT, 0);
  lv_style_set_text_color(&btn_ina, LV_STATE_DEFAULT, lv_color_hex3(0xaaa));
  lv_style_set_image_recolor(&btn_ina, LV_STATE_DEFAULT, lv_color_hex3(0xaaa));
  
  theme.style.btn.rel = &btn_rel;
  theme.style.btn.pr = &btn_pr;
  theme.style.btn.tgl_rel = &btn_tgl_rel;
  theme.style.btn.tgl_pr = &btn_tgl_pr;
  theme.style.btn.ina = &btn_ina;
}

void LittleVgl::InitThemeLabel() {
  lv_style_copy(&prim, &bg);
  lv_style_set_text_color(&prim, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 5, 95));
  
  lv_style_copy(&labelBigStyle, &prim);
  lv_style_set_text_font(&labelBigStyle, LV_STATE_DEFAULT, &jetbrains_mono_extrabold_compressed);
  LabelBigStyle = &(this->labelBigStyle);
  
  lv_style_copy(&sec, &bg);
  lv_style_set_text_color(&sec, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 15, 65));
  
  lv_style_copy(&hint, &bg);
  lv_style_set_text_color(&hint, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 20, 55));
  
  theme.style.label.prim = &prim;
  theme.style.label.sec = &sec;
  theme.style.label.hint = &hint;
}

void LittleVgl::InitThemeLine() {
  theme.style.line.decor = &def;
}

void LittleVgl::InitThemeLed() {
  lv_style_copy(&led, &def);
  lv_style_set_shadow_width(&led, LV_STATE_DEFAULT, LV_DPI / 10);
  lv_style_set_radius(&led, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
  lv_style_set_border_width(&led, LV_STATE_DEFAULT, LV_DPI / 30);
  lv_style_set_border_opa(&led, LV_STATE_DEFAULT, LV_OPA_30);
  lv_style_set_value_color(&led, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 100, 100));
  lv_style_set_bg_grad_color(&led, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 100, 40));
  lv_style_set_border_color(&led, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 60, 60));
  lv_style_set_shadow_color(&led, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 100, 100));
  
  theme.style.led = &led;
}

void LittleVgl::InitThemeImage() {
  theme.style.img.light = &def;
  theme.style.img.dark = &def;
}

void LittleVgl::InitThemeBar() {
  lv_style_copy(&bar_bg, &panel);
  lv_style_set_pad_left(&bar_bg, LV_STATE_DEFAULT, LV_DPI / 16);
  lv_style_set_pad_right(&bar_bg, LV_STATE_DEFAULT, LV_DPI / 16);
  lv_style_set_pad_top(&bar_bg, LV_STATE_DEFAULT, LV_DPI / 16);
  lv_style_set_pad_bottom(&bar_bg, LV_STATE_DEFAULT, LV_DPI / 16);
  lv_style_set_radius(&bar_bg, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
  
  lv_style_copy(&bar_indic, &def);
  lv_style_set_value_color(&bar_indic, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 80, 70));
  lv_style_set_bg_grad_color(&bar_indic, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 80, 70));
  lv_style_set_border_color(&bar_indic, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 20, 15));
  lv_style_set_border_width(&bar_indic, LV_STATE_DEFAULT, 1);
  lv_style_set_border_opa(&bar_indic, LV_STATE_DEFAULT, LV_OPA_COVER);
  lv_style_set_radius(&bar_indic, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
  lv_style_set_pad_left(&bar_indic, LV_STATE_DEFAULT, 0);
  lv_style_set_pad_right(&bar_indic, LV_STATE_DEFAULT, 0);
  lv_style_set_pad_top(&bar_indic, LV_STATE_DEFAULT, 0);
  lv_style_set_pad_bottom(&bar_indic, LV_STATE_DEFAULT, 0);
  
  theme.style.bar.bg = &bar_bg;
  theme.style.bar.indic = &bar_indic;
}

void LittleVgl::InitThemeSlider() {
  lv_style_copy(&slider_knob, theme.style.btn.rel);
  lv_style_set_radius(&slider_knob, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
  
  theme.style.slider.bg = theme.style.bar.bg;
  theme.style.slider.indic = theme.style.bar.indic;
  theme.style.slider.knob = &slider_knob;
}

void LittleVgl::InitThemeSwitch() {
  theme.style.sw.bg = theme.style.bar.bg;
  theme.style.sw.indic = theme.style.bar.indic;
  theme.style.sw.knob_off = theme.style.slider.knob;
  theme.style.sw.knob_on = theme.style.slider.knob;
}

void LittleVgl::InitThemeMeter() {
  static lv_style_t lmeter_bg;
  lv_style_copy(&lmeter_bg, &def);
  lv_style_set_value_color(&lmeter_bg, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 10, 70));
  lv_style_set_bg_grad_color(&lmeter_bg, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 95, 90));
  lv_style_set_pad_left(&lmeter_bg, LV_STATE_DEFAULT, LV_DPI / 10); /*Scale line length*/
  lv_style_set_pad_inner(&lmeter_bg, LV_STATE_DEFAULT, LV_DPI / 10); /*Text padding*/
  lv_style_set_border_color(&lmeter_bg, LV_STATE_DEFAULT, lv_color_hex3(0x333));
  lv_style_set_line_color(&lmeter_bg, LV_STATE_DEFAULT, lv_color_hex3(0x555));
  lv_style_set_line_width(&lmeter_bg, LV_STATE_DEFAULT, 1);
  lv_style_set_text_color(&lmeter_bg, LV_STATE_DEFAULT, lv_color_hex3(0xddd));
  
  theme.style.lmeter = &lmeter_bg;
}

void LittleVgl::InitThemeGauge() {
  static lv_style_t gauge_bg;
  lv_style_copy(&gauge_bg, &def);
  lv_style_set_value_color(&gauge_bg, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 10, 70));
  lv_style_set_bg_grad_color(&gauge_bg, LV_STATE_DEFAULT, gauge_bg.body.main_color);
  lv_style_set_line_color(&gauge_bg, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 80, 75));
  lv_style_set_line_width(&gauge_bg, LV_STATE_DEFAULT, 1);
  lv_style_set_text_color(&gauge_bg, LV_STATE_DEFAULT, lv_color_hex3(0xddd));
  
  theme.style.gauge = &gauge_bg;
}

void LittleVgl::InitThemeArc() {
  lv_style_copy(&arc, &def);
  lv_style_set_line_width(&arc, LV_STATE_DEFAULT, 8);
  lv_style_set_line_color(&arc, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 80, 70));
  lv_style_set_line_rounded(&arc, LV_STATE_DEFAULT, true);
  
  /*For preloader*/
  lv_style_set_border_width(&arc, LV_STATE_DEFAULT, 7);
  lv_style_set_border_color(&arc, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 11, 48));
  lv_style_set_pad_left(&arc, LV_STATE_DEFAULT, 1);
  lv_style_set_pad_right(&arc, LV_STATE_DEFAULT, 1);
  lv_style_set_pad_top(&arc, LV_STATE_DEFAULT, 1);
  lv_style_set_pad_bottom(&arc, LV_STATE_DEFAULT, 1);
  
  theme.style.arc = &arc;
}

void LittleVgl::InitThemePreload() {
//  theme.style.preload = theme.style.arc;
}

void LittleVgl::InitThemeChart() {
  theme.style.chart = &panel;
}

void LittleVgl::InitThemeCalendar() {
  
  lv_style_copy(&cal_bg, &bg);
  lv_style_set_value_color(&cal_bg, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 10, 40));
  lv_style_set_bg_grad_color(&cal_bg, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 10, 40));
  lv_style_set_border_color(&cal_bg, LV_STATE_DEFAULT, lv_color_hex3(0x333));
  lv_style_set_border_width(&cal_bg, LV_STATE_DEFAULT, 1);
  lv_style_set_radius(&cal_bg, LV_STATE_DEFAULT, LV_DPI / 20);
  lv_style_set_pad_left(&cal_bg, LV_STATE_DEFAULT, LV_DPI / 10);
  lv_style_set_pad_right(&cal_bg, LV_STATE_DEFAULT, LV_DPI / 10);
  lv_style_set_pad_top(&cal_bg, LV_STATE_DEFAULT, LV_DPI / 10);
  lv_style_set_pad_bottom(&cal_bg, LV_STATE_DEFAULT, LV_DPI / 10);
  
  
  lv_style_copy(&cal_header, &bg);
  lv_style_set_value_color(&cal_header, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 10, 20));
  lv_style_set_bg_grad_color(&cal_header, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 10, 20));
  lv_style_set_radius(&cal_header, LV_STATE_DEFAULT, 0);
  lv_style_set_border_width(&cal_header, LV_STATE_DEFAULT, 1);
  lv_style_set_border_color(&cal_header, LV_STATE_DEFAULT, lv_color_hex3(0x333));
  lv_style_set_pad_left(&cal_header, LV_STATE_DEFAULT, LV_DPI / 10);
  lv_style_set_pad_right(&cal_header, LV_STATE_DEFAULT, LV_DPI / 10);
  lv_style_set_pad_top(&cal_header, LV_STATE_DEFAULT, LV_DPI / 10);
  lv_style_set_pad_bottom(&cal_header, LV_STATE_DEFAULT, LV_DPI / 10);
  
  
  lv_style_copy(&week_box, &panel);
  lv_style_set_value_color(&week_box, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 30, 45));
  lv_style_set_bg_grad_color(&week_box, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 30, 45));
  lv_style_set_radius(&week_box, LV_STATE_DEFAULT, LV_DPI / 20);
  lv_style_set_border_width(&week_box, LV_STATE_DEFAULT, 1);
  lv_style_set_pad_left(&week_box, LV_STATE_DEFAULT, LV_DPI / 20);
  lv_style_set_pad_right(&week_box, LV_STATE_DEFAULT, LV_DPI / 20);
  lv_style_set_pad_top(&week_box, LV_STATE_DEFAULT, LV_DPI / 25);
  lv_style_set_pad_bottom(&week_box, LV_STATE_DEFAULT, LV_DPI / 25);
  
  lv_style_copy(&today_box, &week_box);
  lv_style_set_value_color(&today_box, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 80, 70));
  lv_style_set_bg_grad_color(&today_box, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 80, 70));
  lv_style_set_radius(&today_box, LV_STATE_DEFAULT, LV_DPI / 20);
  lv_style_set_pad_left(&today_box, LV_STATE_DEFAULT, LV_DPI / 14);
  lv_style_set_pad_right(&today_box, LV_STATE_DEFAULT, LV_DPI / 14);
  lv_style_set_pad_top(&today_box, LV_STATE_DEFAULT, LV_DPI / 14);
  lv_style_set_pad_bottom(&today_box, LV_STATE_DEFAULT, LV_DPI / 14);
  
  lv_style_copy(&highlighted_days, &bg);
  lv_style_set_text_color(&highlighted_days, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 40, 80));
  
  lv_style_copy(&ina_days, &bg);
  lv_style_set_text_color(&ina_days, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 0, 60));
  
  theme.style.calendar.bg = &cal_bg;
  theme.style.calendar.header = &cal_header;
  theme.style.calendar.week_box = &week_box;
  theme.style.calendar.today_box = &today_box;
  theme.style.calendar.highlighted_days = &highlighted_days;
  theme.style.calendar.day_names = &cal_bg;
  theme.style.calendar.inactive_days = &ina_days;
}

void LittleVgl::InitThemeCheckBox() {
  
  lv_style_copy(&rel, &def);
  lv_style_set_radius(&rel, LV_STATE_DEFAULT, LV_DPI / 20);
  lv_style_set_value_color(&rel, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 10, 95));
  lv_style_set_bg_grad_color(&rel, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 10, 95));
  lv_style_set_border_color(&rel, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 10, 50));
  lv_style_set_border_width(&rel, LV_STATE_DEFAULT, 2);
  
  lv_style_copy(&pr, &rel);
  lv_style_set_value_color(&pr, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 10, 80));
  lv_style_set_bg_grad_color(&pr, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 10, 80));
  lv_style_set_border_color(&pr, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 10, 20));
  lv_style_set_border_width(&pr, LV_STATE_DEFAULT, 1);
  
  lv_style_copy(&tgl_rel, &rel);
  lv_style_set_value_color(&tgl_rel, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 80, 90));
  lv_style_set_bg_grad_color(&tgl_rel, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 80, 90));
  lv_style_set_border_color(&tgl_rel, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 80, 50));
  
  lv_style_copy(&tgl_pr, &tgl_rel);
  lv_style_set_value_color(&tgl_pr, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 80, 70));
  lv_style_set_bg_grad_color(&tgl_pr, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 80, 70));
  lv_style_set_border_color(&tgl_pr, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 80, 30));
  lv_style_set_border_width(&tgl_pr, LV_STATE_DEFAULT, 1);
  
  lv_style_copy(&ina, &rel);
  lv_style_set_value_color(&ina, LV_STATE_DEFAULT, lv_color_hex3(0x777));
  lv_style_set_bg_grad_color(&ina, LV_STATE_DEFAULT, lv_color_hex3(0x777));
  lv_style_set_border_width(&ina, LV_STATE_DEFAULT, 0);
  
  theme.style.cb.bg = &lv_style_transp;
  theme.style.cb.box.rel = &rel;
  theme.style.cb.box.pr = &pr;
  theme.style.cb.box.tgl_rel = &tgl_rel;
  theme.style.cb.box.tgl_pr = &tgl_pr;
  theme.style.cb.box.ina = &def;
}

void LittleVgl::InitThemeButtonMatrix() {
  
  lv_style_copy(&btnm_bg, theme.style.btn.rel);
  lv_style_set_pad_left(&btnm_bg, LV_STATE_DEFAULT, 2);
  lv_style_set_pad_right(&btnm_bg, LV_STATE_DEFAULT, 2);
  lv_style_set_pad_top(&btnm_bg, LV_STATE_DEFAULT, 2);
  lv_style_set_pad_bottom(&btnm_bg, LV_STATE_DEFAULT, 2);
  lv_style_set_pad_inner(&btnm_bg, LV_STATE_DEFAULT, 0);
  lv_style_set_border_width(&btnm_bg, LV_STATE_DEFAULT, 1);
  
  lv_style_copy(&btnm_rel, theme.style.btn.rel);
  btnm_rel.body.border.part = LV_BORDER_FULL | LV_BORDER_INTERNAL;
  lv_style_set_border_width(&btnm_rel, LV_STATE_DEFAULT, 1);
  lv_style_set_radius(&btnm_rel, LV_STATE_DEFAULT, 2);
  
  lv_style_copy(&btnm_pr, theme.style.btn.pr);
  btnm_pr.body.border.part = btnm_rel.body.border.part;
  lv_style_set_border_width(&btnm_pr, LV_STATE_DEFAULT, btnm_rel.body.border.width);
  lv_style_set_radius(&btnm_pr, LV_STATE_DEFAULT, btnm_rel.body.radius);
  
  lv_style_copy(&btnm_tgl_rel, theme.style.btn.tgl_rel);
  btnm_tgl_rel.body.border.part = btnm_rel.body.border.part;
  lv_style_set_border_width(&btnm_tgl_rel, LV_STATE_DEFAULT, btnm_rel.body.border.width);
  lv_style_set_radius(&btnm_tgl_rel, LV_STATE_DEFAULT, btnm_rel.body.radius);
  
  lv_style_copy(&btnm_tgl_pr, theme.style.btn.pr);
  btnm_tgl_pr.body.border.part = btnm_rel.body.border.part;
  lv_style_set_border_width(&btnm_tgl_pr, LV_STATE_DEFAULT, btnm_rel.body.border.width);
  lv_style_set_radius(&btnm_tgl_pr, LV_STATE_DEFAULT, btnm_rel.body.radius);
  
  lv_style_copy(&btnm_ina, theme.style.btn.ina);
  btnm_ina.body.border.part = btnm_rel.body.border.part;
  lv_style_set_border_width(&btnm_ina, LV_STATE_DEFAULT, btnm_rel.body.border.width);
  lv_style_set_radius(&btnm_ina, LV_STATE_DEFAULT, btnm_rel.body.radius);
  
  theme.style.btnm.bg = &btnm_bg;
  theme.style.btnm.btn.rel = &btnm_rel;
  theme.style.btnm.btn.pr = &btnm_pr;
  theme.style.btnm.btn.tgl_rel = &btnm_tgl_rel;
  theme.style.btnm.btn.tgl_pr = &btnm_tgl_pr;
  theme.style.btnm.btn.ina = &btnm_ina;
}

void LittleVgl::InitThemeKnob() {
  theme.style.kb.bg = &bg;
  theme.style.kb.btn.rel = theme.style.btn.rel;
  theme.style.kb.btn.pr = theme.style.btn.pr;
  theme.style.kb.btn.tgl_rel = theme.style.btn.tgl_rel;
  theme.style.kb.btn.tgl_pr = theme.style.btn.tgl_pr;
  theme.style.kb.btn.ina = theme.style.btn.ina;
}

void LittleVgl::InitThemeMessageBox() {
  lv_style_copy(&mbox_bg, &bg);
  lv_style_set_value_color(&mbox_bg, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 30, 30));
  lv_style_set_bg_grad_color(&mbox_bg, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 30, 30));
  lv_style_set_border_color(&mbox_bg, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 11, 20));
  lv_style_set_border_width(&mbox_bg, LV_STATE_DEFAULT, 1);
  lv_style_set_shadow_width(&mbox_bg, LV_STATE_DEFAULT, LV_DPI / 10);
  lv_style_set_shadow_color(&mbox_bg, LV_STATE_DEFAULT, lv_color_hex3(0x222));
  lv_style_set_radius(&mbox_bg, LV_STATE_DEFAULT, LV_DPI / 20);
  
  theme.style.mbox.bg = &mbox_bg;
  theme.style.mbox.btn.bg = &lv_style_transp;
  theme.style.mbox.btn.rel = theme.style.btn.rel;
  theme.style.mbox.btn.pr = theme.style.btn.pr;
}

void LittleVgl::InitThemePage() {
  lv_style_copy(&page_scrl, &bg);
  lv_style_set_value_color(&page_scrl, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 10, 40));
  lv_style_set_bg_grad_color(&page_scrl, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 10, 40));
  lv_style_set_border_color(&page_scrl, LV_STATE_DEFAULT, lv_color_hex3(0x333));
  lv_style_set_border_width(&page_scrl, LV_STATE_DEFAULT, 1);
  lv_style_set_radius(&page_scrl, LV_STATE_DEFAULT, LV_DPI / 20);
  
  theme.style.page.bg = &panel;
  theme.style.page.scrl = &page_scrl;
  theme.style.page.sb = &sb;
}

void LittleVgl::InitThemeTextArea() {
  theme.style.ta.area = &panel;
  theme.style.ta.oneline = &panel;
  theme.style.ta.cursor = NULL;
  theme.style.ta.sb = &def;
}

void LittleVgl::InitThemeSpinBox() {
  theme.style.spinbox.bg = &panel;
  theme.style.spinbox.cursor = theme.style.ta.cursor;
  theme.style.spinbox.sb = theme.style.ta.sb;
}

void LittleVgl::InitThemeList() {
  
  lv_style_copy(&list_bg, &panel);
  lv_style_set_pad_top(&list_bg, LV_STATE_DEFAULT, 0);
  lv_style_set_pad_bottom(&list_bg, LV_STATE_DEFAULT, 0);
  lv_style_set_pad_left(&list_bg, LV_STATE_DEFAULT, 0);
  lv_style_set_pad_right(&list_bg, LV_STATE_DEFAULT, 0);
  lv_style_set_pad_inner(&list_bg, LV_STATE_DEFAULT, 0);
  
  lv_style_copy(&list_btn_rel, &bg);
  lv_style_set_image_opa(&list_btn_rel, LV_STATE_DEFAULT, LV_OPA_TRANSP);
  list_btn_rel.body.border.part = LV_BORDER_BOTTOM;
  lv_style_set_border_color(&list_btn_rel, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 10, 5));
  lv_style_set_border_width(&list_btn_rel, LV_STATE_DEFAULT, 1);
  lv_style_set_radius(&list_btn_rel, LV_STATE_DEFAULT, LV_DPI / 10);
  lv_style_set_text_color(&list_btn_rel, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 5, 80));
  lv_style_set_image_recolor(&list_btn_rel, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 5, 80));
  lv_style_set_pad_top(&list_btn_rel, LV_STATE_DEFAULT, LV_DPI / 6);
  lv_style_set_pad_bottom(&list_btn_rel, LV_STATE_DEFAULT, LV_DPI / 6);
  lv_style_set_pad_left(&list_btn_rel, LV_STATE_DEFAULT, LV_DPI / 8);
  lv_style_set_pad_right(&list_btn_rel, LV_STATE_DEFAULT, LV_DPI / 8);
  
  lv_style_copy(&list_btn_pr, theme.style.btn.pr);
  lv_style_set_value_color(&list_btn_pr, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 10, 5));
  lv_style_set_bg_grad_color(&list_btn_pr, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 10, 5));
  lv_style_set_border_color(&list_btn_pr, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 10, 5));
  lv_style_set_border_width(&list_btn_pr, LV_STATE_DEFAULT, 0);
  lv_style_set_pad_top(&list_btn_pr, LV_STATE_DEFAULT, LV_DPI / 6);
  lv_style_set_pad_bottom(&list_btn_pr, LV_STATE_DEFAULT, LV_DPI / 6);
  lv_style_set_pad_left(&list_btn_pr, LV_STATE_DEFAULT, LV_DPI / 8);
  lv_style_set_pad_right(&list_btn_pr, LV_STATE_DEFAULT, LV_DPI / 8);
  lv_style_set_text_color(&list_btn_pr, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 5, 80));
  lv_style_set_image_recolor(&list_btn_pr, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 5, 80));
  
  lv_style_copy(&list_btn_tgl_rel, &list_btn_rel);
  lv_style_set_image_opa(&list_btn_tgl_rel, LV_STATE_DEFAULT, LV_OPA_COVER);
  lv_style_set_value_color(&list_btn_tgl_rel, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 80, 70));
  lv_style_set_bg_grad_color(&list_btn_tgl_rel, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 80, 70));
  lv_style_set_border_color(&list_btn_tgl_rel, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 60, 40));
  lv_style_set_radius(&list_btn_tgl_rel, LV_STATE_DEFAULT, list_bg.body.radius);
  
  lv_style_copy(&list_btn_tgl_pr, &list_btn_tgl_rel);
  lv_style_set_value_color(&list_btn_tgl_pr, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 80, 60));
  lv_style_set_bg_grad_color(&list_btn_tgl_pr, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 80, 60));
  
  theme.style.list.sb = &sb;
  theme.style.list.bg = &list_bg;
  theme.style.list.scrl = &lv_style_transp_tight;
  theme.style.list.btn.rel = &list_btn_rel;
  theme.style.list.btn.pr = &list_btn_pr;
  theme.style.list.btn.tgl_rel = &list_btn_tgl_rel;
  theme.style.list.btn.tgl_pr = &list_btn_tgl_pr;
  theme.style.list.btn.ina = &def;
}

void LittleVgl::InitThemeDropDownList() {
  lv_style_copy(&ddlist_bg, theme.style.btn.rel);
  ddlist_bg.text.line_space = LV_DPI / 8;
  lv_style_set_pad_top(&ddlist_bg, LV_STATE_DEFAULT, LV_DPI / 8);
  lv_style_set_pad_bottom(&ddlist_bg, LV_STATE_DEFAULT, LV_DPI / 8);
  lv_style_set_pad_left(&ddlist_bg, LV_STATE_DEFAULT, LV_DPI / 8);
  lv_style_set_pad_right(&ddlist_bg, LV_STATE_DEFAULT, LV_DPI / 8);
  lv_style_set_radius(&ddlist_bg, LV_STATE_DEFAULT, LV_DPI / 30);
  
  lv_style_copy(&ddlist_sel, theme.style.btn.rel);
  lv_style_set_value_color(&ddlist_sel, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 20, 50));
  lv_style_set_bg_grad_color(&ddlist_sel, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 20, 50));
  lv_style_set_radius(&ddlist_sel, LV_STATE_DEFAULT, 0);
  
  theme.style.ddlist.bg = &ddlist_bg;
  theme.style.ddlist.sel = &ddlist_sel;
  theme.style.ddlist.sb = &def;
}

void LittleVgl::InitThemeRoller() {
  lv_style_t roller_bg;
  
  lv_style_copy(&roller_bg, theme.style.ddlist.bg);
  lv_style_set_value_color(&roller_bg, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 10, 20));
  lv_style_set_bg_grad_color(&roller_bg, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 10, 40));
  lv_style_set_text_color(&roller_bg, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 5, 70));
  roller_bg.text.opa = LV_OPA_60;
  
  theme.style.roller.bg = &roller_bg;
  theme.style.roller.sel = theme.style.ddlist.sel;
}

void LittleVgl::InitThemeTabView() {
  theme.style.tabview.bg = &bg;
  theme.style.tabview.indic = &lv_style_transp;
  theme.style.tabview.btn.bg = &lv_style_transp;
  theme.style.tabview.btn.rel = theme.style.btn.rel;
  theme.style.tabview.btn.pr = theme.style.btn.pr;
  theme.style.tabview.btn.tgl_rel = theme.style.btn.tgl_rel;
  theme.style.tabview.btn.tgl_pr = theme.style.btn.tgl_pr;
}

void LittleVgl::InitThemeTileView() {
  theme.style.tileview.bg = &lv_style_transp_tight;
  theme.style.tileview.scrl = &lv_style_transp_tight;
  theme.style.tileview.sb = theme.style.page.sb;
}

void LittleVgl::InitThemeTable() {
  lv_style_copy(&cell, &panel);
  lv_style_set_radius(&cell, LV_STATE_DEFAULT, 0);
  lv_style_set_border_width(&cell, LV_STATE_DEFAULT, 1);
  lv_style_set_pad_left(&cell, LV_STATE_DEFAULT, LV_DPI / 12);
  lv_style_set_pad_right(&cell, LV_STATE_DEFAULT, LV_DPI / 12);
  lv_style_set_pad_top(&cell, LV_STATE_DEFAULT, LV_DPI / 12);
  lv_style_set_pad_bottom(&cell, LV_STATE_DEFAULT, LV_DPI / 12);
  
  theme.style.table.bg = &lv_style_transp_tight;
  theme.style.table.cell = &cell;
}

void LittleVgl::InitThemeWindow() {
//  lv_style_copy(&win_bg, &bg);
//  lv_style_set_border_color(&win_bg, LV_STATE_DEFAULT, lv_color_hex3(0x333));
//  lv_style_set_border_width(&win_bg, LV_STATE_DEFAULT, 1);
//
//  lv_style_copy(&win_header, &win_bg);
//  win_header.body.main_color     = lv_color_hsv_to_rgb(hue, 10, 20);
//  win_header.body.grad_color     = lv_color_hsv_to_rgb(hue, 10, 20);
//  win_header.body.radius         = 0;
//  win_header.body.padding.left   = 0;
//  win_header.body.padding.right  = 0;
//  win_header.body.padding.top    = 0;
//  lv_style_set_pad_bottom(&win_header, LV_STATE_DEFAULT, 0);
//
//  lv_style_copy(&win_btn_pr, &def);
//  lv_style_set_value_color(&win_btn_pr, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 10, 10));
//  lv_style_set_bg_grad_color(&win_btn_pr, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 10, 10));
//  win_btn_pr.text.color      = lv_color_hex3(0xaaa);
//  win_btn_pr.image.color     = lv_color_hex3(0xaaa);
//
//  theme.style.win.bg      = &win_bg;
//  theme.style.win.sb      = &sb;
//  theme.style.win.header  = &win_header;
//  theme.style.win.content = &lv_style_transp;
//  theme.style.win.btn.rel = &lv_style_transp;
//  theme.style.win.btn.pr  = &win_btn_pr;
}



