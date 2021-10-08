//
// Created by robert on 10/7/21.
//

#include <FreeRTOS.h>
#include "task.h"
#include "stack_macros.h"

void vApplicationStackOverflowHook(xTaskHandle *pxTask, signed char *pcTaskName )
{
  xTaskHandle *bad_task_handle = pxTask;     // pointer to crashed task handle
  signed char *bad_task_name = pcTaskName;     // pointer to the name of the crashed task
  
  for( ;; );
}
