#include "symbols"

var lv_obj: label_time
var lv_obj: label_time_ampm
var lv_obj: label_date
var lv_obj: notification_icon
var lv_obj: heartbeat_icon
var lv_obj: heartbeat_value
var lv_obj: step_icon
var lv_obj: step_value

var clock_type

var buffer{20}

@start() {
    status_icons_create();
    clock_type = get_setting(SETTING_CLOCK_TYPE)

    notification_icon = lv_label_create(screen)
    lv_obj_set_style_local_color(notification_icon, LV_STYLE_TEXT_COLOR, 0x00FF00)
    lv_obj_align(notification_icon, screen, LV_ALIGN_IN_TOP_LEFT);

    heartbeat_icon = lv_label_create(screen, .text = symbol_heartBeat)
    lv_obj_set_style_local_color(heartbeat_icon, LV_STYLE_TEXT_COLOR, 0x1B1B1B)
    lv_obj_align(heartbeat_icon, screen, LV_ALIGN_IN_BOTTOM_LEFT)

    heartbeat_value = lv_label_create(screen)
    lv_obj_set_style_local_color(heartbeat_value, LV_STYLE_TEXT_COLOR, 0xCE1B1B)
    lv_obj_align(heartbeat_value, heartbeat_icon, LV_ALIGN_OUT_RIGHT_MID, 5, 0)

    step_value = lv_label_create(screen, .text = "0")
    lv_obj_set_style_local_color(step_value, LV_STYLE_TEXT_COLOR, 0x00FFE7)
    lv_obj_align(step_value, screen, LV_ALIGN_IN_BOTTOM_RIGHT)

    step_icon = lv_label_create(screen, .text = symbol_shoe)
    lv_obj_set_style_local_color(step_icon, LV_STYLE_TEXT_COLOR, 0x00FFE7)
    lv_obj_align(step_icon, step_value, LV_ALIGN_OUT_LEFT_MID, -5, 0)

    label_date = lv_label_create(screen)
    lv_obj_align(label_date, screen, LV_ALIGN_CENTER, 0, 60)
    lv_obj_set_style_local_color(label_date, LV_STYLE_TEXT_COLOR, 0x999999)

    label_time = lv_label_create(screen)
    lv_obj_set_style_local_ptr(label_time, LV_STYLE_TEXT_FONT, jetbrains_mono_extrabold_compressed)
    lv_obj_align(label_time, screen, LV_ALIGN_IN_RIGHT_MID)

    label_time_ampm = lv_label_create(screen)
    lv_obj_align(label_time_ampm, screen, LV_ALIGN_IN_RIGHT_MID, 0, -55)
}

@refresh() {
    status_icons_update();

    static datetime[DATETIME]

    // Returns true if changed
    if (read_datetime(datetime)) {
        var hour = datetime.hour

        if (clock_type == CLOCK_TYPE_H12) {
            lv_label_set_text(label_time_ampm, hour < 12 ? "AM" : "PM")
            lv_obj_realign(label_time_ampm)

            if (hour == 0) hour = 12
            else if (hour > 12) hour -= 12
        }
        sprintf(buffer, _, clock_type == CLOCK_TYPE_H12 ? "%2d:%02d" : "%02d:%02d", hour, datetime.minute)
        lv_label_set_text(label_time, buffer)
        lv_obj_realign(label_time)

        static day{4}
        static month{4}
        read_datetime_short_str(day, month)

        if (clock_type == CLOCK_TYPE_H12)
            sprintf(buffer, _, "%s %s %d %d", day, month, datetime.year, datetime.day)
        else
            sprintf(buffer, _, "%s %d %s %d", day, datetime.day, month, datetime.year)

        lv_label_set_text(label_date, buffer)
        lv_obj_realign(label_date)
    }

    static new_notifs
    if (swap(new_notifs, has_new_notifications())) {
        lv_label_set_text(notification_icon, new_notifs ? symbol_info : "")
    }

    static heartrate
    static heartrate_state: heartrate_state
    
    var updated = swap(heartrate, get_heartrate())
    updated = swap(heartrate_state, get_heartrate_state()) || updated // Circumvent short-circuiting

    if (updated) {
        if (heartrate_state == HEARTRATE_RUNNING) {
            lv_obj_set_style_local_color(heartbeat_icon, LV_STYLE_TEXT_COLOR, 0xCE1B1B)
            sprintf(buffer, _, "%d", heartrate)
            lv_label_set_text(heartbeat_value, buffer)
        } else {
            lv_obj_set_style_local_color(heartbeat_icon, LV_STYLE_TEXT_COLOR, 0x1B1B1B)
            lv_label_set_text(heartbeat_value, "")
        }

        lv_obj_realign(heartbeat_icon)
        lv_obj_realign(heartbeat_value)
    }

    static steps
    if (swap(steps, get_step_number())) {
        sprintf(buffer, _, "%d", steps)
        lv_label_set_text(step_value, buffer)
        lv_obj_realign(step_value)
        lv_obj_realign(step_icon)
    }
}
