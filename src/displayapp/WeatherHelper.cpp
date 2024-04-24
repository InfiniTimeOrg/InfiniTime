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
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <nrfx_log.h>
using namespace Pinetime::Applications;
    //Linear gradient temperature color calculator :)

    int16_t WeatherHelper::RoundTemperature(int16_t temp) {
      return temp = temp / 100 + (temp % 100 >= 50 ? 1 : 0);
    }
  
    const char* WeatherHelper::floatToRgbHex(lv_color_t rgb) {
      char *rgbHex = new char[7];
      snprintf(rgbHex, 7, "%02X%02X%02X", LV_COLOR_GET_R(rgb), LV_COLOR_GET_G(rgb), LV_COLOR_GET_B(rgb));
      return rgbHex;
    }
  
    lv_color_t hexToFloat(int rgb) {
      //uint16_t r = ((rgb >> 16) & 0xFF);
      //uint16_t g = ((rgb >> 8) & 0xFF);
      //int16_t b = (rgb & 0xFF);
      //NRF_LOG_INFO("hexToFloat: %i, %i, %i", r, g, b);
      lv_color_t outputLv = lv_color_hex3(rgb);
      NRF_LOG_INFO("output lv struct: %i, %i, %i", LV_COLOR_GET_R(outputLv), LV_COLOR_GET_G(outputLv), LV_COLOR_GET_B(outputLv));
      return outputLv;
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
    const lv_color_t lerp(lv_color_t pointA, lv_color_t pointB, float normalValue) {
    // reference: https://dev.to/ndesmic/linear-color-gradients-from-scratch-1a0e
    //std::tuple<float, float, float> lerp(std::tuple<float, float, float> pointA, std::tuple<float, float, float> pointB, float normalValue) {
      NRF_LOG_INFO("Normal value: %f", normalValue);
      auto redA = LV_COLOR_GET_R(pointA);
      auto redB = LV_COLOR_GET_R(pointB);
      auto greenA = LV_COLOR_GET_G(pointA);
      auto greenB = LV_COLOR_GET_G(pointB);
      auto blueA = LV_COLOR_GET_B(pointA);
      auto blueB = LV_COLOR_GET_B(pointB);

      int outputRed = (redA + (redB - redA) * normalValue);
      int outputGreen = (greenA + (greenB - greenA) * normalValue);
      int outputBlue = (blueA + (blueB - blueA) * normalValue);
      auto lerpOutput = LV_COLOR_MAKE(outputRed, outputGreen, outputBlue);
      NRF_LOG_INFO("pointA: %i, %i, %i", redA, greenA, blueA);
      NRF_LOG_INFO("pointB: %i, %i, %i", redB, greenB, blueB);
      NRF_LOG_INFO("lerpOutput: %i, %i, %i", LV_COLOR_GET_R(lerpOutput), LV_COLOR_GET_G(lerpOutput), LV_COLOR_GET_B(lerpOutput));
      return lerpOutput;
    }
    
    const lv_color_t WeatherHelper::TemperatureColor(int16_t temperature) {
      const std::vector<int> colors = {0x5555ff, 0x00c9ff, 0xff9b00, 0xff0000};
      std::vector<lv_color_t> stops;
      for (auto colorVal: colors) {
       stops.emplace_back(hexToFloat(colorVal));
      }
      int tempRounded = RoundTemperature(temperature);
      if (tempRounded < 0) {
         tempRounded = 1;
      }
      // convert temperature to range between newMin and newMax
      float oldMax = 50;
      float oldMin = 0;
      float newMax = 1;
      float newMin = 0;
      float oldRange = (oldMax - oldMin);
      float newRange = (newMax - newMin);
      float newValue = (((tempRounded - oldMin) * newRange) / oldRange) + newMin;
      newValue = normalize(newValue);
      if (newValue <= .33f) {
        return lerp(stops[0], stops[1], newValue);
      } else if (newValue <= .66f) {
        return lerp(stops[1], stops[2], newValue);
      } else {
        return lerp(stops[2], stops[3], newValue);
      }
    }
