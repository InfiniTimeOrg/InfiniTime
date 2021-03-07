/*
 * Copyright (c) 2018 Phytec Messtechnik GmbH
 * Copyright (c) 2018 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mesh/mesh.h"

void board_refresh_display(void);
void board_show_text(const char *text, bool center, int32_t duration);
void board_blink_leds(void);
void board_add_hello(uint16_t addr, const char *name);
void board_add_heartbeat(uint16_t addr, uint8_t hops);
int board_init(void);
