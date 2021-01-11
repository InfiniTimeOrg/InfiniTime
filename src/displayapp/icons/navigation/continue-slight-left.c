#if defined(LV_LVGL_H_INCLUDE_SIMPLE)
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif


#ifndef LV_ATTRIBUTE_MEM_ALIGN
#define LV_ATTRIBUTE_MEM_ALIGN
#endif

#ifndef LV_ATTRIBUTE_IMG_CONTINUE_SLIGHT_LEFT
#define LV_ATTRIBUTE_IMG_CONTINUE_SLIGHT_LEFT
#endif

const LV_ATTRIBUTE_MEM_ALIGN LV_ATTRIBUTE_LARGE_CONST LV_ATTRIBUTE_IMG_CONTINUE_SLIGHT_LEFT uint8_t continue_slight_left_map[] = {
  0x04, 0x02, 0x04, 0xff, 	/*Color of index 0*/
  0xfc, 0xfe, 0xfc, 0xff, 	/*Color of index 1*/

  0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 
  0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0x80, 0x00, 
  0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0x80, 0x00, 
  0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0x80, 0x00, 
  0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0x80, 0x00, 
  0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0x80, 0x00, 
  0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0x80, 0x00, 
  0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 
  0x00, 0x1f, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x00, 
  0x00, 0x1f, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x00, 
  0x00, 0x1f, 0xff, 0xff, 0xff, 0x80, 0x00, 0x00, 
  0x00, 0x1f, 0xff, 0xff, 0xfc, 0x00, 0x00, 0x00, 
  0x00, 0x1f, 0xff, 0xff, 0xf0, 0x00, 0x00, 0x00, 
  0x00, 0x0f, 0xff, 0xff, 0xfc, 0x00, 0x00, 0x00, 
  0x00, 0x0f, 0xff, 0xff, 0xfc, 0x00, 0x00, 0x00, 
  0x00, 0x0f, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 
  0x00, 0x0f, 0xff, 0xff, 0xff, 0x80, 0x00, 0x00, 
  0x00, 0x0f, 0xff, 0xff, 0xff, 0xc0, 0x00, 0x00, 
  0x00, 0x07, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x00, 
  0x00, 0x07, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x00, 
  0x00, 0x07, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x00, 
  0x00, 0x07, 0xff, 0xff, 0xff, 0xfe, 0x00, 0x00, 
  0x00, 0x07, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 
  0x00, 0x03, 0xff, 0xff, 0xff, 0xff, 0x80, 0x00, 
  0x00, 0x03, 0xff, 0xc7, 0xff, 0xff, 0x80, 0x00, 
  0x00, 0x03, 0xff, 0xc3, 0xff, 0xff, 0xc0, 0x00, 
  0x00, 0x03, 0xff, 0xc1, 0xff, 0xff, 0xe0, 0x00, 
  0x00, 0x03, 0xff, 0x80, 0xff, 0xff, 0xe0, 0x00, 
  0x00, 0x01, 0xff, 0x80, 0x7f, 0xff, 0xf0, 0x00, 
  0x00, 0x01, 0xff, 0x80, 0x1f, 0xff, 0xf8, 0x00, 
  0x00, 0x01, 0xff, 0x80, 0x0f, 0xff, 0xf8, 0x00, 
  0x00, 0x01, 0xff, 0x00, 0x07, 0xff, 0xf8, 0x00, 
  0x00, 0x01, 0xff, 0x00, 0x03, 0xff, 0xfc, 0x00, 
  0x00, 0x00, 0xff, 0x00, 0x01, 0xff, 0xfc, 0x00, 
  0x00, 0x00, 0xfe, 0x00, 0x01, 0xff, 0xfc, 0x00, 
  0x00, 0x00, 0xfe, 0x00, 0x00, 0xff, 0xfc, 0x00, 
  0x00, 0x00, 0xfe, 0x00, 0x00, 0x7f, 0xfc, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xfc, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xfc, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xfc, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xfc, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xfc, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xfc, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xfc, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xfc, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xfc, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xfc, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xfc, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xfc, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xfc, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xfc, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xfc, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xfc, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xfc, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xfc, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xfc, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xfc, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xfc, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xfc, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xfc, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xfc, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xfc, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xfc, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xfc, 0x00, 
};

const lv_img_dsc_t continue_slight_left = {
      {
        LV_IMG_CF_INDEXED_1BIT,
        0,
        0,
        64,
        64
    },
    520,
continue_slight_left_map,
};
