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
  bg.text.color = LV_COLOR_WHITE;
  bg.text.font = font;
  bg.image.color = LV_COLOR_WHITE;
  
  lv_style_copy(&scr, &bg);
  scr.body.padding.bottom = 0;
  scr.body.padding.top = 0;
  scr.body.padding.left = 0;
  scr.body.padding.right = 0;
  
  lv_style_copy(&sb, &def);
  lv_style_set_value_color(&sb, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 30, 60));
  lv_style_set_bg_grad_color(&sb, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 30, 60));
  sb.body.border.width = 0;
  sb.body.padding.inner = LV_DPI / 20;
  sb.body.padding.left = 0;
  sb.body.padding.right = 0;
  sb.body.padding.top = 0;
  sb.body.padding.bottom = 0;
  sb.body.radius = LV_DPI / 30;
  sb.body.opa = LV_OPA_COVER;
  
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
  
  panel.line.color = lv_color_hsv_to_rgb(hue, 20, 40);
  panel.line.width = 1;
  
  
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
  btn_rel.body.border.color = lv_color_hex3(0x111);
  btn_rel.body.border.width = 1;
  btn_rel.body.border.opa = LV_OPA_70;
  btn_rel.body.padding.left = LV_DPI / 4;
  btn_rel.body.padding.right = LV_DPI / 4;
  btn_rel.body.padding.top = LV_DPI / 8;
  btn_rel.body.padding.bottom = LV_DPI / 8;
  btn_rel.body.shadow.type = LV_SHADOW_BOTTOM;
  btn_rel.body.shadow.color = lv_color_hex3(0x111);
  btn_rel.body.shadow.width = LV_DPI / 30;
  btn_rel.text.color = lv_color_hex3(0xeee);
  btn_rel.image.color = lv_color_hex3(0xeee);
  
  lv_style_copy(&btn_pr, &btn_rel);
  lv_style_set_value_color(&btn_pr, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 10, 30));
  lv_style_set_bg_grad_color(&btn_pr, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 10, 10));
  
  lv_style_copy(&btn_tgl_rel, &btn_rel);
  lv_style_set_value_color(&btn_tgl_rel, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 10, 20));
  lv_style_set_bg_grad_color(&btn_tgl_rel, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 10, 40));
  btn_tgl_rel.body.shadow.width = LV_DPI / 40;
  btn_tgl_rel.text.color = lv_color_hex3(0xddd);
  btn_tgl_rel.image.color = lv_color_hex3(0xddd);
  
  lv_style_copy(&btn_tgl_pr, &btn_rel);
  lv_style_set_value_color(&btn_tgl_pr, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 10, 10));
  lv_style_set_bg_grad_color(&btn_tgl_pr, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 10, 30));
  btn_tgl_pr.body.shadow.width = LV_DPI / 30;
  btn_tgl_pr.text.color = lv_color_hex3(0xddd);
  btn_tgl_pr.image.color = lv_color_hex3(0xddd);
  
  lv_style_copy(&btn_ina, &btn_rel);
  lv_style_set_value_color(&btn_ina, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 10, 20));
  lv_style_set_bg_grad_color(&btn_ina, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 10, 20));
  btn_ina.body.shadow.width = 0;
  btn_ina.text.color = lv_color_hex3(0xaaa);
  btn_ina.image.color = lv_color_hex3(0xaaa);
  
  theme.style.btn.rel = &btn_rel;
  theme.style.btn.pr = &btn_pr;
  theme.style.btn.tgl_rel = &btn_tgl_rel;
  theme.style.btn.tgl_pr = &btn_tgl_pr;
  theme.style.btn.ina = &btn_ina;
}

void LittleVgl::InitThemeLabel() {
  lv_style_copy(&prim, &bg);
  prim.text.color = lv_color_hsv_to_rgb(hue, 5, 95);
  
  lv_style_copy(&labelBigStyle, &prim);
  labelBigStyle.text.font = &jetbrains_mono_extrabold_compressed;
  LabelBigStyle = &(this->labelBigStyle);
  
  lv_style_copy(&sec, &bg);
  sec.text.color = lv_color_hsv_to_rgb(hue, 15, 65);
  
  lv_style_copy(&hint, &bg);
  hint.text.color = lv_color_hsv_to_rgb(hue, 20, 55);
  
  theme.style.label.prim = &prim;
  theme.style.label.sec = &sec;
  theme.style.label.hint = &hint;
}

void LittleVgl::InitThemeLine() {
  theme.style.line.decor = &def;
}

void LittleVgl::InitThemeLed() {
  lv_style_copy(&led, &def);
  led.body.shadow.width = LV_DPI / 10;
  led.body.radius = LV_RADIUS_CIRCLE;
  led.body.border.width = LV_DPI / 30;
  led.body.border.opa = LV_OPA_30;
  lv_style_set_value_color(&led, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 100, 100));
  lv_style_set_bg_grad_color(&led, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 100, 40));
  led.body.border.color = lv_color_hsv_to_rgb(hue, 60, 60);
  led.body.shadow.color = lv_color_hsv_to_rgb(hue, 100, 100);
  
  theme.style.led = &led;
}

void LittleVgl::InitThemeImage() {
  theme.style.img.light = &def;
  theme.style.img.dark = &def;
}

void LittleVgl::InitThemeBar() {
  lv_style_copy(&bar_bg, &panel);
  bar_bg.body.padding.left = LV_DPI / 16;
  bar_bg.body.padding.right = LV_DPI / 16;
  bar_bg.body.padding.top = LV_DPI / 16;
  bar_bg.body.padding.bottom = LV_DPI / 16;
  bar_bg.body.radius = LV_RADIUS_CIRCLE;
  
  lv_style_copy(&bar_indic, &def);
  lv_style_set_value_color(&bar_indic, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 80, 70));
  lv_style_set_bg_grad_color(&bar_indic, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 80, 70));
  bar_indic.body.border.color = lv_color_hsv_to_rgb(hue, 20, 15);
  bar_indic.body.border.width = 1;
  bar_indic.body.border.opa = LV_OPA_COVER;
  bar_indic.body.radius = LV_RADIUS_CIRCLE;
  bar_indic.body.padding.left = 0;
  bar_indic.body.padding.right = 0;
  bar_indic.body.padding.top = 0;
  bar_indic.body.padding.bottom = 0;
  
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
  lmeter_bg.body.padding.left = LV_DPI / 10; /*Scale line length*/
  lmeter_bg.body.padding.inner = LV_DPI / 10; /*Text padding*/
  lmeter_bg.body.border.color = lv_color_hex3(0x333);
  lmeter_bg.line.color = lv_color_hex3(0x555);
  lmeter_bg.line.width = 1;
  lmeter_bg.text.color = lv_color_hex3(0xddd);
  
  theme.style.lmeter = &lmeter_bg;
}

void LittleVgl::InitThemeGauge() {
  static lv_style_t gauge_bg;
  lv_style_copy(&gauge_bg, &def);
  lv_style_set_value_color(&gauge_bg, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 10, 70));
  lv_style_set_bg_grad_color(&gauge_bg, LV_STATE_DEFAULT, gauge_bg.body.main_color);
  gauge_bg.line.color = lv_color_hsv_to_rgb(hue, 80, 75);
  gauge_bg.line.width = 1;
  gauge_bg.text.color = lv_color_hex3(0xddd);
  
  theme.style.gauge = &gauge_bg;
}

void LittleVgl::InitThemeArc() {
  lv_style_copy(&arc, &def);
  arc.line.width = 8;
  arc.line.color = lv_color_hsv_to_rgb(hue, 80, 70);
  arc.line.rounded = 1;
  
  /*For preloader*/
  arc.body.border.width = 7;
  arc.body.border.color = lv_color_hsv_to_rgb(hue, 11, 48);
  arc.body.padding.left = 1;
  arc.body.padding.right = 1;
  arc.body.padding.top = 1;
  arc.body.padding.bottom = 1;
  
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
  cal_bg.body.border.color = lv_color_hex3(0x333);
  cal_bg.body.border.width = 1;
  cal_bg.body.radius = LV_DPI / 20;
  cal_bg.body.padding.left = LV_DPI / 10;
  cal_bg.body.padding.right = LV_DPI / 10;
  cal_bg.body.padding.top = LV_DPI / 10;
  cal_bg.body.padding.bottom = LV_DPI / 10;
  
  
  lv_style_copy(&cal_header, &bg);
  lv_style_set_value_color(&cal_header, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 10, 20));
  lv_style_set_bg_grad_color(&cal_header, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 10, 20));
  cal_header.body.radius = 0;
  cal_header.body.border.width = 1;
  cal_header.body.border.color = lv_color_hex3(0x333);
  cal_header.body.padding.left = LV_DPI / 10;
  cal_header.body.padding.right = LV_DPI / 10;
  cal_header.body.padding.top = LV_DPI / 10;
  cal_header.body.padding.bottom = LV_DPI / 10;
  
  
  lv_style_copy(&week_box, &panel);
  lv_style_set_value_color(&week_box, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 30, 45));
  lv_style_set_bg_grad_color(&week_box, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 30, 45));
  week_box.body.radius = LV_DPI / 20;
  week_box.body.border.width = 1;
  week_box.body.padding.left = LV_DPI / 20;
  week_box.body.padding.right = LV_DPI / 20;
  week_box.body.padding.top = LV_DPI / 25;
  week_box.body.padding.bottom = LV_DPI / 25;
  
  lv_style_copy(&today_box, &week_box);
  lv_style_set_value_color(&today_box, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 80, 70));
  lv_style_set_bg_grad_color(&today_box, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 80, 70));
  today_box.body.radius = LV_DPI / 20;
  today_box.body.padding.left = LV_DPI / 14;
  today_box.body.padding.right = LV_DPI / 14;
  today_box.body.padding.top = LV_DPI / 14;
  today_box.body.padding.bottom = LV_DPI / 14;
  
  lv_style_copy(&highlighted_days, &bg);
  highlighted_days.text.color = lv_color_hsv_to_rgb(hue, 40, 80);
  
  lv_style_copy(&ina_days, &bg);
  ina_days.text.color = lv_color_hsv_to_rgb(hue, 0, 60);
  
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
  rel.body.radius = LV_DPI / 20;
  lv_style_set_value_color(&rel, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 10, 95));
  lv_style_set_bg_grad_color(&rel, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 10, 95));
  rel.body.border.color = lv_color_hsv_to_rgb(hue, 10, 50);
  rel.body.border.width = 2;;
  
  lv_style_copy(&pr, &rel);
  lv_style_set_value_color(&pr, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 10, 80));
  lv_style_set_bg_grad_color(&pr, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 10, 80));
  pr.body.border.color = lv_color_hsv_to_rgb(hue, 10, 20);
  pr.body.border.width = 1;;
  
  lv_style_copy(&tgl_rel, &rel);
  lv_style_set_value_color(&tgl_rel, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 80, 90));
  lv_style_set_bg_grad_color(&tgl_rel, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 80, 90));
  tgl_rel.body.border.color = lv_color_hsv_to_rgb(hue, 80, 50);
  
  lv_style_copy(&tgl_pr, &tgl_rel);
  lv_style_set_value_color(&tgl_pr, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 80, 70));
  lv_style_set_bg_grad_color(&tgl_pr, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 80, 70));
  tgl_pr.body.border.color = lv_color_hsv_to_rgb(hue, 80, 30);
  tgl_pr.body.border.width = 1;;
  
  lv_style_copy(&ina, &rel);
  lv_style_set_value_color(&ina, LV_STATE_DEFAULT, lv_color_hex3(0x777));
  lv_style_set_bg_grad_color(&ina, LV_STATE_DEFAULT, lv_color_hex3(0x777));
  ina.body.border.width = 0;
  
  theme.style.cb.bg = &lv_style_transp;
  theme.style.cb.box.rel = &rel;
  theme.style.cb.box.pr = &pr;
  theme.style.cb.box.tgl_rel = &tgl_rel;
  theme.style.cb.box.tgl_pr = &tgl_pr;
  theme.style.cb.box.ina = &def;
}

void LittleVgl::InitThemeButtonMatrix() {
  
  lv_style_copy(&btnm_bg, theme.style.btn.rel);
  btnm_bg.body.padding.left = 2;
  btnm_bg.body.padding.right = 2;
  btnm_bg.body.padding.top = 2;
  btnm_bg.body.padding.bottom = 2;
  btnm_bg.body.padding.inner = 0;
  btnm_bg.body.border.width = 1;
  
  lv_style_copy(&btnm_rel, theme.style.btn.rel);
  btnm_rel.body.border.part = LV_BORDER_FULL | LV_BORDER_INTERNAL;
  btnm_rel.body.border.width = 1;
  btnm_rel.body.radius = 2;
  
  lv_style_copy(&btnm_pr, theme.style.btn.pr);
  btnm_pr.body.border.part = btnm_rel.body.border.part;
  btnm_pr.body.border.width = btnm_rel.body.border.width;
  btnm_pr.body.radius = btnm_rel.body.radius;
  
  lv_style_copy(&btnm_tgl_rel, theme.style.btn.tgl_rel);
  btnm_tgl_rel.body.border.part = btnm_rel.body.border.part;
  btnm_tgl_rel.body.border.width = btnm_rel.body.border.width;
  btnm_tgl_rel.body.radius = btnm_rel.body.radius;
  
  lv_style_copy(&btnm_tgl_pr, theme.style.btn.pr);
  btnm_tgl_pr.body.border.part = btnm_rel.body.border.part;
  btnm_tgl_pr.body.border.width = btnm_rel.body.border.width;
  btnm_tgl_pr.body.radius = btnm_rel.body.radius;
  
  lv_style_copy(&btnm_ina, theme.style.btn.ina);
  btnm_ina.body.border.part = btnm_rel.body.border.part;
  btnm_ina.body.border.width = btnm_rel.body.border.width;
  btnm_ina.body.radius = btnm_rel.body.radius;
  
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
  mbox_bg.body.border.color = lv_color_hsv_to_rgb(hue, 11, 20);
  mbox_bg.body.border.width = 1;
  mbox_bg.body.shadow.width = LV_DPI / 10;
  mbox_bg.body.shadow.color = lv_color_hex3(0x222);
  mbox_bg.body.radius = LV_DPI / 20;
  theme.style.mbox.bg = &mbox_bg;
  theme.style.mbox.btn.bg = &lv_style_transp;
  theme.style.mbox.btn.rel = theme.style.btn.rel;
  theme.style.mbox.btn.pr = theme.style.btn.pr;
}

void LittleVgl::InitThemePage() {
  lv_style_copy(&page_scrl, &bg);
  lv_style_set_value_color(&page_scrl, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 10, 40));
  lv_style_set_bg_grad_color(&page_scrl, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 10, 40));
  page_scrl.body.border.color = lv_color_hex3(0x333);
  page_scrl.body.border.width = 1;
  page_scrl.body.radius = LV_DPI / 20;
  
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
  list_bg.body.padding.top = 0;
  list_bg.body.padding.bottom = 0;
  list_bg.body.padding.left = 0;
  list_bg.body.padding.right = 0;
  list_bg.body.padding.inner = 0;
  
  lv_style_copy(&list_btn_rel, &bg);
  list_btn_rel.body.opa = LV_OPA_TRANSP;
  list_btn_rel.body.border.part = LV_BORDER_BOTTOM;
  list_btn_rel.body.border.color = lv_color_hsv_to_rgb(hue, 10, 5);
  list_btn_rel.body.border.width = 1;
  list_btn_rel.body.radius = LV_DPI / 10;
  list_btn_rel.text.color = lv_color_hsv_to_rgb(hue, 5, 80);
  list_btn_rel.image.color = lv_color_hsv_to_rgb(hue, 5, 80);
  list_btn_rel.body.padding.top = LV_DPI / 6;
  list_btn_rel.body.padding.bottom = LV_DPI / 6;
  list_btn_rel.body.padding.left = LV_DPI / 8;
  list_btn_rel.body.padding.right = LV_DPI / 8;
  
  lv_style_copy(&list_btn_pr, theme.style.btn.pr);
  lv_style_set_value_color(&list_btn_pr, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 10, 5));
  lv_style_set_bg_grad_color(&list_btn_pr, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 10, 5));
  list_btn_pr.body.border.color = lv_color_hsv_to_rgb(hue, 10, 5);
  list_btn_pr.body.border.width = 0;
  list_btn_pr.body.padding.top = LV_DPI / 6;
  list_btn_pr.body.padding.bottom = LV_DPI / 6;
  list_btn_pr.body.padding.left = LV_DPI / 8;
  list_btn_pr.body.padding.right = LV_DPI / 8;
  list_btn_pr.text.color = lv_color_hsv_to_rgb(hue, 5, 80);
  list_btn_pr.image.color = lv_color_hsv_to_rgb(hue, 5, 80);
  
  lv_style_copy(&list_btn_tgl_rel, &list_btn_rel);
  list_btn_tgl_rel.body.opa = LV_OPA_COVER;
  lv_style_set_value_color(&list_btn_tgl_rel, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 80, 70));
  lv_style_set_bg_grad_color(&list_btn_tgl_rel, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 80, 70));
  list_btn_tgl_rel.body.border.color = lv_color_hsv_to_rgb(hue, 60, 40);
  list_btn_tgl_rel.body.radius = list_bg.body.radius;
  
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
  ddlist_bg.body.padding.top = LV_DPI / 8;
  ddlist_bg.body.padding.bottom = LV_DPI / 8;
  ddlist_bg.body.padding.left = LV_DPI / 8;
  ddlist_bg.body.padding.right = LV_DPI / 8;
  ddlist_bg.body.radius = LV_DPI / 30;
  
  lv_style_copy(&ddlist_sel, theme.style.btn.rel);
  lv_style_set_value_color(&ddlist_sel, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 20, 50));
  lv_style_set_bg_grad_color(&ddlist_sel, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 20, 50));
  ddlist_sel.body.radius = 0;
  
  theme.style.ddlist.bg = &ddlist_bg;
  theme.style.ddlist.sel = &ddlist_sel;
  theme.style.ddlist.sb = &def;
}

void LittleVgl::InitThemeRoller() {
  lv_style_t roller_bg;
  
  lv_style_copy(&roller_bg, theme.style.ddlist.bg);
  lv_style_set_value_color(&roller_bg, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 10, 20));
  lv_style_set_bg_grad_color(&roller_bg, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 10, 40));
  roller_bg.text.color = lv_color_hsv_to_rgb(hue, 5, 70);
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
  cell.body.radius = 0;
  cell.body.border.width = 1;
  cell.body.padding.left = LV_DPI / 12;
  cell.body.padding.right = LV_DPI / 12;
  cell.body.padding.top = LV_DPI / 12;
  cell.body.padding.bottom = LV_DPI / 12;
  
  theme.style.table.bg = &lv_style_transp_tight;
  theme.style.table.cell = &cell;
}

void LittleVgl::InitThemeWindow() {
//  lv_style_copy(&win_bg, &bg);
//  win_bg.body.border.color = lv_color_hex3(0x333);
//  win_bg.body.border.width = 1;
//
//  lv_style_copy(&win_header, &win_bg);
//  win_header.body.main_color     = lv_color_hsv_to_rgb(hue, 10, 20);
//  win_header.body.grad_color     = lv_color_hsv_to_rgb(hue, 10, 20);
//  win_header.body.radius         = 0;
//  win_header.body.padding.left   = 0;
//  win_header.body.padding.right  = 0;
//  win_header.body.padding.top    = 0;
//  win_header.body.padding.bottom = 0;
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



