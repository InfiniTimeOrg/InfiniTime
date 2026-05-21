#include "infinitime"

@start() {
	var lv_obj: label = lv_label_create()
	lv_obj_set_pos(label, 20, 20)
	lv_label_set_text(label, "Hello world")
}
