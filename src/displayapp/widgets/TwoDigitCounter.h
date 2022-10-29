#pragma once
#include <lvgl/lvgl.h>

namespace Pinetime {
    namespace Applications {
        namespace Widgets {
            class TwoDigitCounter {
                public:
                    TwoDigitCounter(int min, int max, lv_font_t& font);

                    void Create();
                    void resetTotal();
                    void UpBtnPressed(bool isTens);
                    void DownBtnPressed(bool isTens);

                    void SetValueChangedEventCallback(void* userData, void (*handler)(void* userData));

                    lv_obj_t* GetObject() const {
                        return counterContainer;
                    };

                    int getValue() {
                        return value;
                    }

                    void setValue(int newValue) {
                        value =  newValue;
                    }
                    
                private:
                    void UpdateLabel();
                    void (*ValueChangedHandler)(void* userData) = nullptr;

                    lv_obj_t* counterContainer;
                    lv_obj_t* tensUpBtn;
                    lv_obj_t* tensDownBtn;
                    lv_obj_t* upBtn;
                    lv_obj_t* downBtn;
                    lv_obj_t* number;
                    lv_obj_t* tensUpperLine;
                    lv_obj_t* tensLowerLine;
                    lv_obj_t* upperLine;
                    lv_obj_t* lowerLine;
                    lv_point_t linePoints[2];
                    
                    int min;
                    int max;
                    static inline int value = 40;
                    lv_font_t& font;

                    void* userData = nullptr;
            };
        }
    }
}