/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

/* Bluetooth: Mesh Generic OnOff, Generic Level, Lighting & Vendor Models
 *
 * Copyright (c) 2018 Vikrant More
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <math.h>

#include "ble_mesh.h"
#include "device_composition.h"
#include "state_binding.h"
#include "transition.h"


uint16_t lightness, target_lightness;
int16_t temperature, target_temperature;

static int32_t ceiling(float num)
{
	int32_t inum;

	inum = (int32_t) num;
	if (num == (float) inum) {
		return inum;
	}

	return inum + 1;
}

uint16_t actual_to_linear(uint16_t val)
{
	float tmp;

	tmp = ((float) val / 65535);

	return (uint16_t) ceiling(65535 * tmp * tmp);
}

uint16_t linear_to_actual(uint16_t val)
{
	return (uint16_t) (65535 * sqrt(((float) val / 65535)));
}

static void constrain_lightness(uint16_t var)
{
	if (var > 0 && var < light_lightness_srv_user_data.light_range_min) {
		var = light_lightness_srv_user_data.light_range_min;
	} else if (var > light_lightness_srv_user_data.light_range_max) {
		var = light_lightness_srv_user_data.light_range_max;
	}

	lightness = var;
}

static void constrain_lightness2(uint16_t var)
{
	/* This is as per Mesh Model Specification 3.3.2.2.3 */
	if (var > 0 && var < light_lightness_srv_user_data.light_range_min) {
		if (gen_level_srv_root_user_data.last_delta < 0) {
			var = 0U;
		} else {
			var = light_lightness_srv_user_data.light_range_min;
		}
	} else if (var > light_lightness_srv_user_data.light_range_max) {
		var = light_lightness_srv_user_data.light_range_max;
	}

	lightness = var;
}

static void constrain_target_lightness(uint16_t var)
{
	if (var > 0 &&
	    var < light_lightness_srv_user_data.light_range_min) {
		var = light_lightness_srv_user_data.light_range_min;
	} else if (var > light_lightness_srv_user_data.light_range_max) {
		var = light_lightness_srv_user_data.light_range_max;
	}

	target_lightness = var;
}

static int16_t light_ctl_temp_to_level(uint16_t temp)
{
	float tmp;

	/* Mesh Model Specification 6.1.3.1.1 2nd formula start */

	tmp = (temp - light_ctl_srv_user_data.temp_range_min) * 65535;

	tmp = tmp / (light_ctl_srv_user_data.temp_range_max -
		     light_ctl_srv_user_data.temp_range_min);

	return (int16_t) (tmp - 32768);

	/* 6.1.3.1.1 2nd formula end */
}

static uint16_t level_to_light_ctl_temp(int16_t level)
{
	uint16_t tmp;
	float diff;

	/* Mesh Model Specification 6.1.3.1.1 1st formula start */
	diff = (float) (light_ctl_srv_user_data.temp_range_max -
			light_ctl_srv_user_data.temp_range_min) / 65535;


	tmp = (uint16_t) ((level + 32768) * diff);

	return (light_ctl_srv_user_data.temp_range_min + tmp);

	/* 6.1.3.1.1 1st formula end */
}

void state_binding(uint8_t light, uint8_t temp)
{
	switch (temp) {
	case ONOFF_TEMP:
	case CTL_TEMP:
		temperature =
			light_ctl_temp_to_level(light_ctl_srv_user_data.temp);

		gen_level_srv_s0_user_data.level = temperature;
		break;
	case LEVEL_TEMP:
		temperature = gen_level_srv_s0_user_data.level;
		light_ctl_srv_user_data.temp =
			level_to_light_ctl_temp(temperature);
		break;
	default:
		break;
	}

	switch (light) {
	case ONPOWERUP:
		if (gen_onoff_srv_root_user_data.onoff == STATE_OFF) {
			lightness = 0U;
		} else if (gen_onoff_srv_root_user_data.onoff == STATE_ON) {
			lightness = light_lightness_srv_user_data.last;
		}
		break;
	case ONOFF:
		if (gen_onoff_srv_root_user_data.onoff == STATE_OFF) {
			lightness = 0U;
		} else if (gen_onoff_srv_root_user_data.onoff == STATE_ON) {
			if (light_lightness_srv_user_data.def == 0) {
				lightness = light_lightness_srv_user_data.last;
			} else {
				lightness = light_lightness_srv_user_data.def;
			}
		}
		break;
	case LEVEL:
		lightness = gen_level_srv_root_user_data.level + 32768;
		break;
	case DELTA_LEVEL:
		lightness = gen_level_srv_root_user_data.level + 32768;
		constrain_lightness2(lightness);
		goto jump;
	case ACTUAL:
		lightness = light_lightness_srv_user_data.actual;
		break;
	case LINEAR:
		lightness =
			linear_to_actual(light_lightness_srv_user_data.linear);
		break;
	case CTL:
		lightness = light_ctl_srv_user_data.lightness;
		break;
	default:
		break;
	}

	constrain_lightness(lightness);

jump:
	if (lightness != 0) {
		light_lightness_srv_user_data.last = lightness;
	}

	if (lightness) {
		gen_onoff_srv_root_user_data.onoff = STATE_ON;
	} else {
		gen_onoff_srv_root_user_data.onoff = STATE_OFF;
	}

	gen_level_srv_root_user_data.level = lightness - 32768;
	light_lightness_srv_user_data.actual = lightness;
	light_lightness_srv_user_data.linear = actual_to_linear(lightness);
	light_ctl_srv_user_data.lightness = lightness;
}

void calculate_lightness_target_values(uint8_t type)
{
	bool set_light_ctl_temp_target_value;
	uint16_t tmp;

	set_light_ctl_temp_target_value = true;

	switch (type) {
	case ONOFF:
		if (gen_onoff_srv_root_user_data.target_onoff == 0) {
			tmp = 0U;
		} else {
			if (light_lightness_srv_user_data.def == 0) {
				tmp = light_lightness_srv_user_data.last;
			} else {
				tmp = light_lightness_srv_user_data.def;
			}
		}
		break;
	case LEVEL:
		tmp = gen_level_srv_root_user_data.target_level + 32768;
		break;
	case ACTUAL:
		tmp = light_lightness_srv_user_data.target_actual;
		break;
	case LINEAR:
		tmp = linear_to_actual(light_lightness_srv_user_data.target_linear);
		break;
	case CTL:
		set_light_ctl_temp_target_value = false;

		tmp = light_ctl_srv_user_data.target_lightness;

		target_temperature = light_ctl_temp_to_level(light_ctl_srv_user_data.target_temp);
		gen_level_srv_s0_user_data.target_level = target_temperature;
		break;
	default:
		return;
	}

	constrain_target_lightness(tmp);

	if (target_lightness) {
		gen_onoff_srv_root_user_data.target_onoff = STATE_ON;
	} else {
		gen_onoff_srv_root_user_data.target_onoff = STATE_OFF;
	}

	gen_level_srv_root_user_data.target_level = target_lightness - 32768;

	light_lightness_srv_user_data.target_actual = target_lightness;

	light_lightness_srv_user_data.target_linear =
		actual_to_linear(target_lightness);

	light_ctl_srv_user_data.target_lightness = target_lightness;

	if (set_light_ctl_temp_target_value) {
		target_temperature = light_ctl_srv_user_data.temp;
		light_ctl_srv_user_data.target_temp = target_temperature;
	}
}

void calculate_temp_target_values(uint8_t type)
{
	bool set_light_ctl_delta_uv_target_value;

	set_light_ctl_delta_uv_target_value = true;

	switch (type) {
	case LEVEL_TEMP:
		target_temperature = gen_level_srv_s0_user_data.target_level;
		light_ctl_srv_user_data.target_temp =
			level_to_light_ctl_temp(target_temperature);
		break;
	case CTL_TEMP:
		set_light_ctl_delta_uv_target_value = false;

		target_temperature = light_ctl_temp_to_level(light_ctl_srv_user_data.target_temp);
		gen_level_srv_s0_user_data.target_level = target_temperature;
		break;
	default:
		return;
	}

	target_lightness = light_ctl_srv_user_data.lightness;
	light_ctl_srv_user_data.target_lightness = target_lightness;

	if (set_light_ctl_delta_uv_target_value) {

		light_ctl_srv_user_data.target_delta_uv =
			light_ctl_srv_user_data.delta_uv;
	}
}

