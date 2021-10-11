/**
 * @file lv_pinetime_theme.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include <lvgl/lvgl.h> /*To see all the widgets*/

#if LV_USE_THEME_PINETIME

#include "lv_pinetime_theme.h"
//#include "../../../misc/lv_gc.h"
#include <lvgl/src/misc/lv_gc.h>

/*********************
 *      DEFINES
 *********************/

#define COLOR_WHITE   lv_color_white()
#define COLOR_BLACK   lv_color_black()
#define COLOR_LIGHT   lv_palette_lighten(LV_PALETTE_GREY, 3)
#define COLOR_MID     lv_palette_lighten(LV_PALETTE_GREY, 1)
#define COLOR_DARK    COLOR_BLACK
//#define COLOR_DARK    lv_palette_main(LV_PALETTE_GREY)
#define COLOR_DIM     lv_palette_darken(LV_PALETTE_GREY, 2)
#define PAD_DEF       LV_DPX(5)
#define RADIUS_ROUND  10

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void style_init_reset(lv_style_t * style);
static void theme_apply(lv_theme_t * th, lv_obj_t * obj);

/**********************
 *  STATIC VARIABLES
 **********************/
static lv_theme_t theme;

static lv_style_t style_pad;
static lv_style_t style_circle;

static lv_style_t style_bg;
static lv_style_t style_box;
static lv_style_t style_box_border;
static lv_style_t style_btn;
static lv_style_t style_btn_checked;
static lv_style_t style_btn_disabled;
static lv_style_t style_btn_disabled_checked;
static lv_style_t style_btn_border;
static lv_style_t style_label_white;
static lv_style_t style_icon;
static lv_style_t style_bar_indic;
static lv_style_t style_slider_knob;
static lv_style_t style_slider_knob_pressed;
static lv_style_t style_scrollbar;
static lv_style_t style_list_btn;
static lv_style_t style_list_btn_checked;
static lv_style_t style_list_btn_checked_pressed;
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
static lv_style_t style_bg_grad;
static lv_style_t style_lmeter;
static lv_style_t style_chart_series;
static lv_style_t style_cb_bg;
static lv_style_t style_cb_bullet;
static lv_style_t style_cb_bullet_focused;
static lv_style_t style_cb_bullet_checked;

static bool inited;

/**********************
 *      MACROS
 **********************/

/**********************
 *   STATIC FUNCTIONS
 **********************/

//static lv_color_t dark_color_filter_cb(const lv_color_filter_dsc_t * f, lv_color_t c, lv_opa_t opa)
//{
//    LV_UNUSED(f);
//    return lv_color_darken(c, opa);
//}


//static lv_color_t grey_filter_cb(const lv_color_filter_dsc_t * f, lv_color_t color, lv_opa_t opa)
//{
//    LV_UNUSED(f);
//    return lv_color_mix(lv_color_white(), color, opa);
//}

static void style_init(void)
{
  style_init_reset(&style_pad);
  lv_style_set_pad_top(&style_pad, LV_VER_RES / 30);
  lv_style_set_pad_bottom(&style_pad, LV_VER_RES / 30);
  lv_style_set_pad_left(&style_pad, LV_VER_RES / 40);
  lv_style_set_pad_right(&style_pad, LV_VER_RES / 40);
  
  style_init_reset(&style_circle);
  lv_style_set_radius(&style_circle, LV_RADIUS_CIRCLE);
  
  style_init_reset(&style_bg);
  lv_style_set_bg_opa(&style_bg, LV_OPA_COVER);
  lv_style_set_bg_color(&style_bg, COLOR_BLACK);
  lv_style_set_text_font(&style_bg, theme.font_normal);
  
  style_init_reset(&style_box);
  lv_style_set_bg_opa(&style_box, LV_OPA_COVER);
  lv_style_set_radius(&style_box, 10);
//  lv_style_set_value_color(&style_box, LV_PINETIME_BLUE);
//  lv_style_set_value_font(&style_box, theme.font_normal);
  
  style_init_reset(&style_box_border);
  lv_style_set_bg_opa(&style_box_border, LV_OPA_TRANSP);
  lv_style_set_border_width(&style_box_border, 2);
  lv_style_set_border_color(&style_box_border, LV_PINETIME_GRAY);
  lv_style_set_text_color(&style_box, LV_PINETIME_BLUE);
  
//  style_init_reset(&style_title);
//  lv_style_set_text_color(&style_title, LV_PINETIME_WHITE);
//  lv_style_set_text_font(&style_title, theme.font_subtitle);
  
  style_init_reset(&style_label_white);
  lv_style_set_text_color(&style_label_white, LV_PINETIME_WHITE);
  
  style_init_reset(&style_btn);
  lv_style_set_radius(&style_btn, 10);
  lv_style_set_bg_opa(&style_btn, LV_OPA_COVER);
  lv_style_set_bg_color(&style_btn, lv_color_hex(0x2F3540));
  lv_style_set_border_color(&style_btn, theme.color_primary);
  lv_style_set_border_width(&style_btn, 0);
  lv_style_set_text_color(&style_btn, COLOR_WHITE);
  lv_style_set_pad_left(&style_btn, LV_DPX(20));
  lv_style_set_pad_right(&style_btn, LV_DPX(20));
  lv_style_set_pad_top(&style_btn, LV_DPX(20));
  lv_style_set_pad_bottom(&style_btn, LV_DPX(20));
  lv_style_set_pad_gap(&style_btn, LV_DPX(15));
  lv_style_set_outline_width(&style_btn, LV_DPX(2));
  lv_style_set_outline_opa(&style_btn, LV_OPA_0);
  lv_style_set_outline_color(&style_btn, theme.color_primary);
  
  style_init_reset(&style_btn_checked);
  lv_style_set_border_opa(&style_btn_checked, LV_OPA_TRANSP);
  lv_style_set_bg_color(&style_btn_checked, lv_palette_main(LV_PALETTE_GREEN));
  lv_style_set_text_color(&style_btn_checked, lv_color_hex(0xffffff));
  
  style_init_reset(&style_btn_disabled);
  lv_style_set_bg_color(&style_btn_disabled, lv_color_hex(0x2F3540));
  lv_style_set_text_color(&style_btn_disabled, lv_color_hex(0x888888));
  
  style_init_reset(&style_btn_disabled_checked);
  lv_style_set_bg_color(&style_btn_disabled_checked, lv_color_hex3(0x888));
  
  style_init_reset(&style_btn_border);
  lv_style_set_radius(&style_btn_border, LV_RADIUS_CIRCLE);
  lv_style_set_border_color(&style_btn_border, LV_PINETIME_WHITE);
  lv_style_set_border_width(&style_btn_border, 2);
  lv_style_set_bg_opa(&style_btn_border, LV_OPA_TRANSP);
  lv_style_set_bg_color(&style_btn_border, LV_PINETIME_WHITE);
  lv_style_set_text_color(&style_btn_border, LV_PINETIME_WHITE);
  
  style_init_reset(&style_icon);
  lv_style_set_text_color(&style_icon, LV_PINETIME_WHITE);
  
  style_init_reset(&style_bar_indic);
  lv_style_set_bg_opa(&style_bar_indic, LV_OPA_COVER);
  lv_style_set_radius(&style_bar_indic, 10);
  
  style_init_reset(&style_scrollbar);
  lv_style_set_bg_opa(&style_scrollbar, LV_OPA_COVER);
  lv_style_set_radius(&style_scrollbar, LV_RADIUS_CIRCLE);
  lv_style_set_bg_color(&style_scrollbar, LV_PINETIME_LIGHT_GRAY);
  lv_style_set_size(&style_scrollbar, LV_HOR_RES / 80);
  lv_style_set_pad_right(&style_scrollbar, LV_HOR_RES / 60);
  
  style_init_reset(&style_list_btn);
  lv_style_set_bg_opa(&style_list_btn, LV_OPA_COVER);
  lv_style_set_bg_color(&style_list_btn, LV_PINETIME_WHITE);
  lv_style_set_text_color(&style_list_btn, LV_PINETIME_BLUE);
  lv_style_set_img_recolor(&style_list_btn, LV_PINETIME_BLUE);
  lv_style_set_pad_left(&style_list_btn, LV_HOR_RES / 25);
  lv_style_set_pad_right(&style_list_btn, LV_HOR_RES / 25);
  lv_style_set_pad_top(&style_list_btn, LV_HOR_RES / 100);
  lv_style_set_pad_bottom(&style_list_btn, LV_HOR_RES / 100);
  lv_style_set_pad_gap(&style_list_btn, LV_HOR_RES / 50);
  
  style_init_reset(&style_list_btn_checked);
  lv_style_set_bg_color(&style_list_btn_checked, LV_PINETIME_GRAY);
  lv_style_set_text_color(&style_list_btn_checked, LV_PINETIME_WHITE);
  lv_style_set_img_recolor(&style_list_btn_checked, LV_PINETIME_WHITE);
  
  style_init_reset(&style_list_btn_checked_pressed);
  lv_style_set_bg_color(&style_list_btn_checked_pressed, lv_color_darken(LV_PINETIME_GRAY, LV_OPA_20));
  lv_style_set_text_color(&style_list_btn_checked_pressed, LV_PINETIME_WHITE);
  lv_style_set_img_recolor(&style_list_btn_checked_pressed, LV_PINETIME_WHITE);
  
  style_init_reset(&style_ddlist_list);
  lv_style_set_text_line_space(&style_ddlist_list, LV_VER_RES / 25);
  lv_style_set_shadow_width(&style_ddlist_list, LV_VER_RES / 20);
  lv_style_set_shadow_color(&style_ddlist_list, LV_PINETIME_GRAY);
  lv_style_set_bg_color(&style_ddlist_list, LV_PINETIME_GRAY);
  
  style_init_reset(&style_ddlist_selected);
  lv_style_set_bg_opa(&style_ddlist_selected, LV_OPA_COVER);
  lv_style_set_bg_color(&style_ddlist_selected, LV_PINETIME_BLUE);
  
  style_init_reset(&style_sw_bg);
  lv_style_set_bg_opa(&style_sw_bg, LV_OPA_COVER);
  lv_style_set_bg_color(&style_sw_bg, LV_PINETIME_LIGHT_GRAY);
  lv_style_set_radius(&style_sw_bg, LV_RADIUS_CIRCLE);
  
  style_init_reset(&style_sw_indic);
  lv_style_set_bg_opa(&style_sw_indic, LV_OPA_COVER);
  lv_style_set_bg_color(&style_sw_indic, LV_PINETIME_GREEN);
  
  style_init_reset(&style_sw_knob);
  lv_style_set_bg_opa(&style_sw_knob, LV_OPA_COVER);
  lv_style_set_bg_color(&style_sw_knob, LV_PINETIME_WHITE);
  lv_style_set_radius(&style_sw_knob, LV_RADIUS_CIRCLE);
  lv_style_set_pad_top(&style_sw_knob, -4);
  lv_style_set_pad_bottom(&style_sw_knob, -4);
  lv_style_set_pad_left(&style_sw_knob, -4);
  lv_style_set_pad_right(&style_sw_knob, -4);
  
  style_init_reset(&style_slider_knob);
  lv_style_set_bg_opa(&style_slider_knob, LV_OPA_COVER);
  lv_style_set_bg_color(&style_slider_knob, lv_palette_main(LV_PALETTE_RED));
  lv_style_set_border_color(&style_slider_knob, COLOR_WHITE);
  lv_style_set_border_width(&style_slider_knob, 6);
  lv_style_set_radius(&style_slider_knob, LV_RADIUS_CIRCLE);
  lv_style_set_pad_top(&style_slider_knob, 10);
  lv_style_set_pad_bottom(&style_slider_knob, 10);
  lv_style_set_pad_left(&style_slider_knob, 10);
  lv_style_set_pad_right(&style_slider_knob, 10);
  
  style_init_reset(&style_slider_knob_pressed);
  lv_style_set_pad_top(&style_slider_knob_pressed, 14);
  lv_style_set_pad_bottom(&style_slider_knob_pressed, 14);
  lv_style_set_pad_left(&style_slider_knob_pressed, 14);
  lv_style_set_pad_right(&style_slider_knob_pressed, 14);
  
  style_init_reset(&style_arc_indic);
  lv_style_set_line_color(&style_arc_indic, LV_PINETIME_BLUE);
  lv_style_set_line_width(&style_arc_indic, LV_DPX(25));
  lv_style_set_line_rounded(&style_arc_indic, true);
  
  style_init_reset(&style_arc_bg);
  lv_style_set_line_color(&style_arc_bg, LV_PINETIME_GRAY);
  lv_style_set_line_width(&style_arc_bg, LV_DPX(25));
  lv_style_set_line_rounded(&style_arc_bg, true);
  lv_style_set_pad_all(&style_arc_bg, LV_DPX(5));
  
  lv_style_reset(&style_arc_knob);
  lv_style_set_radius(&style_arc_knob, LV_RADIUS_CIRCLE);
  lv_style_set_bg_opa(&style_arc_knob, LV_OPA_COVER);
  lv_style_set_bg_color(&style_arc_knob, LV_PINETIME_LIGHT_GRAY);
  lv_style_set_pad_all(&style_arc_knob, LV_DPX(5));
  
  style_init_reset(&style_table_cell);
  lv_style_set_border_color(&style_table_cell, LV_PINETIME_GRAY);
  lv_style_set_border_width(&style_table_cell, 1);
  lv_style_set_border_side(&style_table_cell, LV_BORDER_SIDE_FULL);
  lv_style_set_pad_left(&style_table_cell, 5);
  lv_style_set_pad_right(&style_table_cell, 5);
  lv_style_set_pad_top(&style_table_cell, 2);
  lv_style_set_pad_bottom(&style_table_cell, 2);
  
  style_init_reset(&style_pad_small);
  int32_t pad_small_value = 10;
  lv_style_set_pad_left(&style_pad_small, pad_small_value);
  lv_style_set_pad_right(&style_pad_small, pad_small_value);
  lv_style_set_pad_top(&style_pad_small, pad_small_value);
  lv_style_set_pad_bottom(&style_pad_small, pad_small_value);
  lv_style_set_pad_gap(&style_pad_small, pad_small_value);
  
  style_init_reset(&style_bg_grad);
  lv_style_set_bg_color(&style_bg_grad, lv_color_hsv_to_rgb(10, 10, 40));
  lv_style_set_bg_grad_color(&style_bg_grad, lv_color_hsv_to_rgb(10, 10, 20));
  lv_style_set_bg_grad_dir(&style_bg_grad, LV_GRAD_DIR_VER);
  
  style_init_reset(&style_lmeter);
  lv_style_set_radius(&style_lmeter, LV_RADIUS_CIRCLE);
  lv_style_set_pad_left(&style_lmeter, LV_DPX(20));
  lv_style_set_pad_right(&style_lmeter, LV_DPX(20));
  lv_style_set_pad_top(&style_lmeter, LV_DPX(20));
  lv_style_set_pad_gap(&style_lmeter, LV_DPX(30));
//  lv_style_set_scale_width(&style_lmeter, LV_DPX(25));
  
  lv_style_set_line_color(&style_lmeter, theme.color_primary);
//  lv_style_set_scale_grad_color(&style_lmeter, theme.color_primary);
//  lv_style_set_scale_end_color(&style_lmeter, lv_color_hex3(0x888));
  lv_style_set_line_width(&style_lmeter, LV_DPX(10));
//  lv_style_set_scale_end_line_width(&style_lmeter, LV_DPX(7));
  
  style_init_reset(&style_chart_series);
  lv_style_set_line_color(&style_chart_series, LV_PINETIME_WHITE);
  lv_style_set_line_width(&style_chart_series, 4);
  lv_style_set_size(&style_chart_series, 4);
  lv_style_set_bg_opa(&style_chart_series, 0);
  
  lv_style_reset(&style_cb_bg);
  lv_style_set_radius(&style_cb_bg, LV_DPX(4));
  lv_style_set_pad_gap(&style_cb_bg, LV_DPX(10));
  lv_style_set_outline_color(&style_cb_bg, theme.color_primary);
  lv_style_set_outline_width(&style_cb_bg, LV_DPX(2));
  lv_style_set_outline_pad(&style_cb_bg, LV_DPX(20));
  lv_style_set_outline_opa(&style_cb_bg, LV_OPA_0);
  
  lv_style_reset(&style_cb_bullet);
  lv_style_set_radius(&style_cb_bullet, LV_DPX(4));
  lv_style_set_pad_left(&style_cb_bullet, LV_DPX(8));
  lv_style_set_pad_right(&style_cb_bullet, LV_DPX(8));
  lv_style_set_pad_top(&style_cb_bullet, LV_DPX(8));
  lv_style_set_pad_bottom(&style_cb_bullet, LV_DPX(8));
  
  lv_style_reset(&style_cb_bullet_focused);
  lv_style_set_outline_opa(&style_cb_bullet_focused, LV_OPA_TRANSP);
  
  lv_style_reset(&style_cb_bullet_checked);
  lv_style_set_bg_img_recolor(&style_cb_bullet_checked, lv_color_white());
}


/**********************
 *   GLOBAL FUNCTIONS
 **********************/

lv_theme_t * lv_pinetime_theme_init(lv_disp_t * disp,
                                   lv_color_t color_primary,
                                   lv_color_t color_secondary,
                                   const lv_font_t* font)
{
    theme.color_primary = color_primary;
    theme.color_secondary = color_secondary;
    theme.disp = disp;
    theme.font_small = font;
    theme.font_normal = font;
    theme.font_large = font;
    
    theme.apply_cb = theme_apply;

    style_init();

    inited = true;

    if(disp == NULL || lv_disp_get_theme(disp) == &theme) lv_obj_report_style_change(NULL);

    return &theme;
}

bool lv_pinetime_theme_is_inited(void)
{
    return inited;
}


static void theme_apply(lv_theme_t * th, lv_obj_t * obj)
{
  LV_UNUSED(th);

  if (obj == NULL) {
    return;
  }

  if (lv_obj_get_parent(obj) == NULL) {
    lv_obj_add_style(obj, &style_bg, 0);
    lv_obj_add_style(obj, &style_label_white, 0);
    return;
  }

  if (lv_obj_check_type(obj, &lv_obj_class)) {
    lv_obj_add_style(obj, &style_box, 0);
  }
  #if LV_USE_BTN
  else if (lv_obj_check_type(obj, &lv_btn_class)) {
    lv_obj_add_style(obj, &style_btn, 0);
    lv_obj_add_style(obj, &style_btn_checked, LV_STATE_CHECKED);
    lv_obj_add_style(obj, &style_btn_disabled, LV_STATE_DISABLED);
    lv_obj_add_style(obj, &style_btn_disabled_checked, LV_STATE_DISABLED | LV_STATE_CHECKED);
  }
  #endif

  #if LV_USE_BTNMATRIX
  else if (lv_obj_check_type(obj, &lv_btnmatrix_class)) {
    lv_obj_add_style(obj, &style_bg, 0);
    lv_obj_add_style(obj, &style_pad_small, 0);
    lv_obj_add_style(obj, &style_btn, LV_PART_ITEMS);
    lv_obj_add_style(obj, &style_btn_checked, LV_PART_ITEMS | LV_STATE_CHECKED);
    lv_obj_add_style(obj, &style_btn_disabled, LV_PART_ITEMS | LV_STATE_DISABLED);
    lv_obj_add_style(obj, &style_btn_disabled_checked, LV_PART_ITEMS | LV_STATE_DISABLED | LV_STATE_CHECKED);
  }
  #endif

  #if LV_USE_BAR
  else if (lv_obj_check_type(obj, &lv_bar_class)) {
    lv_obj_add_style(obj, &style_bar_indic, LV_PART_INDICATOR);
  }
  #endif

  #if LV_USE_SLIDER
  else if (lv_obj_check_type(obj, &lv_slider_class)) {
    lv_obj_add_style(obj, &style_sw_bg, 0);
    lv_obj_add_style(obj, &style_sw_knob, LV_PART_KNOB);
  }
  #endif

  #if LV_USE_TABLE
  else if (lv_obj_check_type(obj, &lv_table_class)) {
    lv_obj_add_style(obj, &style_bg, 0);
    lv_obj_add_style(obj, &style_table_cell, LV_PART_ITEMS);
    lv_obj_add_style(obj, &style_label_white, LV_PART_ITEMS);
  }
  #endif

  #if LV_USE_CHECKBOX
  else if (lv_obj_check_type(obj, &lv_checkbox_class)) {
    lv_obj_add_style(obj, &style_cb_bg, 0);
    
    lv_obj_add_style(obj, &style_btn, LV_PART_INDICATOR);
    lv_obj_add_style(obj, &style_btn_checked, LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_add_style(obj, &style_btn_disabled, LV_PART_INDICATOR | LV_STATE_DISABLED);
    lv_obj_add_style(obj, &style_btn_disabled_checked, LV_PART_INDICATOR | LV_STATE_DISABLED | LV_STATE_CHECKED);
    
    lv_obj_add_style(obj, &style_cb_bullet, LV_PART_INDICATOR);
    lv_obj_add_style(obj, &style_cb_bullet_checked, LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_add_style(obj, &style_cb_bullet_focused, LV_PART_INDICATOR | LV_STATE_FOCUSED);
  }
  #endif

  #if LV_USE_SWITCH
  else if (lv_obj_check_type(obj, &lv_switch_class)) {
    lv_obj_add_style(obj, &style_sw_bg, 0);
    lv_obj_add_style(obj, &style_sw_indic, LV_PART_INDICATOR);
    lv_obj_add_style(obj, &style_sw_knob, LV_PART_KNOB);
  }
  #endif

  #if LV_USE_CHART
  else if (lv_obj_check_type(obj, &lv_chart_class)) {
    lv_obj_add_style(obj, &style_btn, 0);
    lv_obj_add_style(obj, &style_btn_checked, LV_STATE_CHECKED);
    lv_obj_add_style(obj, &style_btn_disabled, LV_STATE_DISABLED);
    lv_obj_add_style(obj, &style_btn_disabled_checked, LV_STATE_DISABLED | LV_STATE_CHECKED);
    lv_obj_add_style(obj, &style_chart_series, 0);
  }
  #endif

  #if LV_USE_DROPDOWN
  else if (lv_obj_check_type(obj, &lv_dropdown_class)) {
    lv_obj_add_style(obj, &style_btn, 0);
    lv_obj_add_style(obj, &style_pad, 0);
    lv_obj_add_style(obj, &style_btn_checked, LV_STATE_CHECKED);
    lv_obj_add_style(obj, &style_btn_disabled, LV_STATE_DISABLED);
    lv_obj_add_style(obj, &style_btn_disabled_checked, LV_STATE_DISABLED | LV_STATE_CHECKED);
  } else if (lv_obj_check_type(obj, &lv_dropdownlist_class)) {
    lv_obj_add_style(obj, &style_box, 0);
    lv_obj_add_style(obj, &style_ddlist_list, 0);
    lv_obj_add_style(obj, &style_pad, 0);
    lv_obj_add_style(obj, &style_ddlist_selected, LV_PART_SELECTED);
    lv_obj_add_style(obj, &style_scrollbar, LV_PART_SCROLLBAR);
  }
  #endif

  #if LV_USE_ARC
  else if (lv_obj_check_type(obj, &lv_arc_class)) {
    lv_obj_add_style(obj, &style_arc_bg, 0);
    lv_obj_add_style(obj, &style_arc_indic, LV_PART_INDICATOR);
    lv_obj_add_style(obj, &style_arc_knob, LV_PART_KNOB);
  }
  #endif

  #if LV_USE_METER
  else if (lv_obj_check_type(obj, &lv_meter_class)) {
    lv_obj_add_style(obj, &style_bg, 0);
    lv_obj_add_style(obj, &style_lmeter, 0);
  }
  #endif

  #if LV_USE_CALENDAR
  else if (lv_obj_check_type(obj, &lv_calendar_class)) {
//    lv_obj_add_style(obj, &styles->light, 0);
//    lv_obj_add_style(obj, &styles->light, LV_PART_ITEMS | LV_STATE_PRESSED);
//    lv_obj_add_style(obj, &styles->pressed, LV_PART_ITEMS | LV_STATE_PRESSED);
//    lv_obj_add_style(obj, &styles->disabled, LV_PART_ITEMS | LV_STATE_DISABLED);
  }
  #endif

  #if LV_USE_LIST
  else if (lv_obj_check_type(obj, &lv_list_class)) {
    lv_obj_add_style(obj, &style_box, 0);
    lv_obj_add_style(obj, &style_scrollbar, LV_PART_SCROLLBAR);
  } else if (lv_obj_check_type(obj, &lv_list_text_class)) {

  } else if (lv_obj_check_type(obj, &lv_list_btn_class)) {
    lv_obj_add_style(obj, &style_list_btn, 0);
    lv_obj_add_style(obj, &style_list_btn_checked, LV_STATE_CHECKED);
    lv_obj_add_style(obj, &style_list_btn_checked_pressed, LV_STATE_CHECKED | LV_STATE_PRESSED);
  }
  #endif
  
  lv_obj_refresh_style(obj, LV_PART_ANY, LV_STYLE_PROP_ANY);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void style_init_reset(lv_style_t * style)
{
    if(inited) {
      lv_style_reset(style);
    } else{
      lv_style_init(style);
    }
}

#endif
