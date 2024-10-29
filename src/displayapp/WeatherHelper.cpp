/*  Copyright (C) 2024 Caleb Fontenot

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

#include "WeatherHelper.h"
#include <FreeRTOS.h>
#include <lvgl/src/lv_misc/lv_color.h>
#include <tuple>
#include <vector>
#include <array>
#include <cstdint>
#include <cstdio>
#include <algorithm>
#include <nrfx_log.h>

using namespace Pinetime::Applications;
    //Linear gradient temperature color calculator :)
    
    int16_t WeatherHelper::RoundTemperature(int16_t temp) {
      return temp = temp / 100 + (temp % 100 >= 50 ? 1 : 0);
    }
    std::tuple<int, int, int> rgb565to888(int r, int g, int b) {
        return {
        ( r * 527 + 23 ) >> 6,
        ( g * 259 + 33 ) >> 6,
        ( b * 527 + 23 ) >> 6
      };
    }
  
    const char* WeatherHelper::floatToRgbHex(lv_color_t rgb) {
      std::tuple<int, int, int> tuple = rgb565to888(LV_COLOR_GET_R(rgb), LV_COLOR_GET_G(rgb), LV_COLOR_GET_B(rgb));
      char *rgbHex = new char[7];
      snprintf(rgbHex, 7, "%02X%02X%02X", std::get<0>(tuple), std::get<1>(tuple), std::get<2>(tuple));
      return rgbHex;
    }
  
    lv_color_t hexToFloat(int rgb) {
      return lv_color_hex(rgb);
    }
    
    float normalize(float value) {
    if (value < 0.0f) {
        return 0.0f;
    } else if (value > 1.0f) {
        return 1.0f;
    } else {
        return value;
    }
}

    // reference: https://dev.to/ndesmic/linear-color-gradients-from-scratch-1a0e
    const lv_color_t lerp(lv_color_t pointA, lv_color_t pointB, float normalValue) {
      auto [redA, greenA, blueA] = rgb565to888(LV_COLOR_GET_R(pointA), LV_COLOR_GET_G(pointA), LV_COLOR_GET_B(pointA));
      auto [redB, greenB, blueB] = rgb565to888(LV_COLOR_GET_R(pointB), LV_COLOR_GET_G(pointB), LV_COLOR_GET_B(pointB));
      NRF_LOG_INFO("Normal value: %f", normalValue);

      int outputRed = (redA + (redB - redA) * normalValue);
      int outputGreen = (greenA + (greenB - greenA) * normalValue);
      int outputBlue = (blueA + (blueB - blueA) * normalValue);
      
      //increase brightness
      float incAmount = 1.2;
      outputRed = std::min(255, static_cast<int>(outputRed*incAmount));
      outputGreen = std::min(255, static_cast<int>(outputGreen*incAmount));
      outputBlue = std::min(255, static_cast<int>(outputBlue*incAmount));
      
      auto lerpOutput = LV_COLOR_MAKE(outputRed, outputGreen, outputBlue);
      NRF_LOG_INFO("pointA: %i, %i, %i", redA, greenA, blueA);
      NRF_LOG_INFO("pointB: %i, %i, %i", redB, greenB, blueB);
      NRF_LOG_INFO("lerpOutput: %i, %i, %i", LV_COLOR_GET_R(lerpOutput), LV_COLOR_GET_G(lerpOutput), LV_COLOR_GET_B(lerpOutput));
      return lerpOutput;
    }
    
    constexpr std::array<lv_color_t, 4> getColors() {
      const std::array<int, 4> colors = {0x5555ff, 0x00c9ff, 0xff9b00, 0xff0000};
      std::array<lv_color_t, 4> stops;
      int8_t i = 0;
      for (auto colorVal: colors) {
       stops[i++] = (hexToFloat(colorVal));
      }
      return stops;
    }
    
    const lv_color_t WeatherHelper::TemperatureColor(int16_t temperature) {
      std::array<lv_color_t, 4> stops = getColors();
      int tempRounded = RoundTemperature(temperature);
      if (tempRounded < 0) {
         tempRounded = 1;
      }
      // convert temperature to range between newMin and newMax
      float oldRange = (oldMax - oldMin);
      float newRange = (newMax - newMin);
      float newValue = (((tempRounded - oldMin) * newRange) / oldRange) + newMin;
      newValue = normalize(newValue);
      return lerp(stops[0], stops[3], newValue);
    }
