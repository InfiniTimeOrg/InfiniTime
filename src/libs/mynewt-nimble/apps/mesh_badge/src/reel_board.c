/*
 * Copyright (c) 2018 Phytec Messtechnik GmbH
 * Copyright (c) 2018 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "os/mynewt.h"
#include "bsp/bsp.h"
#include "console/console.h"
#include "hal/hal_flash.h"
#include "hal/hal_gpio.h"
#include "mesh/glue.h"
#include "services/gap/ble_svc_gap.h"

#include "mesh_badge.h"
#include "display/cfb.h"
#include "mesh.h"
#include "board.h"

#define printk console_printf

enum font_size {
	FONT_BIG = 0,
	FONT_MEDIUM = 1,
	FONT_SMALL = 2,
};

struct font_info {
	uint8_t columns;
} fonts[] = {
	[FONT_BIG] =    { .columns = 12 },
	[FONT_MEDIUM] = { .columns = 16 },
	[FONT_SMALL] =  { .columns = 25 },
};

#define LONG_PRESS_TIMEOUT K_SECONDS(1)

#define STAT_COUNT 128

#define EDGE (GPIO_INT_EDGE | GPIO_INT_DOUBLE_EDGE)

#ifdef SW0_GPIO_FLAGS
#define PULL_UP SW0_GPIO_FLAGS
#else
#define PULL_UP 0
#endif

static struct os_dev *epd_dev;
static bool pressed;
static bool stats_view;
static struct k_delayed_work epd_work;
static struct k_delayed_work long_press_work;

static struct {
    int pin;
} leds[] = {
	{ .pin = LED_1, },
	{ .pin = RGB_LED_RED, },
	{ .pin = RGB_LED_GRN, },
	{ .pin = RGB_LED_BLU, },
};

struct k_delayed_work led_timer;

static size_t print_line(enum font_size font_size, int row, const char *text,
			 size_t len, bool center)
{
	uint8_t font_height, font_width;
	char line[fonts[FONT_SMALL].columns + 1];
	int pad;

	cfb_framebuffer_set_font(epd_dev, font_size);

	len = min(len, fonts[font_size].columns);
	memcpy(line, text, len);
	line[len] = '\0';

	if (center) {
		pad = (fonts[font_size].columns - len) / 2;
	} else {
		pad = 0;
	}

	cfb_get_font_size(epd_dev, font_size, &font_width, &font_height);

	if (cfb_print(epd_dev, line, font_width * pad, font_height * row)) {
		printk("Failed to print a string\n");
	}

	return len;
}

static size_t get_len(enum font_size font, const char *text)
{
	const char *space = NULL;
	size_t i;

	for (i = 0; i <= fonts[font].columns; i++) {
		switch (text[i]) {
		case '\n':
		case '\0':
			return i;
		case ' ':
			space = &text[i];
			break;
		default:
			continue;
		}
	}

	/* If we got more characters than fits a line, and a space was
	 * encountered, fall back to the last space.
	 */
	if (space) {
		return space - text;
	}

	return fonts[font].columns;
}

void board_blink_leds(void)
{
	k_delayed_work_submit(&led_timer, K_MSEC(100));
}

void board_show_text(const char *text, bool center, int32_t duration)
{
	int i;

	cfb_framebuffer_clear(epd_dev, false);

	for (i = 0; i < 3; i++) {
		size_t len;

		while (*text == ' ' || *text == '\n') {
			text++;
		}

		len = get_len(FONT_BIG, text);
		if (!len) {
			break;
		}

		text += print_line(FONT_BIG, i, text, len, center);
		if (!*text) {
			break;
		}
	}

	cfb_framebuffer_finalize(epd_dev);

	if (duration != K_FOREVER) {
		k_delayed_work_submit(&epd_work, duration);
	}
}

static struct stat {
	uint16_t addr;
	char name[9];
	uint8_t min_hops;
	uint8_t max_hops;
	uint16_t hello_count;
	uint16_t heartbeat_count;
} stats[STAT_COUNT] = {
	[0 ... (STAT_COUNT - 1)] = {
		.min_hops = BT_MESH_TTL_MAX,
		.max_hops = 0,
	},
};

static uint32_t stat_count;

#define NO_UPDATE -1

static int add_hello(uint16_t addr, const char *name)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(stats); i++) {
		struct stat *stat = &stats[i];

		if (!stat->addr) {
			stat->addr = addr;
			strncpy(stat->name, name, sizeof(stat->name) - 1);
			stat->hello_count = 1;
			stat_count++;
			return i;
		}

		if (stat->addr == addr) {
			/* Update name, incase it has changed */
			strncpy(stat->name, name, sizeof(stat->name) - 1);

			if (stat->hello_count < 0xffff) {
				stat->hello_count++;
				return i;
			}

			return NO_UPDATE;
		}
	}

	return NO_UPDATE;
}

static int add_heartbeat(uint16_t addr, uint8_t hops)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(stats); i++) {
		struct stat *stat = &stats[i];

		if (!stat->addr) {
			stat->addr = addr;
			stat->heartbeat_count = 1;
			stat->min_hops = hops;
			stat->max_hops = hops;
			stat_count++;
			return i;
		}

		if (stat->addr == addr) {
			if (hops < stat->min_hops) {
				stat->min_hops = hops;
			} else if (hops > stat->max_hops) {
				stat->max_hops = hops;
			}

			if (stat->heartbeat_count < 0xffff) {
				stat->heartbeat_count++;
				return i;
			}

			return NO_UPDATE;
		}
	}

	return NO_UPDATE;
}

void board_add_hello(uint16_t addr, const char *name)
{
	uint32_t sort_i;

	sort_i = add_hello(addr, name);
	if (sort_i != NO_UPDATE) {
	}
}

void board_add_heartbeat(uint16_t addr, uint8_t hops)
{
	uint32_t sort_i;

	sort_i = add_heartbeat(addr, hops);
	if (sort_i != NO_UPDATE) {
	}
}

static void show_statistics(void)
{
	int top[4] = { -1, -1, -1, -1 };
	int len, i, line = 0;
	struct stat *stat;
	char str[32];

	cfb_framebuffer_clear(epd_dev, false);

	len = snprintk(str, sizeof(str),
		       "Own Address: 0x%04x", mesh_get_addr());
	print_line(FONT_SMALL, line++, str, len, false);

	len = snprintk(str, sizeof(str),
		       "Node Count:  %lu", stat_count + 1);
	print_line(FONT_SMALL, line++, str, len, false);

	/* Find the top sender */
	for (i = 0; i < ARRAY_SIZE(stats); i++) {
		int j;

		stat = &stats[i];
		if (!stat->addr) {
			break;
		}

		if (!stat->hello_count) {
			continue;
		}

		for (j = 0; j < ARRAY_SIZE(top); j++) {
			if (top[j] < 0) {
				top[j] = i;
				break;
			}

			if (stat->hello_count <= stats[top[j]].hello_count) {
				continue;
			}

			/* Move other elements down the list */
			if (j < ARRAY_SIZE(top) - 1) {
				memmove(&top[j + 1], &top[j],
					((ARRAY_SIZE(top) - j - 1) *
					 sizeof(top[j])));
			}

			top[j] = i;
			break;
		}
	}

	if (stat_count >= 0) {
		len = snprintk(str, sizeof(str), "Most messages from:");
		print_line(FONT_SMALL, line++, str, len, false);

		for (i = 0; i < ARRAY_SIZE(top); i++) {
			if (top[i] < 0) {
				break;
			}

			stat = &stats[top[i]];

			len = snprintk(str, sizeof(str), "%-3u 0x%04x %s",
				       stat->hello_count, stat->addr,
				       stat->name);
			print_line(FONT_SMALL, line++, str, len, false);
		}
	}

	cfb_framebuffer_finalize(epd_dev);
}

static void epd_update(struct ble_npl_event *work)
{
	char buf[MYNEWT_VAL(BLE_SVC_GAP_DEVICE_NAME_MAX_LENGTH)];
	int i;

	if (stats_view) {
		show_statistics();
		return;
	}

	strncpy(buf, bt_get_name(), sizeof(buf));

	/* Convert commas to newlines */
	for (i = 0; buf[i] != '\0'; i++) {
		if (buf[i] == ',') {
			buf[i] = '\n';
		}
	}

	board_show_text(buf, true, K_FOREVER);
}

static void long_press(struct ble_npl_event *work)
{
	/* Treat as release so actual release doesn't send messages */
	pressed = false;
	stats_view = !stats_view;
	board_refresh_display();
}

static bool button_is_pressed(void)
{
	uint32_t val;

	val = (uint32_t) hal_gpio_read(BUTTON_1);

	return !val;
}

static void button_interrupt(struct os_event *ev)
{
	int pin_pos = (int ) ev->ev_arg;

	if (button_is_pressed() == pressed) {
		return;
	}

	pressed = !pressed;
	printk("Button %s\n", pressed ? "pressed" : "released");

	if (pressed) {
		k_delayed_work_submit(&long_press_work, LONG_PRESS_TIMEOUT);
		return;
	}

	k_delayed_work_cancel(&long_press_work);

	if (!mesh_is_initialized()) {
		return;
	}

	/* Short press does currently nothing in statistics view */
	if (stats_view) {
		return;
	}

	if (pin_pos == BUTTON_1) {
		mesh_send_hello();
	}
}

static struct os_event button_event;

static void
gpio_irq_handler(void *arg)
{
	button_event.ev_arg = arg;
	os_eventq_put(os_eventq_dflt_get(), &button_event);
}

static int configure_button(void)
{
	button_event.ev_cb = button_interrupt;

	hal_gpio_irq_init(BUTTON_1, gpio_irq_handler, (void *) BUTTON_1,
			  HAL_GPIO_TRIG_BOTH, HAL_GPIO_PULL_UP);
	hal_gpio_irq_enable(BUTTON_1);

	return 0;
}

static void led_timeout(struct ble_npl_event *work)
{
	static int led_cntr;
	int i;

	/* Disable all LEDs */
	for (i = 0; i < ARRAY_SIZE(leds); i++) {
		hal_gpio_write(leds[i].pin, 1);
	}

	/* Stop after 5 iterations */
	if (led_cntr > (ARRAY_SIZE(leds) * 5)) {
		led_cntr = 0;
		return;
	}

	/* Select and enable current LED */
	i = led_cntr++ % ARRAY_SIZE(leds);
	hal_gpio_write(leds[i].pin, 0);

	k_delayed_work_submit(&led_timer, K_MSEC(100));
}

static int configure_leds(void)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(leds); i++) {
		hal_gpio_init_out(leds[i].pin, 1);
	}

	k_delayed_work_init(&led_timer, led_timeout);
	return 0;
}

static int erase_storage(void)
{
	bt_set_name(MYNEWT_VAL(BLE_SVC_GAP_DEVICE_NAME));
	ble_store_clear();
	schedule_mesh_reset();
	return 0;
}

void board_refresh_display(void)
{
	k_delayed_work_submit(&epd_work, K_NO_WAIT);
}

int board_init(void)
{
	epd_dev = os_dev_lookup(MYNEWT_VAL(SSD1673_OS_DEV_NAME));
	if (epd_dev == NULL) {
		printk("SSD1673 device not found\n");
		return -ENODEV;
	}

	if (cfb_framebuffer_init(epd_dev)) {
		printk("Framebuffer initialization failed\n");
		return -EIO;
	}

	cfb_framebuffer_clear(epd_dev, true);

	if (configure_button()) {
		printk("Failed to configure button\n");
		return -EIO;
	}

	if (configure_leds()) {
		printk("LED init failed\n");
		return -EIO;
	}

	k_delayed_work_init(&epd_work, epd_update);
	k_delayed_work_init(&long_press_work, long_press);

	pressed = button_is_pressed();
	if (pressed) {
		printk("Erasing storage\n");
		board_show_text("Resetting Device", false, K_SECONDS(4));
		erase_storage();
	}

	return 0;
}
