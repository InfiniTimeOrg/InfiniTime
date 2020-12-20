#pragma once

#include <lvgl/lvgl.h>
#include <cstdint>
#include "Screen.h"

namespace Pinetime {
  namespace Components {
    class LittleVgl;
  }
  namespace Applications {
    namespace Screens {

      class Paddle : public Screen{
        public:
          Paddle(DisplayApp* app, Pinetime::Components::LittleVgl& lvgl);
          ~Paddle() override;

          bool Refresh() override;
          bool OnButtonPushed() override;
          bool OnTouchEvent(TouchEvents event) override;
          bool OnTouchEvent(uint16_t x, uint16_t y) override;
          
        private:  
          Pinetime::Components::LittleVgl& lvgl;
        
          int paddleBottomY = 90;            // bottom extreme of the paddle
          int paddleTopY = 150;		        //top extreme of the paddle

          int ballX = 107;	            // Initial x_coordinate for the ball (12px offset from the center to counteract the ball's 24px size)
          int ballY = 107;	            // Initial y_coordinate for the ball

          int dx = 2;		            // Velocity of the ball in the x_coordinate
          int dy = 3;		            // Velocity of the ball in the y_coordinate

          int counter = 0;	            // init Frame refresh limit counter
          int score = 0;   

          char scoreStr[10];

          lv_img_dsc_t paddle; 
          lv_img_dsc_t ball;

          lv_obj_t* points;
          lv_obj_t* paddle_image;		// pointer to paddle image
          lv_obj_t* ball_image;		// pointer to ball image

          bool running = true;
      };
    }
  }
}
