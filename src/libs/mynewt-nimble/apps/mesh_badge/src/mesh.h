/*
 * Copyright (c) 2018 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mesh/mesh.h"

void mesh_send_hello(void);

uint16_t mesh_get_addr(void);
bool mesh_is_initialized(void);
void mesh_start(void);
int mesh_init(uint8_t addr_type);
