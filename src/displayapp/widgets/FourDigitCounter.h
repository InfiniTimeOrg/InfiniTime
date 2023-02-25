#pragma once

#include <lvgl/lvgl.h>

enum NumberPlace { Thousands, Hundreds, Tens};

namespace Pinetime {
    namespace Applications {
        namespace Widgets {
            class FourDigitCounter {
                public:
                    FourDigitCounter(int min, int max, lv_font_t& font);

                    void Create();
                    void resetTotal();
                    void UpBtnPressed(NumberPlace numberPlace);
                    void DownBtnPressed(NumberPlace numberPlace);

                    void SetValueChangedEventCallback(void* userData, void (*handler)(void* userData));

                    lv_obj_t* GetObject() const {
                        return counterContainer;
                    };

                    int getValue() {
                        return yugiohLife;
                    }

                    void setValue(int newValue) {
                        yugiohLife =  newValue;
                    }
                private:
                    void UpdateLabel();
                    void (*ValueChangedHandler)(void* userData) = nullptr;

                    lv_obj_t* counterContainer;
                    lv_obj_t* thousandsUpBtn;
                    lv_obj_t* hundredsUpBtn;
                    lv_obj_t* tensUpBtn;
                    lv_obj_t* thousandsDownBtn;
                    lv_obj_t* hundredsDownBtn;
                    lv_obj_t* tensDownBtn;
                    lv_obj_t* number;

                    lv_obj_t* thousandsUpperLine;
                    lv_obj_t* thousandsLowerLine;
                    lv_obj_t* hundredsUpperLine;
                    lv_obj_t* hundredsLowerLine;
                    lv_obj_t* tensUpperLine;
                    lv_obj_t* tensLowerLine;
                    
                    lv_point_t linePoints[2];
                    
                    int min;
                    int max;
                    static inline int yugiohLife = 8000; //default starting life point total
                    lv_font_t& font;

                    void* userData = nullptr;
            };
        }
    }
}