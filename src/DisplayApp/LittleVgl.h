#pragma once

#include <libs/lvgl/src/lv_core/lv_style.h>
#include <libs/lvgl/src/lv_themes/lv_theme.h>
#include <libs/lvgl/src/lv_hal/lv_hal.h>
#include <drivers/St7789.h>
#include <drivers/Cst816s.h>


static void disp_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p);
static bool touchpad_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data);

namespace Pinetime {
  namespace Components {
    class LittleVgl {
      public:
        LittleVgl(Pinetime::Drivers::St7789& lcd, Pinetime::Drivers::Cst816S& touchPanel);

        LittleVgl(const LittleVgl&) = delete;
        LittleVgl& operator=(const LittleVgl&) = delete;
        LittleVgl(LittleVgl&&) = delete;
        LittleVgl& operator=(LittleVgl&&) = delete;

        void FlushDisplay(const lv_area_t * area, lv_color_t * color_p);
        bool GetTouchPadInfo(lv_indev_data_t *ptr);
        void SetFullRefresh();
      private:
        void InitDisplay();
        void InitTouchpad();
        void InitTheme();
        void InitBaseTheme();
        void InitThemeContainer();
        void InitThemeButton();
        void InitThemeLabel();
        void InitThemeLine();
        void InitThemeLed();
        void InitThemeImage();
        void InitThemeBar();
        void InitThemeSlider();
        void InitThemeSwitch();
        void InitThemeMeter();
        void InitThemeGauge();
        void InitThemeArc();
        void InitThemePreload();
        void InitThemeChart();
        void InitThemeCalendar();
        void InitThemeCheckBox();
        void InitThemeButtonMatrix();
        void InitThemeKnob();
        void InitThemeMessageBox();
        void InitThemePage();
        void InitThemeTextArea();
        void InitThemeSpinBox();
        void InitThemeList();
        void InitThemeDropDownList();
        void InitThemeRoller();
        void InitThemeTabView();
        void InitThemeTileView();
        void InitThemeTable();
        void InitThemeWindow();

        Pinetime::Drivers::St7789& lcd;
        Pinetime::Drivers::Cst816S& touchPanel;


        lv_disp_buf_t disp_buf_2;
        lv_color_t buf2_1[LV_HOR_RES_MAX * 4];
        lv_color_t buf2_2[LV_HOR_RES_MAX * 4];

        lv_disp_drv_t disp_drv;
        lv_point_t previousClick;

        lv_style_t def;
        lv_style_t scr, bg, sb, panel;
        lv_font_t * font = nullptr;
        uint16_t hue = 10;
        lv_theme_t theme;
        lv_style_t btn_rel, btn_pr, btn_tgl_rel, btn_tgl_pr, btn_ina;
        lv_style_t prim, sec, hint;
        lv_style_t led;
        lv_style_t bar_bg, bar_indic;
        lv_style_t slider_knob;
        lv_style_t arc;
        lv_style_t cal_bg;
        lv_style_t cal_header;
        lv_style_t week_box;
        lv_style_t today_box;
        lv_style_t highlighted_days;
        lv_style_t ina_days;
        lv_style_t rel, pr, tgl_rel, tgl_pr, ina;
        lv_style_t btnm_bg, btnm_rel, btnm_pr, btnm_tgl_rel, btnm_tgl_pr, btnm_ina;
        lv_style_t mbox_bg;
        lv_style_t page_scrl;
        lv_style_t list_bg, list_btn_rel, list_btn_pr, list_btn_tgl_rel, list_btn_tgl_pr;
        lv_style_t ddlist_bg, ddlist_sel;
        lv_style_t cell;
        lv_style_t win_bg;
        lv_style_t win_header;
        lv_style_t win_btn_pr;

        bool firstTouch = true;
        bool fullRefresh = false;
        static constexpr uint8_t nbWriteLines = 4;
        static constexpr uint16_t totalNbLines = 320;
        static constexpr uint16_t visibleNbLines = 240;
        static constexpr uint8_t MaxScrollOffset() { return LV_VER_RES_MAX - nbWriteLines; }
        enum class ScrollDirections {Unknown, Up, Down};
        ScrollDirections scrollDirection = ScrollDirections::Up;
        uint16_t writeOffset = 0;
        uint16_t scrollOffset = 0;
    };
  }
}

