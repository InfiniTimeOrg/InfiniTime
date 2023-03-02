#include "displayapp/InfiniTimeTheme.h"
#include <algorithm>

// Replace LV_DPX with a constexpr version using a constant LV_DPI
#undef LV_DPX

namespace {
  constexpr int LV_DPX(int n) {
    if (n == 0) {
      return 0;
    }
    return std::max(((LV_DPI * n + 80) / 160), 1); /*+80 for rounding*/
  }
}

static void theme_apply(lv_obj_t* obj, lv_theme_style_t name);

static lv_theme_t theme;

static lv_style_t style_bg;
static lv_style_t style_box;
static lv_style_t style_btn;
static lv_style_t style_label_white;
static lv_style_t style_icon;
static lv_style_t style_bar_indic;
static lv_style_t style_slider_knob;
static lv_style_t style_scrollbar;
static lv_style_t style_list_btn;
static lv_style_t style_ddlist_list;
static lv_style_t style_ddlist_selected;
static lv_style_t style_sw_bg;
static lv_style_t style_sw_indic;
static lv_style_t style_sw_knob;
static lv_style_t style_arc_bg;
static lv_style_t style_arc_knob;
static lv_style_t style_arc_indic;
static lv_style_t style_table_cell;
static lv_style_t style_pad_small;
static lv_style_t style_lmeter;
static lv_style_t style_chart_serie;
static lv_style_t style_cb_bg;
static lv_style_t style_cb_bullet;

static bool inited;

static void style_init_reset(lv_style_t* style) {
  if (inited) {
    lv_style_reset(style);
  } else {
    lv_style_init(style);
  }
}

static void basic_init() {
  style_init_reset(&style_bg);
  lv_style_set_bg_opa(&style_bg, LV_STATE_DEFAULT, LV_OPA_COVER);
  lv_style_set_bg_color(&style_bg, LV_STATE_DEFAULT, LV_COLOR_BLACK);
  lv_style_set_text_font(&style_bg, LV_STATE_DEFAULT, theme.font_normal);

  style_init_reset(&style_box);
  lv_style_set_bg_opa(&style_box, LV_STATE_DEFAULT, LV_OPA_COVER);
  lv_style_set_radius(&style_box, LV_STATE_DEFAULT, 10);
  lv_style_set_value_font(&style_box, LV_STATE_DEFAULT, theme.font_normal);

  style_init_reset(&style_label_white);
  lv_style_set_text_color(&style_label_white, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_style_set_text_color(&style_label_white, LV_STATE_DISABLED, LV_COLOR_GRAY);

  style_init_reset(&style_btn);
  lv_style_set_radius(&style_btn, LV_STATE_DEFAULT, 10);
  lv_style_set_bg_opa(&style_btn, LV_STATE_DEFAULT, LV_OPA_COVER);
  lv_style_set_bg_color(&style_btn, LV_STATE_DEFAULT, Colors::bg);
  lv_style_set_bg_color(&style_btn, LV_STATE_CHECKED, Colors::highlight);
  lv_style_set_bg_color(&style_btn, LV_STATE_DISABLED, Colors::bgDark);

  lv_style_set_text_color(&style_btn, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_style_set_text_color(&style_btn, LV_STATE_DISABLED, LV_COLOR_GRAY);

  lv_style_set_pad_all(&style_btn, LV_STATE_DEFAULT, LV_DPX(20));
  lv_style_set_pad_inner(&style_btn, LV_STATE_DEFAULT, LV_DPX(15));

  style_init_reset(&style_icon);
  lv_style_set_text_color(&style_icon, LV_STATE_DEFAULT, LV_COLOR_WHITE);

  style_init_reset(&style_bar_indic);
  lv_style_set_bg_opa(&style_bar_indic, LV_STATE_DEFAULT, LV_OPA_COVER);
  lv_style_set_radius(&style_bar_indic, LV_STATE_DEFAULT, 10);

  style_init_reset(&style_scrollbar);
  lv_style_set_bg_opa(&style_scrollbar, LV_STATE_DEFAULT, LV_OPA_COVER);
  lv_style_set_radius(&style_scrollbar, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
  lv_style_set_bg_color(&style_scrollbar, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_style_set_size(&style_scrollbar, LV_STATE_DEFAULT, LV_HOR_RES / 80);
  lv_style_set_pad_right(&style_scrollbar, LV_STATE_DEFAULT, LV_HOR_RES / 60);

  style_init_reset(&style_list_btn);
  lv_style_set_bg_opa(&style_list_btn, LV_STATE_DEFAULT, LV_OPA_COVER);
  lv_style_set_bg_color(&style_list_btn, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_style_set_text_color(&style_list_btn, LV_STATE_DEFAULT, Colors::bg);
  lv_style_set_text_color(&style_list_btn, LV_STATE_CHECKED, LV_COLOR_WHITE);
  lv_style_set_image_recolor(&style_list_btn, LV_STATE_DEFAULT, Colors::bg);
  lv_style_set_image_recolor(&style_list_btn, LV_STATE_CHECKED, LV_COLOR_WHITE);
  lv_style_set_pad_left(&style_list_btn, LV_STATE_DEFAULT, LV_HOR_RES / 25);
  lv_style_set_pad_right(&style_list_btn, LV_STATE_DEFAULT, LV_HOR_RES / 25);
  lv_style_set_pad_top(&style_list_btn, LV_STATE_DEFAULT, LV_HOR_RES / 100);
  lv_style_set_pad_bottom(&style_list_btn, LV_STATE_DEFAULT, LV_HOR_RES / 100);
  lv_style_set_pad_inner(&style_list_btn, LV_STATE_DEFAULT, LV_HOR_RES / 50);

  style_init_reset(&style_ddlist_list);
  // Causes lag unfortunately, so we'll have to live with the selected item overflowing the corner
  // lv_style_set_clip_corner(&style_ddlist_list, LV_STATE_DEFAULT, true);
  lv_style_set_text_line_space(&style_ddlist_list, LV_STATE_DEFAULT, LV_VER_RES / 25);
  lv_style_set_bg_color(&style_ddlist_list, LV_STATE_DEFAULT, Colors::lightGray);
  lv_style_set_pad_all(&style_ddlist_list, LV_STATE_DEFAULT, 20);

  style_init_reset(&style_ddlist_selected);
  lv_style_set_bg_opa(&style_ddlist_selected, LV_STATE_DEFAULT, LV_OPA_COVER);
  lv_style_set_bg_color(&style_ddlist_selected, LV_STATE_DEFAULT, Colors::bg);

  style_init_reset(&style_sw_bg);
  lv_style_set_bg_opa(&style_sw_bg, LV_STATE_DEFAULT, LV_OPA_COVER);
  lv_style_set_bg_color(&style_sw_bg, LV_STATE_DEFAULT, Colors::bg);
  lv_style_set_radius(&style_sw_bg, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);

  style_init_reset(&style_sw_indic);
  lv_style_set_bg_opa(&style_sw_indic, LV_STATE_DEFAULT, LV_OPA_COVER);
  lv_style_set_bg_color(&style_sw_indic, LV_STATE_DEFAULT, Colors::highlight);

  style_init_reset(&style_sw_knob);
  lv_style_set_bg_opa(&style_sw_knob, LV_STATE_DEFAULT, LV_OPA_COVER);
  lv_style_set_bg_color(&style_sw_knob, LV_STATE_DEFAULT, LV_COLOR_SILVER);
  lv_style_set_bg_color(&style_sw_knob, LV_STATE_CHECKED, LV_COLOR_WHITE);
  lv_style_set_radius(&style_sw_knob, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
  lv_style_set_pad_all(&style_sw_knob, LV_STATE_DEFAULT, -4);

  style_init_reset(&style_slider_knob);
  lv_style_set_bg_opa(&style_slider_knob, LV_STATE_DEFAULT, LV_OPA_COVER);
  lv_style_set_bg_color(&style_slider_knob, LV_STATE_DEFAULT, LV_COLOR_RED);
  lv_style_set_border_color(&style_slider_knob, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_style_set_border_width(&style_slider_knob, LV_STATE_DEFAULT, 6);
  lv_style_set_radius(&style_slider_knob, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
  lv_style_set_pad_all(&style_slider_knob, LV_STATE_DEFAULT, 10);
  lv_style_set_pad_all(&style_slider_knob, LV_STATE_PRESSED, 14);

  style_init_reset(&style_arc_indic);
  lv_style_set_line_color(&style_arc_indic, LV_STATE_DEFAULT, Colors::lightGray);
  lv_style_set_line_width(&style_arc_indic, LV_STATE_DEFAULT, LV_DPX(25));
  lv_style_set_line_rounded(&style_arc_indic, LV_STATE_DEFAULT, true);

  style_init_reset(&style_arc_bg);
  lv_style_set_line_color(&style_arc_bg, LV_STATE_DEFAULT, Colors::bg);
  lv_style_set_line_width(&style_arc_bg, LV_STATE_DEFAULT, LV_DPX(25));
  lv_style_set_line_rounded(&style_arc_bg, LV_STATE_DEFAULT, true);
  lv_style_set_pad_all(&style_arc_bg, LV_STATE_DEFAULT, LV_DPX(5));

  lv_style_reset(&style_arc_knob);
  lv_style_set_radius(&style_arc_knob, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
  lv_style_set_bg_opa(&style_arc_knob, LV_STATE_DEFAULT, LV_OPA_COVER);
  lv_style_set_bg_color(&style_arc_knob, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_style_set_pad_all(&style_arc_knob, LV_STATE_DEFAULT, LV_DPX(5));

  style_init_reset(&style_table_cell);
  lv_style_set_border_color(&style_table_cell, LV_STATE_DEFAULT, LV_COLOR_GRAY);
  lv_style_set_border_width(&style_table_cell, LV_STATE_DEFAULT, 1);
  lv_style_set_border_side(&style_table_cell, LV_STATE_DEFAULT, LV_BORDER_SIDE_FULL);
  lv_style_set_pad_left(&style_table_cell, LV_STATE_DEFAULT, 5);
  lv_style_set_pad_right(&style_table_cell, LV_STATE_DEFAULT, 5);
  lv_style_set_pad_top(&style_table_cell, LV_STATE_DEFAULT, 2);
  lv_style_set_pad_bottom(&style_table_cell, LV_STATE_DEFAULT, 2);

  style_init_reset(&style_pad_small);
  lv_style_int_t pad_small_value = 10;
  lv_style_set_pad_all(&style_pad_small, LV_STATE_DEFAULT, pad_small_value);
  lv_style_set_pad_inner(&style_pad_small, LV_STATE_DEFAULT, pad_small_value);

  style_init_reset(&style_lmeter);
  lv_style_set_radius(&style_lmeter, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
  lv_style_set_pad_left(&style_lmeter, LV_STATE_DEFAULT, LV_DPX(20));
  lv_style_set_pad_right(&style_lmeter, LV_STATE_DEFAULT, LV_DPX(20));
  lv_style_set_pad_top(&style_lmeter, LV_STATE_DEFAULT, LV_DPX(20));
  lv_style_set_pad_inner(&style_lmeter, LV_STATE_DEFAULT, LV_DPX(30));
  lv_style_set_scale_width(&style_lmeter, LV_STATE_DEFAULT, LV_DPX(25));

  lv_style_set_line_color(&style_lmeter, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_style_set_scale_grad_color(&style_lmeter, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_style_set_scale_end_color(&style_lmeter, LV_STATE_DEFAULT, LV_COLOR_GRAY);
  lv_style_set_line_width(&style_lmeter, LV_STATE_DEFAULT, LV_DPX(10));
  lv_style_set_scale_end_line_width(&style_lmeter, LV_STATE_DEFAULT, LV_DPX(7));

  style_init_reset(&style_chart_serie);
  lv_style_set_line_color(&style_chart_serie, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_style_set_line_width(&style_chart_serie, LV_STATE_DEFAULT, 4);
  lv_style_set_size(&style_chart_serie, LV_STATE_DEFAULT, 4);
  lv_style_set_bg_opa(&style_chart_serie, LV_STATE_DEFAULT, 0);

  lv_style_reset(&style_cb_bg);
  lv_style_set_radius(&style_cb_bg, LV_STATE_DEFAULT, LV_DPX(4));
  lv_style_set_pad_inner(&style_cb_bg, LV_STATE_DEFAULT, 18);

  lv_style_reset(&style_cb_bullet);
  lv_style_set_radius(&style_cb_bullet, LV_STATE_DEFAULT, LV_DPX(4));
  lv_style_set_pattern_image(&style_cb_bullet, LV_STATE_CHECKED, LV_SYMBOL_OK);
  lv_style_set_pattern_recolor(&style_cb_bullet, LV_STATE_CHECKED, LV_COLOR_WHITE);
  lv_style_set_pad_all(&style_cb_bullet, LV_STATE_DEFAULT, LV_DPX(8));
}

/**
 * Initialize the default
 * @param color_primary the primary color of the theme
 * @param color_secondary the secondary color for the theme
 * @param flags ORed flags starting with `LV_THEME_DEF_FLAG_...`
 * @param font_small pointer to a small font
 * @param font_normal pointer to a normal font
 * @param font_subtitle pointer to a large font
 * @param font_title pointer to a extra large font
 * @return a pointer to reference this theme later
 */
lv_theme_t* lv_pinetime_theme_init() {
  theme.color_primary = LV_COLOR_WHITE;
  theme.color_secondary = LV_COLOR_GRAY;
  theme.font_small = &jetbrains_mono_bold_20;
  theme.font_normal = &jetbrains_mono_bold_20;
  theme.font_subtitle = &jetbrains_mono_bold_20;
  theme.font_title = &jetbrains_mono_bold_20;
  theme.flags = 0;

  basic_init();

  theme.apply_xcb = theme_apply;

  inited = true;

  return &theme;
}

static void theme_apply(lv_obj_t* obj, lv_theme_style_t name) {
  lv_style_list_t* list;

  switch (name) {
    case LV_THEME_NONE:
      break;

    case LV_THEME_SCR:
      lv_obj_clean_style_list(obj, LV_OBJ_PART_MAIN);
      list = lv_obj_get_style_list(obj, LV_OBJ_PART_MAIN);
      _lv_style_list_add_style(list, &style_bg);
      _lv_style_list_add_style(list, &style_label_white);
      break;

    case LV_THEME_OBJ:
      lv_obj_clean_style_list(obj, LV_OBJ_PART_MAIN);
      list = lv_obj_get_style_list(obj, LV_OBJ_PART_MAIN);
      _lv_style_list_add_style(list, &style_box);
      break;

    case LV_THEME_CONT:
      lv_obj_clean_style_list(obj, LV_OBJ_PART_MAIN);
      list = lv_obj_get_style_list(obj, LV_CONT_PART_MAIN);
      _lv_style_list_add_style(list, &style_box);
      break;

    case LV_THEME_BTN:
      lv_obj_clean_style_list(obj, LV_BTN_PART_MAIN);
      list = lv_obj_get_style_list(obj, LV_BTN_PART_MAIN);
      _lv_style_list_add_style(list, &style_btn);
      break;

    case LV_THEME_BTNMATRIX:
      list = lv_obj_get_style_list(obj, LV_BTNMATRIX_PART_BG);
      _lv_style_list_add_style(list, &style_bg);
      _lv_style_list_add_style(list, &style_pad_small);

      list = lv_obj_get_style_list(obj, LV_BTNMATRIX_PART_BTN);
      _lv_style_list_add_style(list, &style_btn);
      break;

    case LV_THEME_BAR:
      lv_obj_clean_style_list(obj, LV_BAR_PART_BG);
      list = lv_obj_get_style_list(obj, LV_BAR_PART_BG);

      lv_obj_clean_style_list(obj, LV_BAR_PART_INDIC);
      list = lv_obj_get_style_list(obj, LV_BAR_PART_INDIC);
      _lv_style_list_add_style(list, &style_bar_indic);
      break;

    case LV_THEME_IMAGE:
      lv_obj_clean_style_list(obj, LV_IMG_PART_MAIN);
      list = lv_obj_get_style_list(obj, LV_IMG_PART_MAIN);
      _lv_style_list_add_style(list, &style_icon);
      break;

    case LV_THEME_LABEL:
      lv_obj_clean_style_list(obj, LV_LABEL_PART_MAIN);
      list = lv_obj_get_style_list(obj, LV_LABEL_PART_MAIN);
      _lv_style_list_add_style(list, &style_label_white);
      break;

    case LV_THEME_SLIDER:
      lv_obj_clean_style_list(obj, LV_SLIDER_PART_BG);
      list = lv_obj_get_style_list(obj, LV_SLIDER_PART_BG);
      _lv_style_list_add_style(list, &style_sw_bg);

      lv_obj_clean_style_list(obj, LV_SLIDER_PART_INDIC);
      list = lv_obj_get_style_list(obj, LV_SLIDER_PART_INDIC);

      lv_obj_clean_style_list(obj, LV_SLIDER_PART_KNOB);
      list = lv_obj_get_style_list(obj, LV_SLIDER_PART_KNOB);
      _lv_style_list_add_style(list, &style_slider_knob);
      break;

    case LV_THEME_LIST:
      lv_obj_clean_style_list(obj, LV_LIST_PART_BG);
      list = lv_obj_get_style_list(obj, LV_LIST_PART_BG);
      _lv_style_list_add_style(list, &style_box);

      lv_obj_clean_style_list(obj, LV_LIST_PART_SCROLLABLE);
      list = lv_obj_get_style_list(obj, LV_LIST_PART_SCROLLABLE);

      lv_obj_clean_style_list(obj, LV_LIST_PART_SCROLLBAR);
      list = lv_obj_get_style_list(obj, LV_LIST_PART_SCROLLBAR);
      _lv_style_list_add_style(list, &style_scrollbar);
      break;

    case LV_THEME_LIST_BTN:
      lv_obj_clean_style_list(obj, LV_BTN_PART_MAIN);
      list = lv_obj_get_style_list(obj, LV_BTN_PART_MAIN);
      _lv_style_list_add_style(list, &style_list_btn);
      break;

    case LV_THEME_ARC:
      lv_obj_clean_style_list(obj, LV_ARC_PART_BG);
      list = lv_obj_get_style_list(obj, LV_ARC_PART_BG);
      _lv_style_list_add_style(list, &style_arc_bg);

      lv_obj_clean_style_list(obj, LV_ARC_PART_INDIC);
      list = lv_obj_get_style_list(obj, LV_ARC_PART_INDIC);
      _lv_style_list_add_style(list, &style_arc_indic);

      lv_obj_clean_style_list(obj, LV_ARC_PART_KNOB);
      list = lv_obj_get_style_list(obj, LV_ARC_PART_KNOB);
      _lv_style_list_add_style(list, &style_arc_knob);
      break;

    case LV_THEME_SWITCH:
      lv_obj_clean_style_list(obj, LV_SWITCH_PART_BG);
      list = lv_obj_get_style_list(obj, LV_SWITCH_PART_BG);
      _lv_style_list_add_style(list, &style_sw_bg);

      lv_obj_clean_style_list(obj, LV_SWITCH_PART_INDIC);
      list = lv_obj_get_style_list(obj, LV_SWITCH_PART_INDIC);
      _lv_style_list_add_style(list, &style_sw_indic);

      lv_obj_clean_style_list(obj, LV_SWITCH_PART_KNOB);
      list = lv_obj_get_style_list(obj, LV_SWITCH_PART_KNOB);
      _lv_style_list_add_style(list, &style_sw_knob);
      break;

    case LV_THEME_DROPDOWN:
      lv_obj_clean_style_list(obj, LV_DROPDOWN_PART_MAIN);
      list = lv_obj_get_style_list(obj, LV_DROPDOWN_PART_MAIN);
      _lv_style_list_add_style(list, &style_btn);

      lv_obj_clean_style_list(obj, LV_DROPDOWN_PART_LIST);
      list = lv_obj_get_style_list(obj, LV_DROPDOWN_PART_LIST);
      _lv_style_list_add_style(list, &style_box);
      _lv_style_list_add_style(list, &style_ddlist_list);

      lv_obj_clean_style_list(obj, LV_DROPDOWN_PART_SELECTED);
      list = lv_obj_get_style_list(obj, LV_DROPDOWN_PART_SELECTED);
      _lv_style_list_add_style(list, &style_ddlist_selected);

      lv_obj_clean_style_list(obj, LV_DROPDOWN_PART_SCROLLBAR);
      list = lv_obj_get_style_list(obj, LV_DROPDOWN_PART_SCROLLBAR);
      _lv_style_list_add_style(list, &style_scrollbar);
      break;

    case LV_THEME_TABLE: {
      list = lv_obj_get_style_list(obj, LV_TABLE_PART_BG);
      _lv_style_list_add_style(list, &style_bg);

      int idx = 1; /* start value should be 1, not zero, since cell styles
                  start at 1 due to presence of LV_TABLE_PART_BG=0
                  in the enum (lv_table.h) */
      /* declaring idx outside loop to work with older compilers */
      for (; idx <= LV_TABLE_CELL_STYLE_CNT; idx++) {
        list = lv_obj_get_style_list(obj, idx);
        _lv_style_list_add_style(list, &style_table_cell);
        _lv_style_list_add_style(list, &style_label_white);
      }
    } break;

    case LV_THEME_LINEMETER:
      list = lv_obj_get_style_list(obj, LV_LINEMETER_PART_MAIN);
      _lv_style_list_add_style(list, &style_bg);
      _lv_style_list_add_style(list, &style_lmeter);
      break;

    case LV_THEME_CHART:
      lv_obj_clean_style_list(obj, LV_CHART_PART_SERIES);
      list = lv_obj_get_style_list(obj, LV_CHART_PART_SERIES);
      _lv_style_list_add_style(list, &style_btn);
      _lv_style_list_add_style(list, &style_chart_serie);
      break;

    case LV_THEME_CHECKBOX:
      list = lv_obj_get_style_list(obj, LV_CHECKBOX_PART_BG);
      _lv_style_list_add_style(list, &style_cb_bg);

      list = lv_obj_get_style_list(obj, LV_CHECKBOX_PART_BULLET);
      _lv_style_list_add_style(list, &style_btn);
      _lv_style_list_add_style(list, &style_cb_bullet);
      break;

    default:
      break;
  }

  lv_obj_refresh_style(obj, LV_OBJ_PART_ALL, LV_STYLE_PROP_ALL);
}
