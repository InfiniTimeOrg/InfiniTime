#include "displayapp/screens/Notepad.h"
#include <lvgl/lvgl.h>
#include <string.h>
#include "components/settings/Settings.h"

using namespace Pinetime::Applications::Screens;

namespace {
  void event_handler(lv_obj_t* obj, lv_event_t event) {
    auto* screen = static_cast<Notepad*>(obj->user_data);
    screen->on_touch_event(obj, event);
  }
}

Notepad::~Notepad() {
  lv_obj_clean(lv_scr_act());
}

static const char* dict[] = {
  "",
  "",
  "toitofmeisinimnomydobeonwesoheohifupgoatasorokanusamuhbyidmrhiumahdrhayammnt",
  "ingyoutheandfornotwasitsbutallaregetouthershecannowhimhowgotonedidwhyseeillheyhesyeshiswhohadsaywaytooiveourletmanhasanyofftwogoddayputg"
  "uyhuhbiglotnewbadmomdadtryownaskoldsonhmmsawsirmayjobboycaryetfewuserunbyeendagositfunkidbitmrswowsetfarsamdiehitpaywareyelawaddairlowar"
  "twinagebuyfeecutsixarmtaxeatoilredbedtop",
  "havethatwiththeythisfromwhatwillmakeknowtimeyearwhensomethemtakeintojustcomeyourthanlikethenmorewantlookalsoherefindgivewellmanyonlytell"
  "veryevenbackgoodlifedownworkcalloverlastneedfeelhighmostmuchmeankeepsameseemtalkturnhandhelppartshowsuchcitycaseweekeachhearplaymovehold"
  "livenextmusthomeroomfactareahellbookkindwordsidefourheadlongbothawayhourlinegameeverlosemeetnamefiveoncerealnothbestidealeadbodyteamstop"
  "facereaddoorsuregrowmornopenwalknewsgirlfoodablelovewaitsendstayfallplankilllateyeahelsepasssellroleratecarewifeminddruglesspullviewfree"
  "hopetruetownfulljoinroadformhardpickwearbasesitehalfcosteasywalldatateststarlandtypestepbabydrawtreefilmparkhairruletermmilefiredealrest"
  "seekriskwestsoonfillpastupongoaldropbankpushnotefinenearpage",
  "theirwouldabouttherethinkwhichcouldotherthesefirstthingthosewomanchildafterworldstillthreestateneverhouseleavewhilegreatbegingroupwheree"
  "verystartmightplaceagainrightsmallnighttodaypointbringlargeunderwritemoneystorywateryoungmonthstudyissueblackhellosincepoweroftenuntilst"
  "andamonglaterleastwhitelearntablewatchanythspeakallowlevelspendpartycourtforceearlyoffermaybehumanservesensebuilddeathreachlocalraisecla"
  "ssmajorphotofieldalongmusiclightwholevoicecolorheartcarrydrivepricebreakvaluethankmodelagreewhosepapereventspacequiteclearcoverimageteac"
  "hphonepiecethirdmoviecatchnorthcauseshortfloorplantclosewrongsouthboardfightorderthrowstoresoundfocusblood",
  "peopleshouldschoolbecomesomethreallyfamilysystemduringnumberalwayshappenbeforemotherthoughlittlearoundfriendfathermemberalmostcenterchan"
  "geminutefollowpublicsocialparentcreateofficepersonstreetreasonwithinhealthresultmomentenoughacrosssecondtowardappearpolicyincludexpectma"
  "rketnationbehindremaineffectformer",
};
static const int dictlen[] = {0, 0, 76, 312, 740, 650, 306};
static const char* num2s[] = {"", ".,?", "abc", "def", "ghi", "jkl", "mno", "pqrs", "tuv", "wxyz"};

static const char* t9ButtonMap[] = {
  ".,?",  "abc",  "def",  "\n",
  "ghi",  "jkl",  "mno",  "\n",
  "pqrs",  "tuv",  "wxyz",  "\n",
  "<>",  "_",  "123",  "",
};

static const char* numButtonMap[] = {
  "1", "2", "3", "\n",
  "4", "5", "6", "\n",
  "7", "8", "9", "\n",
  ".", "0", "Aa", "",
};

Notepad::Notepad(Pinetime::Applications::DisplayApp* app, char* textptr) : Screen(app) {
  // init
  text = textptr;
  tpos = strlen(text);

  // textarea
  result = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_long_mode(result, LV_LABEL_LONG_BREAK);
  lv_obj_set_pos(result, 0, 0);

  // backspace
  bspButton = lv_btn_create(lv_scr_act(), nullptr);
  lv_obj_set_size(bspButton, 52, 52);
  lv_obj_set_pos(bspButton, 186, 0);
  bspLabel = lv_label_create(bspButton, nullptr);
  lv_obj_align(bspLabel, nullptr, LV_ALIGN_CENTER, 0, 0);
  bspButton->user_data = this;
  lv_obj_set_event_cb(bspButton, event_handler);

  // T9 keyboard
  buttonMatrix = lv_btnmatrix_create(lv_scr_act(), nullptr);
  lv_btnmatrix_set_map(buttonMatrix, t9ButtonMap);
  lv_obj_set_size(buttonMatrix, 240, 180);
  lv_obj_set_style_local_pad_all(buttonMatrix, LV_BTNMATRIX_PART_BG, LV_STATE_DEFAULT, 0);
  buttonMatrix->user_data = this;
  lv_obj_set_event_cb(buttonMatrix, event_handler);

  toggle_edit();
}

void Notepad::toggle_edit() {
  in_edit_mode = !in_edit_mode;
  if (in_edit_mode) {
    lv_obj_set_size(result, 180, 60);
    lv_obj_set_pos(buttonMatrix, 0, 60);
    lv_label_set_text(bspLabel, "<=");
    update_textarea();
  } else {
    lv_obj_set_size(result, 180, LV_VER_RES);
    lv_obj_set_pos(buttonMatrix, LV_HOR_RES, LV_VER_RES);
    lv_label_set_text(bspLabel, "Edit");
    lv_label_set_text_static(result, text);
  }
}

bool Notepad::c_in_str(char c, const char* s) {
  for (uint8_t i = 0; i < strlen(s); i++) {
    if (s[i] == c) {
      return true;
    }
  }
  return false;
}

void Notepad::next_guess() {
  if (rpos == 0) {
    return;
  }
  if (rpos == 1) {
    uint8_t num = raw_input[0] - 48;
    guess_idx = (guess_idx + 1) % strlen(num2s[num]);
    guess[0] = num2s[num][guess_idx];
  } else if (rpos <= 6) {
    // search dictionary
    for (int i = rpos; i <= dictlen[rpos]; i += rpos) {
      bool found = true;
      for (uint8_t j = 0; j < rpos; j++) {
        if (c_in_str(dict[rpos][(guess_idx + i + j) % dictlen[rpos]], num2s[raw_input[j] - 48]) == false) {
          found = false;
          break;
        }
      }
      if (found == true) {
        guess_idx = (guess_idx + i) % strlen(dict[rpos]);
        strncpy(guess, dict[rpos] + guess_idx, rpos);
        return;
      }
    }
    // if no match found
    guess[gpos - 1] = num2s[raw_input[rpos - 1] - 48][0];
  }
}

void Notepad::update_textarea() {
  if (tpos + gpos > 16) {
    if (gpos == 0) {
      lv_label_set_text_fmt(result, "<..%s%c", text + tpos - 16, eol);
    } else {
      lv_label_set_text_fmt(result, "<..%s(%s)%c", text + tpos + gpos - 16, guess, eol);
    }
  } else {
    if (gpos == 0) {
      lv_label_set_text_fmt(result, "%s%c", text, eol);
    } else {
      lv_label_set_text_fmt(result, "%s(%s)%c", text, guess, eol);
    }
  }
}

void Notepad::t9_enter() {
  strncpy(text + tpos, guess, gpos);
  tpos += gpos - 1;
  gpos = 0;
  rpos = 0;
}

void Notepad::on_input_len_change() {
  guess[gpos] = '\0';
  raw_input[rpos] = '\0';
  guess_idx = dictlen[rpos] - rpos;
  next_guess();
  update_textarea();
  if (guess[0] == '\0') {
    tpos++;
  }
}

void Notepad::on_touch_event(lv_obj_t* obj, lv_event_t event) {
  if (event == LV_EVENT_RELEASED && obj == bspButton) {
    ignore_repeat = false;
  } else if (event == LV_EVENT_LONG_PRESSED && obj == bspButton && !ignore_repeat) {
    if (!ignore_repeat) {
      toggle_edit();
    }
    ignore_repeat = true;
  } else if (event == LV_EVENT_CLICKED && !ignore_repeat) {
    if (obj == bspButton) {
      if (!in_edit_mode) {
        toggle_edit();
        return;
      }
      if (guess[0] != '\0') {
        if (gpos > 0) {
          rpos--;
          gpos--;
        }
        guess[gpos] = '\0';
        guess_idx = 0;
      } else {
        if (tpos > 0) {
          tpos--;
        }
        text[tpos] = '\0';
      }
      eol = '_';
      update_textarea();
    } else if (obj == buttonMatrix) {
      const char* buttonstr = lv_btnmatrix_get_active_btn_text(obj);
      if (strcmp(buttonstr, "Aa") == 0) {
        lv_btnmatrix_set_map(buttonMatrix, t9ButtonMap);
        return;
      } else if (strcmp(buttonstr, "123") == 0) {
        lv_btnmatrix_set_map(buttonMatrix, numButtonMap);
        if (gpos == 0) {
          return;
        } else {
          t9_enter();
          update_textarea();
        }
      } else if (strcmp(buttonstr, "<>") == 0) {
        next_guess();
        update_textarea();
        return;
      }
      if (tpos + gpos >= 50) {
        eol = '!';
        update_textarea();
        return;
      } else {
        eol = '_';
      }
      if (buttonstr == "_") {
        if (gpos > 0) {
          t9_enter();
        } else {
          text[tpos] = ' ';
        }
      } else if (strlen(buttonstr) == 1) {
        // direct input
        text[tpos] = buttonstr[0];
      } else {
        // t9 input
        if (rpos == 7) {
          // t9 buffer full
          t9_enter();
          on_input_len_change();
        }
        // find raw_input (number from numButtonMap)
        for (uint8_t i = 0; i < 14; i++) {
          if (strcmp(buttonstr, t9ButtonMap[i]) == 0) {
            raw_input[rpos] = numButtonMap[i][0];
            rpos++;
            gpos++;
            break;
          }
        }
      }
      on_input_len_change();
    }
  }
}
