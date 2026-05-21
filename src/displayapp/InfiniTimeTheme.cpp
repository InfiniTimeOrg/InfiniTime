#include "displayapp/InfiniTimeTheme.h"
#include <libraries/log/nrf_log.h>
#include "components/fs/FS.h"
#include <algorithm>
#include <cstring>

// Replace LV_DPX with a constexpr version using a constant LV_DPI
#undef LV_DPX

namespace {
  constexpr int LV_DPX(int n) {
    if (n == 0) {
      return 0;
    }
    return std::max(((LV_DPI * n + 80) / 160), 1); /*+80 for rounding*/
  }

  /**
   * Parse hex color string (RRGGBB format) to lv_color_t
   * Returns the parsed color or defaultColor if parsing fails
   */
  lv_color_t parseHexColor(const char* hexStr, lv_color_t defaultColor) {
    if (!hexStr || std::strlen(hexStr) < 6) {
      return defaultColor;
    }

    char* endptr;
    uint32_t value = std::strtoul(hexStr, &endptr, 16);

    // Check if conversion was successful and we consumed all characters
    if (endptr == hexStr || *endptr != '\0' || value > 0xFFFFFF) {
      return defaultColor;
    }

    uint8_t r = (value >> 16) & 0xFF;
    uint8_t g = (value >> 8) & 0xFF;
    uint8_t b = value & 0xFF;

    lv_color_t result = LV_COLOR_MAKE(r, g, b);

    return result;
  }

  /**
   * Parse a configuration line in format: KEY=VALUE
   * Lines starting with # are treated as comments
   * Returns true if the line was successfully parsed
   */
  bool parseLine(const char* line, char* key, char* value, size_t maxKeyLen, size_t maxValueLen) {
    // Skip whitespace and comments
    while (*line == ' ' || *line == '\t') {
      line++;
    }
    if (*line == '#' || *line == '\0' || *line == '\n' || *line == '\r') {
      return false;
    }

    // Parse key
    size_t keyLen = 0;
    while (*line != '=' && *line != '\0' && *line != '\n' && keyLen < maxKeyLen - 1) {
      char c = *line++;
      // Trim trailing whitespace from key
      if (c != ' ' && c != '\t') {
        key[keyLen++] = c;
      } else if (keyLen > 0 && key[keyLen - 1] != ' ') {
        key[keyLen++] = c;
      }
    }

    // Trim trailing whitespace from key
    while (keyLen > 0 && (key[keyLen - 1] == ' ' || key[keyLen - 1] == '\t')) {
      keyLen--;
    }

    key[keyLen] = '\0';

    if (*line != '=' || keyLen == 0) {
      return false;
    }
    line++; // Skip '='

    // Skip leading whitespace in value
    while (*line == ' ' || *line == '\t') {
      line++;
    }

    // Parse value
    size_t valueLen = 0;
    while (*line != '\0' && *line != '\n' && *line != '\r' && valueLen < maxValueLen - 1) {
      value[valueLen++] = *line++;
    }
    value[valueLen] = '\0';

    return true;
  }

  /**
   * Load colors from /themes/theme.cfg file in LittleFS
   * File format:
   * # This is a comment
   * bg=5D697E
   * accent=383838
   * accent_dark=181818
   * highlight=00B000
   * color_primary=FFFFFF
   * color_secondary=808080
   */
  void loadThemeConfig(Pinetime::Controllers::FS* filesystem) {
    if (!filesystem) {
      return;
    }

    lfs_file_t file;
    if (filesystem->FileOpen(&file, "/themes/theme.cfg", LFS_O_RDONLY) != LFS_ERR_OK) {
      NRF_LOG_INFO("loadThemeConfig: Failed to open /themes/theme.cfg");
      return;
    }

    // Read the entire file into a buffer (reasonable limit for config file)
    constexpr size_t maxConfigSize = 1024;
    uint8_t buffer[maxConfigSize];
    lfs_ssize_t bytesRead = filesystem->FileRead(&file, buffer, maxConfigSize - 1);
    filesystem->FileClose(&file);

    if (bytesRead <= 0) {
      return;
    }

    buffer[bytesRead] = '\0';
    const char* configData = reinterpret_cast<const char*>(buffer);

    // Parse the configuration file line by line
    char line[256];
    char key[64];
    char value[64];
    size_t lineStart = 0;

    while (lineStart < static_cast<size_t>(bytesRead)) {
      size_t lineEnd = lineStart;
      // Find the end of the line
      while (lineEnd < static_cast<size_t>(bytesRead) && configData[lineEnd] != '\n') {
        lineEnd++;
      }

      size_t lineLen = lineEnd - lineStart;
      if (lineLen >= sizeof(line)) {
        lineLen = sizeof(line) - 1;
      }

      std::memcpy(line, &configData[lineStart], lineLen);
      line[lineLen] = '\0';

      if (parseLine(line, key, value, sizeof(key), sizeof(value))) {
        if (std::strcmp(key, "accent_light") == 0) {
          Colors::accent_light = parseHexColor(value, Colors::accent_light);
        } else if (std::strcmp(key, "accent") == 0) {
          Colors::accent = parseHexColor(value, Colors::accent);
        } else if (std::strcmp(key, "accent_dark") == 0) {
          Colors::accent_dark = parseHexColor(value, Colors::accent_dark);
        } else if (std::strcmp(key, "highlight") == 0) {
          Colors::highlight = parseHexColor(value, Colors::highlight);
        } else if (std::strcmp(key, "text_primary") == 0) {
          Colors::text_primary = parseHexColor(value, Colors::text_primary);
        } else if (std::strcmp(key, "text_header") == 0) {
          Colors::text_header = parseHexColor(value, Colors::text_header);
        } else if (std::strcmp(key, "page_bg") == 0) {
          Colors::page_bg = parseHexColor(value, Colors::page_bg);
        } else if (std::strcmp(key, "icon") == 0) {
          Colors::icon = parseHexColor(value, Colors::icon);
        }
      }

      lineStart = lineEnd + 1; // Skip the newline character
    }
  }
}

static Pinetime::Controllers::FS* themeFilesystem = nullptr;

// Helper function to recursively refresh styles of an object and all its children
static void refresh_object_tree(lv_obj_t* obj) {
  if (obj == nullptr) {
    return;
  }

  // Refresh this object
  lv_obj_refresh_style(obj, LV_OBJ_PART_ALL, LV_STYLE_PROP_ALL);

  // Recursively refresh all children
  lv_obj_t* child = lv_obj_get_child(obj, nullptr);
  while (child != nullptr) {
    refresh_object_tree(child);
    child = lv_obj_get_child(obj, child);
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
  lv_style_set_bg_color(&style_bg, LV_STATE_DEFAULT, Colors::page_bg);
  lv_style_set_text_font(&style_bg, LV_STATE_DEFAULT, theme.font_normal);

  style_init_reset(&style_box);
  lv_style_set_bg_opa(&style_box, LV_STATE_DEFAULT, LV_OPA_COVER);
  lv_style_set_radius(&style_box, LV_STATE_DEFAULT, 10);
  lv_style_set_value_font(&style_box, LV_STATE_DEFAULT, theme.font_normal);

  style_init_reset(&style_label_white);
  lv_style_set_text_color(&style_label_white, LV_STATE_DEFAULT, Colors::text_primary);
  lv_style_set_text_color(&style_label_white, LV_STATE_DISABLED, LV_COLOR_GRAY);

  style_init_reset(&style_btn);
  lv_style_set_radius(&style_btn, LV_STATE_DEFAULT, 10);
  lv_style_set_bg_opa(&style_btn, LV_STATE_DEFAULT, LV_OPA_COVER);
  lv_style_set_bg_color(&style_btn, LV_STATE_DEFAULT, Colors::accent_light);
  lv_style_set_bg_color(&style_btn, LV_STATE_CHECKED, Colors::highlight);
  lv_style_set_bg_color(&style_btn, LV_STATE_DISABLED, Colors::accent_dark);

  lv_style_set_text_color(&style_btn, LV_STATE_DEFAULT, Colors::text_primary);
  lv_style_set_text_color(&style_btn, LV_STATE_DISABLED, LV_COLOR_GRAY);

  lv_style_set_pad_all(&style_btn, LV_STATE_DEFAULT, LV_DPX(20));
  lv_style_set_pad_inner(&style_btn, LV_STATE_DEFAULT, LV_DPX(15));

  style_init_reset(&style_icon);
  lv_style_set_text_color(&style_icon, LV_STATE_DEFAULT, Colors::text_primary);

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
  lv_style_set_text_color(&style_list_btn, LV_STATE_DEFAULT, Colors::accent_light);
  lv_style_set_text_color(&style_list_btn, LV_STATE_CHECKED, LV_COLOR_WHITE);
  lv_style_set_image_recolor(&style_list_btn, LV_STATE_DEFAULT, Colors::accent_light);
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
  lv_style_set_bg_color(&style_ddlist_selected, LV_STATE_DEFAULT, Colors::accent_light);

  style_init_reset(&style_sw_bg);
  lv_style_set_bg_opa(&style_sw_bg, LV_STATE_DEFAULT, LV_OPA_COVER);
  lv_style_set_bg_color(&style_sw_bg, LV_STATE_DEFAULT, Colors::accent_light);
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
  lv_style_set_line_color(&style_arc_indic, LV_STATE_DEFAULT, Colors::accent_light);
  lv_style_set_line_width(&style_arc_indic, LV_STATE_DEFAULT, LV_DPX(25));
  lv_style_set_line_rounded(&style_arc_indic, LV_STATE_DEFAULT, true);

  style_init_reset(&style_arc_bg);
  lv_style_set_line_color(&style_arc_bg, LV_STATE_DEFAULT, Colors::accent_dark);
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
 * @return a pointer to reference this theme later
 */
lv_theme_t* lv_pinetime_theme_init(Pinetime::Controllers::FS* filesystem) {

  // Set the filesystem pointer if provided
  if (filesystem != nullptr) {
    themeFilesystem = filesystem;
    loadThemeConfig(filesystem);
  }

  theme.color_primary = Colors::text_primary;
  theme.color_secondary = Colors::text_header;
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

void lv_pinetime_theme_set_filesystem(Pinetime::Controllers::FS* filesystem) {

  themeFilesystem = filesystem;

  // If theme was already initialized and we now have a filesystem, try to reload colors
  if (inited && filesystem != nullptr) {
    loadThemeConfig(filesystem);
  }
}

void lv_pinetime_theme_reload_config() {
  if (themeFilesystem != nullptr) {
    loadThemeConfig(themeFilesystem);

    // Update the theme struct with new colors
    theme.color_primary = Colors::text_primary;
    theme.color_secondary = Colors::text_header;

    // Reinitialize the theme styles with the new colors
    basic_init();

    // Refresh all objects on screen to apply the new styles
    lv_obj_t* scr = lv_scr_act();
    if (scr != nullptr) {
      refresh_object_tree(scr);
      lv_obj_invalidate(scr);
    }
  }
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

bool lv_pinetime_theme_is_custom_loaded() {
  if (themeFilesystem == nullptr) {
    return false;
  }

  // Simple heuristic: if primary color differs from white, we likely loaded custom theme
  // White is the default color_primary in the header
  if (Colors::text_primary.full != LV_COLOR_WHITE.full) {
    return true;
  }

  // Additional check: if bg color differs from default
  // Default is LV_COLOR_MAKE(0x5d, 0x69, 0x7e)
  const lv_color_t defaultBg = LV_COLOR_MAKE(0x5d, 0x69, 0x7e);
  if (Colors::accent_light.full != defaultBg.full) {
    return true;
  }

  return false;
}
