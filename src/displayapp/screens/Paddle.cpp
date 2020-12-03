#include "Paddle.h"
#include "../DisplayApp.h"
#include "../LittleVgl.h"

using namespace Pinetime::Applications::Screens;
extern lv_font_t jetbrains_mono_extrabold_compressed;
extern lv_font_t jetbrains_mono_bold_20;

Paddle::Paddle(Pinetime::Applications::DisplayApp* app, Pinetime::Components::LittleVgl& lvgl) : Screen(app), lvgl{lvgl} {
  app->SetTouchMode(DisplayApp::TouchModes::Polling);
	
  points = lv_label_create(lv_scr_act(), NULL);
  lv_label_set_text(points, "0");	
  lv_obj_align(points, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 0);
	
  paddle.header.always_zero = 0;
  paddle.header.w = 4;
  paddle.header.h = 60;
  paddle.data_size = 60 * 4 * LV_COLOR_SIZE / 8;
  paddle.header.cf = LV_IMG_CF_TRUE_COLOR_CHROMA_KEYED;
  paddle.data = paddle_map;  
  paddle_image = lv_img_create(lv_scr_act(), NULL);
  lv_img_set_src(paddle_image, &paddle);
  
  ball.header.always_zero = 0;
  ball.header.w = 24;
  ball.header.h = 24;
  ball.data_size = 24 * 24 * LV_COLOR_SIZE / 8;
  ball.header.cf = LV_IMG_CF_TRUE_COLOR_CHROMA_KEYED;
  ball.data = ball_map;  
  ball_image = lv_img_create(lv_scr_act(), NULL);
  lv_img_set_src(ball_image, &ball);
}

Paddle::~Paddle() {
  // Reset the touchmode
  app->SetTouchMode(DisplayApp::TouchModes::Gestures);
  lv_obj_clean(lv_scr_act());
}

bool Paddle::Refresh() {
  
  if((counter++ % 5) == 0){

    counter = 0;	
    
    ball_x += dx;
    ball_y += dy;	    
	  
    lv_obj_set_pos(ball_image, ball_x, ball_y);
  
    //checks if it has touched the sides (floor and ceiling)	  
    if(ball_y <= 0 || ball_y >= 215){
      dy *= -1;
    }
  
    //checks if it has touched the side (left side)  
    if(ball_x >= 215){
      dx *= -1; 
    }
  
    //checks if it is in the position of the paddle	  
    if(ball_y >= (y_paddle_bottom + 16) && ball_y <= (y_paddle_top - 8)){ 
      if(ball_x >= 0 && ball_x < 4){    
          lv_obj_set_pos(ball_image, 5, ball_y);
          dx *= -1;   
          score++;
        } 
    }	
	  
    //checks if it has gone behind the paddle
    else if(ball_x <= -40){
      ball_x = 107;
      ball_y = 107; 
      score = 0;	    
    } 
    sprintf(Val, "%d", score);	
    lv_label_set_text(points, Val);	  
  }      
  return running;
}

bool Paddle::OnButtonPushed() {
  running = false;
  return true;
}


bool Paddle::OnTouchEvent(Pinetime::Applications::TouchEvents event) { 
  return true; 
}

bool Paddle::OnTouchEvent(uint16_t x, uint16_t y) {
 
  lv_obj_set_pos(paddle_image, 0, y - 30);		// sets the center paddle pos. (30px offset) with the the y_coordinate of the finger and defaults the x_coordinate to 0
  y_paddle_top = y - 30;					// refreshes the upper extreme of the paddle
  y_paddle_bottom = y + 30;					// refreshes the lower extreme of the paddle
  
  return true;
}
