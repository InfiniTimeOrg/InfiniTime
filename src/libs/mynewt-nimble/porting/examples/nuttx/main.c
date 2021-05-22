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

#include <nuttx/config.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>


#include <pthread.h>
#include "nimble/nimble_npl.h"
#include "nimble/nimble_port.h"

#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include "services/ans/ble_svc_ans.h"
#include "services/ias/ble_svc_ias.h"
#include "services/lls/ble_svc_lls.h"
#include "services/tps/ble_svc_tps.h"

static struct ble_npl_task s_task_host;
static struct ble_npl_task s_task_hci;

void nimble_host_task(void *param);
void ble_hci_sock_ack_handler(void *param);
void ble_hci_sock_init(void);
void ble_hci_sock_set_device(int dev);
void ble_store_ram_init(void);

#define TASK_DEFAULT_PRIORITY       1
#define TASK_DEFAULT_STACK          NULL
#define TASK_DEFAULT_STACK_SIZE     400

void *ble_hci_sock_task(void *param)
{
    printf("hci sock task\n");
    ble_hci_sock_ack_handler(param);
    return NULL;
}

void *ble_host_task(void *param)
{
    printf("host task\n");
    nimble_host_task(param);
    return NULL;
}

int main(int argc, char *argv[])
{
    int ret = 0;

    /* allow to specify custom hci */
    if (argc > 1) {
        ble_hci_sock_set_device(atoi(argv[1]));
    }

    printf("hci init\n");
    ble_hci_sock_init();
    printf("port init\n");
    nimble_port_init();

    /* This example provides GATT Alert service */
    printf("gap init\n");
    ble_svc_gap_init();
    printf("gatt init\n");
    ble_svc_gatt_init();
    printf("ans init\n");
    ble_svc_ans_init();
    printf("ias init\n");
    ble_svc_ias_init();
    printf("lls init\n");
    ble_svc_lls_init();
    printf("tps init\n");
    ble_svc_tps_init();

    /* XXX Need to have template for store */
    ble_store_ram_init();

    printf("hci_sock task init\n");
    ret = ble_npl_task_init(&s_task_hci, "hci_sock", ble_hci_sock_task,
                      NULL, TASK_DEFAULT_PRIORITY, BLE_NPL_TIME_FOREVER,
                      TASK_DEFAULT_STACK, TASK_DEFAULT_STACK_SIZE);

    if (ret != 0)
      {
        fprintf(stderr, "error starting hci task: %i\n", ret);
      }

    /* Create task which handles default event queue for host stack. */
    printf("ble_host task init\n");
    ret = ble_npl_task_init(&s_task_host, "ble_host", ble_host_task,
                      NULL, TASK_DEFAULT_PRIORITY, BLE_NPL_TIME_FOREVER,
                      TASK_DEFAULT_STACK, TASK_DEFAULT_STACK_SIZE);


    if (ret != 0)
      {
        fprintf(stderr, "error starting ble task: %i\n", ret);
      }

    while (true)
      {
        usleep(100);
        //pause();
      }
    return 0;
}
