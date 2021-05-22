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

#ifndef _DEVICE_COMPOSITION_H
#define _DEVICE_COMPOSITION_H

#define CID_RUNTIME 0x05C3

#define STATE_OFF	0x00
#define STATE_ON	0x01
#define STATE_DEFAULT	0x01
#define STATE_RESTORE	0x02

/* Following 4 values are as per Mesh Model specification */
#define LIGHTNESS_MIN	0x0001
#define LIGHTNESS_MAX	0xFFFF
#define TEMP_MIN	0x0320
#define TEMP_MAX	0x4E20

/* Refer 7.2 of Mesh Model Specification */
#define RANGE_SUCCESSFULLY_UPDATED	0x00
#define CANNOT_SET_RANGE_MIN		0x01
#define CANNOT_SET_RANGE_MAX		0x02

struct generic_onoff_state {
	uint8_t onoff;
	uint8_t target_onoff;

	uint8_t last_tid;
	uint16_t last_src_addr;
	uint16_t last_dst_addr;
	int64_t last_msg_timestamp;

	int32_t tt_delta;

	struct transition *transition;
};

struct generic_level_state {
	int16_t level;
	int16_t target_level;

	int16_t last_level;
	int32_t last_delta;

	uint8_t last_tid;
	uint16_t last_src_addr;
	uint16_t last_dst_addr;
	int64_t last_msg_timestamp;

	int32_t tt_delta;

	struct transition *transition;
};

struct generic_onpowerup_state {
	uint8_t onpowerup;
};

struct gen_def_trans_time_state {
	uint8_t tt;
};

struct vendor_state {
	int current;
	uint32_t response;
	uint8_t last_tid;
	uint16_t last_src_addr;
	uint16_t last_dst_addr;
	int64_t last_msg_timestamp;
};

struct light_lightness_state {
	uint16_t linear;
	uint16_t target_linear;

	uint16_t actual;
	uint16_t target_actual;

	uint16_t last;
	uint16_t def;

	uint8_t status_code;
	uint16_t light_range_min;
	uint16_t light_range_max;
	uint32_t lightness_range;

	uint8_t last_tid;
	uint16_t last_src_addr;
	uint16_t last_dst_addr;
	int64_t last_msg_timestamp;

	int32_t tt_delta_actual;
	int32_t tt_delta_linear;

	struct transition *transition;
};

struct light_ctl_state {
	uint16_t lightness;
	uint16_t target_lightness;

	uint16_t temp;
	uint16_t target_temp;

	int16_t delta_uv;
	int16_t target_delta_uv;

	uint8_t status_code;
	uint16_t temp_range_min;
	uint16_t temp_range_max;
	uint32_t temperature_range;

	uint16_t lightness_def;
	uint16_t temp_def;
	uint32_t lightness_temp_def;
	int16_t delta_uv_def;

	uint32_t lightness_temp_last;

	uint8_t last_tid;
	uint16_t last_src_addr;
	uint16_t last_dst_addr;
	int64_t last_msg_timestamp;

	int32_t tt_delta_lightness;
	int32_t tt_delta_temp;
	int32_t tt_delta_duv;

	struct transition *transition;
};

extern struct generic_onoff_state gen_onoff_srv_root_user_data;
extern struct generic_level_state gen_level_srv_root_user_data;
extern struct gen_def_trans_time_state gen_def_trans_time_srv_user_data;
extern struct generic_onpowerup_state gen_power_onoff_srv_user_data;
extern struct light_lightness_state light_lightness_srv_user_data;
extern struct light_ctl_state light_ctl_srv_user_data;
extern struct generic_level_state gen_level_srv_s0_user_data;

extern struct bt_mesh_model root_models[];
extern struct bt_mesh_model vnd_models[];
extern struct bt_mesh_model s0_models[];

extern const struct bt_mesh_comp comp;

void gen_onoff_publish(struct bt_mesh_model *model);
void gen_level_publish(struct bt_mesh_model *model);
void light_lightness_publish(struct bt_mesh_model *model);
void light_lightness_linear_publish(struct bt_mesh_model *model);
void light_ctl_publish(struct bt_mesh_model *model);
void light_ctl_temp_publish(struct bt_mesh_model *model);

#endif
