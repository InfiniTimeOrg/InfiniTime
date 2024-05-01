#include "displayapp/screens/WeatherColorTester.h"
#include "displayapp/WeatherHelper.h"
#include "displayapp/LittleVgl.h"
#include <nrfx_log.h>
#include <algorithm> // std::fill
using namespace Pinetime::Applications::Screens;

WeatherColorTester::WeatherColorTester(Pinetime::Components::LittleVgl& lvgl) : lvgl {lvgl} {
    taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
    Refresh();
}

void WeatherColorTester::Refresh() { //WeatherHelper::oldMax
    for (size_t i = 0; i < WeatherHelper::oldMax; ++i) {
        // get current color
        currentColor = WeatherHelper::TemperatureColor(i * 100);
        // create area we want to color
        lv_area_t area;
        area.y1 = x + i;
        area.x1 = 0;
        area.y2 = (x + i) + 1;
        area.x2 = y;
        NRF_LOG_INFO("area coords: %i, %i, %i, %i", area.x1, area.y1, area.x2, area.y2);
        // write the buffer to the display
        lvgl.SetFullRefresh(Components::LittleVgl::FullRefreshDirections::None);
        std::fill(buffer, buffer + bufferSize, currentColor);
        lvgl.FlushDisplay(&area, buffer);
    }
}

WeatherColorTester::~WeatherColorTester() {
  lv_task_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
}
