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
#pragma once

#include <lvgl/lvgl.h>

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

void lv_disp_set_direction(_disp_t* disp, uint_least8_t direction) {

}
