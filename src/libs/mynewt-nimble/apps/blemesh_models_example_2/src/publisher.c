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

#include "console/console.h"
#include "hal/hal_gpio.h"

#include "app_gpio.h"

#include "ble_mesh.h"
#include "device_composition.h"
#include "publisher.h"

#define ONOFF
#define GENERIC_LEVEL
/* #define LIGHT_CTL */
/* #define LIGHT_CTL_TEMP */

static bool is_randomization_of_TIDs_done;

#if (defined(ONOFF) || defined(ONOFF_TT))
static u8_t tid_onoff;
#elif defined(VND_MODEL_TEST)
static u8_t tid_vnd;
#endif

static u8_t tid_level;

void randomize_publishers_TID(void)
{
#if (defined(ONOFF) || defined(ONOFF_TT))
	bt_rand(&tid_onoff, sizeof(tid_onoff));
#elif defined(VND_MODEL_TEST)
	bt_rand(&tid_vnd, sizeof(tid_vnd));
#endif

	bt_rand(&tid_level, sizeof(tid_level));

	is_randomization_of_TIDs_done = true;
}

static u32_t button_read(int button)
{
	return (uint32_t) hal_gpio_read(button);
}

void publish(struct os_event *work)
{
	int err = 0;

	if (is_randomization_of_TIDs_done == false) {
		return;
	}

	if (button_read(button_device[0]) == 0) {
#if defined(ONOFF)
		bt_mesh_model_msg_init(root_models[3].pub->msg,
				       BT_MESH_MODEL_OP_GEN_ONOFF_SET_UNACK);
		net_buf_simple_add_u8(root_models[3].pub->msg, 0x01);
		net_buf_simple_add_u8(root_models[3].pub->msg, tid_onoff++);
		err = bt_mesh_model_publish(&root_models[3]);
#elif defined(ONOFF_TT)
		bt_mesh_model_msg_init(root_models[3].pub->msg,
				       BT_MESH_MODEL_OP_GEN_ONOFF_SET_UNACK);
		net_buf_simple_add_u8(root_models[3].pub->msg, 0x01);
		net_buf_simple_add_u8(root_models[3].pub->msg, tid_onoff++);
		net_buf_simple_add_u8(root_models[3].pub->msg, 0x45);
		net_buf_simple_add_u8(root_models[3].pub->msg, 0x28);
		err = bt_mesh_model_publish(&root_models[3]);
#elif defined(VND_MODEL_TEST)
		bt_mesh_model_msg_init(vnd_models[0].pub->msg,
				       BT_MESH_MODEL_OP_3(0x03, CID_RUNTIME));
		net_buf_simple_add_le16(vnd_models[0].pub->msg, 0x0001);
		net_buf_simple_add_u8(vnd_models[0].pub->msg, tid_vnd++);
		err = bt_mesh_model_publish(&vnd_models[0]);
#endif

	} else if (button_read(button_device[1]) == 0) {
#if defined(ONOFF)
		bt_mesh_model_msg_init(root_models[3].pub->msg,
				       BT_MESH_MODEL_OP_GEN_ONOFF_SET_UNACK);
		net_buf_simple_add_u8(root_models[3].pub->msg, 0x00);
		net_buf_simple_add_u8(root_models[3].pub->msg, tid_onoff++);
		err = bt_mesh_model_publish(&root_models[3]);
#elif defined(ONOFF_TT)
		bt_mesh_model_msg_init(root_models[3].pub->msg,
				       BT_MESH_MODEL_OP_GEN_ONOFF_SET_UNACK);
		net_buf_simple_add_u8(root_models[3].pub->msg, 0x00);
		net_buf_simple_add_u8(root_models[3].pub->msg, tid_onoff++);
		net_buf_simple_add_u8(root_models[3].pub->msg, 0x45);
		net_buf_simple_add_u8(root_models[3].pub->msg, 0x28);
		err = bt_mesh_model_publish(&root_models[3]);
#elif defined(VND_MODEL_TEST)
		bt_mesh_model_msg_init(vnd_models[0].pub->msg,
				       BT_MESH_MODEL_OP_3(0x03, CID_RUNTIME));
		net_buf_simple_add_le16(vnd_models[0].pub->msg, 0x0000);
		net_buf_simple_add_u8(vnd_models[0].pub->msg, tid_vnd++);
		err = bt_mesh_model_publish(&vnd_models[0]);
#endif

	} else if (button_read(button_device[2]) == 0) {
#if defined(GENERIC_LEVEL)
		bt_mesh_model_msg_init(root_models[5].pub->msg,
				       BT_MESH_MODEL_OP_GEN_LEVEL_SET_UNACK);
		net_buf_simple_add_le16(root_models[5].pub->msg, LEVEL_S25);
		net_buf_simple_add_u8(root_models[5].pub->msg, tid_level++);
		err = bt_mesh_model_publish(&root_models[5]);
#elif defined(ONOFF_GET)
		bt_mesh_model_msg_init(root_models[3].pub->msg,
				       BT_MESH_MODEL_OP_GEN_ONOFF_GET);
		err = bt_mesh_model_publish(&root_models[3]);
#elif defined(GENERIC_DELTA_LEVEL)
		bt_mesh_model_msg_init(root_models[5].pub->msg,
				       BT_MESH_MODEL_OP_GEN_DELTA_SET_UNACK);
		net_buf_simple_add_le32(root_models[5].pub->msg, 100);
		net_buf_simple_add_u8(root_models[5].pub->msg, tid_level++);
		err = bt_mesh_model_publish(&root_models[5]);
#elif defined(GENERIC_MOVE_LEVEL_TT)
		bt_mesh_model_msg_init(root_models[5].pub->msg,
				       BT_MESH_MODEL_OP_GEN_MOVE_SET_UNACK);
		net_buf_simple_add_le16(root_models[5].pub->msg, 13100);
		net_buf_simple_add_u8(root_models[5].pub->msg, tid_level++);
		net_buf_simple_add_u8(root_models[5].pub->msg, 0x45);
		net_buf_simple_add_u8(root_models[5].pub->msg, 0x00);
		err = bt_mesh_model_publish(&root_models[5]);
#elif defined(LIGHT_LIGHTNESS_TT)
		bt_mesh_model_msg_init(root_models[13].pub->msg,
				       BT_MESH_MODEL_OP_2(0x82, 0x4D));
		net_buf_simple_add_le16(root_models[13].pub->msg, LEVEL_U25);
		net_buf_simple_add_u8(root_models[13].pub->msg, tid_level++);
		net_buf_simple_add_u8(root_models[13].pub->msg, 0x45);
		net_buf_simple_add_u8(root_models[13].pub->msg, 0x28);
		err = bt_mesh_model_publish(&root_models[13]);
#elif defined(LIGHT_CTL)
		bt_mesh_model_msg_init(root_models[16].pub->msg,
				       BT_MESH_MODEL_OP_2(0x82, 0x5F));
		/* Lightness */
		net_buf_simple_add_le16(root_models[16].pub->msg, LEVEL_U25);
		/* Temperature (value should be from 0x0320 to 0x4E20 */
		/* This is as per 6.1.3.1 in Mesh Model Specification */
		net_buf_simple_add_le16(root_models[16].pub->msg, 0x0320);
		/* Delta UV */
		net_buf_simple_add_le16(root_models[16].pub->msg, 0x0000);
		net_buf_simple_add_u8(root_models[16].pub->msg, tid_level++);
		err = bt_mesh_model_publish(&root_models[16]);
#elif defined(LIGHT_CTL_TT)
		bt_mesh_model_msg_init(root_models[16].pub->msg,
				       BT_MESH_MODEL_OP_2(0x82, 0x5F));
		/* Lightness */
		net_buf_simple_add_le16(root_models[16].pub->msg, LEVEL_U25);
		/* Temperature (value should be from 0x0320 to 0x4E20 */
		/* This is as per 6.1.3.1 in Mesh Model Specification */
		net_buf_simple_add_le16(root_models[16].pub->msg, 0x0320);
		/* Delta UV */
		net_buf_simple_add_le16(root_models[16].pub->msg, 0x0000);
		net_buf_simple_add_u8(root_models[16].pub->msg, tid_level++);
		net_buf_simple_add_u8(root_models[16].pub->msg, 0x45);
		net_buf_simple_add_u8(root_models[16].pub->msg, 0x00);
		err = bt_mesh_model_publish(&root_models[16]);
#elif defined(LIGHT_CTL_TEMP)
		bt_mesh_model_msg_init(root_models[16].pub->msg,
				       BT_MESH_MODEL_OP_2(0x82, 0x65));
		/* Temperature (value should be from 0x0320 to 0x4E20 */
		/* This is as per 6.1.3.1 in Mesh Model Specification */
		net_buf_simple_add_le16(root_models[16].pub->msg, 0x0320);
		/* Delta UV */
		net_buf_simple_add_le16(root_models[16].pub->msg, 0x0000);
		net_buf_simple_add_u8(root_models[16].pub->msg, tid_level++);
		err = bt_mesh_model_publish(&root_models[16]);
#endif

	} else if (button_read(button_device[3]) == 0) {
#if defined(GENERIC_LEVEL)
		bt_mesh_model_msg_init(root_models[5].pub->msg,
				       BT_MESH_MODEL_OP_GEN_LEVEL_SET_UNACK);
		net_buf_simple_add_le16(root_models[5].pub->msg, LEVEL_S100);
		net_buf_simple_add_u8(root_models[5].pub->msg, tid_level++);
		err = bt_mesh_model_publish(&root_models[5]);
#elif defined(GENERIC_DELTA_LEVEL)
		bt_mesh_model_msg_init(root_models[5].pub->msg,
				       BT_MESH_MODEL_OP_GEN_DELTA_SET_UNACK);
		net_buf_simple_add_le32(root_models[5].pub->msg, -100);
		net_buf_simple_add_u8(root_models[5].pub->msg, tid_level++);
		err = bt_mesh_model_publish(&root_models[5]);
#elif defined(GENERIC_MOVE_LEVEL_TT)
		bt_mesh_model_msg_init(root_models[5].pub->msg,
				       BT_MESH_MODEL_OP_GEN_MOVE_SET_UNACK);
		net_buf_simple_add_le16(root_models[5].pub->msg, -13100);
		net_buf_simple_add_u8(root_models[5].pub->msg, tid_level++);
		net_buf_simple_add_u8(root_models[5].pub->msg, 0x45);
		net_buf_simple_add_u8(root_models[5].pub->msg, 0x00);
		err = bt_mesh_model_publish(&root_models[5]);
#elif defined(LIGHT_LIGHTNESS_TT)
		bt_mesh_model_msg_init(root_models[13].pub->msg,
				       BT_MESH_MODEL_OP_2(0x82, 0x4D));
		net_buf_simple_add_le16(root_models[13].pub->msg, LEVEL_U100);
		net_buf_simple_add_u8(root_models[13].pub->msg, tid_level++);
		net_buf_simple_add_u8(root_models[13].pub->msg, 0x45);
		net_buf_simple_add_u8(root_models[13].pub->msg, 0x28);
		err = bt_mesh_model_publish(&root_models[13]);
#elif defined(LIGHT_CTL)
		bt_mesh_model_msg_init(root_models[16].pub->msg,
				       BT_MESH_MODEL_OP_2(0x82, 0x5F));
		/* Lightness */
		net_buf_simple_add_le16(root_models[16].pub->msg, LEVEL_U100);
		/* Temperature (value should be from 0x0320 to 0x4E20 */
		/* This is as per 6.1.3.1 in Mesh Model Specification */
		net_buf_simple_add_le16(root_models[16].pub->msg, 0x4E20);
		/* Delta UV */
		net_buf_simple_add_le16(root_models[16].pub->msg, 0x0000);
		net_buf_simple_add_u8(root_models[16].pub->msg, tid_level++);
		err = bt_mesh_model_publish(&root_models[16]);
#elif defined(LIGHT_CTL_TT)
		bt_mesh_model_msg_init(root_models[16].pub->msg,
				       BT_MESH_MODEL_OP_2(0x82, 0x5F));
		/* Lightness */
		net_buf_simple_add_le16(root_models[16].pub->msg, LEVEL_U100);
		/* Temperature (value should be from 0x0320 to 0x4E20 */
		/* This is as per 6.1.3.1 in Mesh Model Specification */
		net_buf_simple_add_le16(root_models[16].pub->msg, 0x4E20);
		/* Delta UV */
		net_buf_simple_add_le16(root_models[16].pub->msg, 0x0000);
		net_buf_simple_add_u8(root_models[16].pub->msg, tid_level++);
		net_buf_simple_add_u8(root_models[16].pub->msg, 0x45);
		net_buf_simple_add_u8(root_models[16].pub->msg, 0x00);
		err = bt_mesh_model_publish(&root_models[16]);
#elif defined(LIGHT_CTL_TEMP)
		bt_mesh_model_msg_init(root_models[16].pub->msg,
				       BT_MESH_MODEL_OP_2(0x82, 0x65));
		/* Temperature (value should be from 0x0320 to 0x4E20 */
		/* This is as per 6.1.3.1 in Mesh Model Specification */
		net_buf_simple_add_le16(root_models[16].pub->msg, 0x4E20);
		/* Delta UV */
		net_buf_simple_add_le16(root_models[16].pub->msg, 0x0000);
		net_buf_simple_add_u8(root_models[16].pub->msg, tid_level++);
		err = bt_mesh_model_publish(&root_models[16]);
#endif
	}

	if (err) {
		printk("bt_mesh_model_publish: err: %d\n", err);
	}
}

