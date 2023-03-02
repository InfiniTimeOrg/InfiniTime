#include <array>
#include <lvgl/lvgl.h>

namespace Pinetime {
  namespace Applications {
    namespace Widgets {

      template <int nButtons>
      class MultiPurposeButton {
      public:
        struct ButtonData {
          lv_event_cb_t eventCb;
          const char* text;
          lv_color_t color;
        };

        explicit MultiPurposeButton(std::array<ButtonData, nButtons> buttonData, void* userData);

        void SetActiveButton(size_t index);

        void Enable();
        void Disable();

        lv_obj_t* GetObject() {
          return button;
        }

      private:
        lv_obj_t* button;
        lv_obj_t* buttonLabel;
        std::array<ButtonData, nButtons> buttonData;
      };

      template <int nButtons>
      MultiPurposeButton<nButtons>::MultiPurposeButton(std::array<MultiPurposeButton<nButtons>::ButtonData, nButtons> buttonData,
                                                       void* userData)
        : button {lv_btn_create(lv_scr_act(), nullptr)}, buttonLabel {lv_label_create(button, nullptr)}, buttonData {buttonData} {
        button->user_data = userData;
        SetActiveButton(0);
      }

      template <int nButtons>
      void MultiPurposeButton<nButtons>::SetActiveButton(size_t index) {
        lv_obj_set_event_cb(button, buttonData[index].eventCb);
        lv_label_set_text_static(buttonLabel, buttonData[index].text);
        lv_obj_set_style_local_bg_color(button, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, buttonData[index].color);
      }

      template <int nButtons>
      void MultiPurposeButton<nButtons>::Enable() {
        lv_obj_set_state(button, LV_STATE_DEFAULT);
        lv_obj_set_state(buttonLabel, LV_STATE_DEFAULT);
      }

      template <int nButtons>
      void MultiPurposeButton<nButtons>::Disable() {
        lv_obj_set_state(button, LV_STATE_DISABLED);
        lv_obj_set_state(buttonLabel, LV_STATE_DISABLED);
      }
    }
  }
}
