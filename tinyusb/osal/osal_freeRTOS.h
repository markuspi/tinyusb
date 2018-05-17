/**************************************************************************/
/*!
    @file     osal_freeRTOS.h
    @author   hathach (tinyusb.org)

    @section LICENSE

    Software License Agreement (BSD License)

    Copyright (c) 2013, hathach (tinyusb.org)
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    1. Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    3. Neither the name of the copyright holders nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ''AS IS'' AND ANY
    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    INCLUDING NEGLIGENCE OR OTHERWISE ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    This file is part of the tinyusb stack.
*/
/**************************************************************************/

/** \ingroup group_osal
 * @{
 *  \defgroup Group_FreeRTOS  FreeRTOS
 *  @{ */

#ifndef _TUSB_OSAL_FREERTOS_H_
#define _TUSB_OSAL_FREERTOS_H_

//------------- FreeRTOS Headers -------------//
#include "FreeRTOS.h"
#include "semphr.h"
#include "queue.h"
#include "task.h"

#ifdef __cplusplus
extern "C" {
#endif

#if 0
// Helper to determine if we are in ISR to use ISR API (only cover ARM Cortex)
static inline bool in_isr(void)
{
  return (SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk);
}
#endif

//--------------------------------------------------------------------+
// TASK API
//--------------------------------------------------------------------+
typedef void (*osal_func_t)(void *param);
typedef void* osal_task_t;

static inline osal_task_t osal_task_create(osal_func_t code, const char* name, uint32_t stack_size, void* param, uint32_t prio)
{
  osal_task_t task_hdl;
  xTaskCreate(code, (const char*) name, stack_size, param, prio, &task_hdl);
  return task_hdl;
}

static inline void osal_task_delay(uint32_t msec)
{
  vTaskDelay( pdMS_TO_TICKS(msec) );
}

//--------------------------------------------------------------------+
// QUEUE API
//--------------------------------------------------------------------+
typedef struct
{
  uint16_t queue_sz;
  uint16_t item_sz;
  void*    pool;

  StaticQueue_t sq;
}osal_queue_def_t;

typedef QueueHandle_t osal_queue_t;

static inline osal_queue_t osal_queue_create(osal_queue_def_t* qdef)
{
  return xQueueCreateStatic(qdef->queue_sz, qdef->item_sz, qdef->pool, &qdef->sq);
}

static inline void osal_queue_receive (osal_queue_t const queue_hdl, void *p_data, uint32_t msec, tusb_error_t *p_error)
{
  uint32_t const ticks = (msec == OSAL_TIMEOUT_WAIT_FOREVER) ? portMAX_DELAY : pdMS_TO_TICKS(msec);
  (*p_error) = ( xQueueReceive(queue_hdl, p_data, ticks) ? TUSB_ERROR_NONE : TUSB_ERROR_OSAL_TIMEOUT);
}

static inline bool osal_queue_send_isr(osal_queue_t const queue_hdl, void const * data)
{
  return xQueueSendToBackFromISR(queue_hdl, data, NULL);
}

static inline bool osal_queue_send(osal_queue_t const queue_hdl, void const * data)
{
  return xQueueSendToBack(queue_hdl, data, OSAL_TIMEOUT_WAIT_FOREVER) == pdTRUE;
}

static inline void osal_queue_flush(osal_queue_t const queue_hdl)
{
  // TODO move to thread context
//  xQueueReset(queue_hdl);
}

//--------------------------------------------------------------------+
// Semaphore API
//--------------------------------------------------------------------+
typedef StaticSemaphore_t osal_semaphore_def_t;
typedef SemaphoreHandle_t osal_semaphore_t;

static inline osal_semaphore_t osal_semaphore_create(osal_semaphore_def_t* semdef)
{
  return xSemaphoreCreateBinaryStatic(semdef);
}

static inline bool osal_semaphore_post_isr(osal_semaphore_t sem_hdl)
{
  return xSemaphoreGiveFromISR(sem_hdl, NULL) == pdTRUE;
}

static inline bool osal_semaphore_post(osal_semaphore_t sem_hdl)
{
  return xSemaphoreGive(sem_hdl) == pdTRUE;
}

static inline void osal_semaphore_wait(osal_semaphore_t sem_hdl, uint32_t msec, tusb_error_t *p_error)
{
  uint32_t const ticks = (msec == OSAL_TIMEOUT_WAIT_FOREVER) ? portMAX_DELAY : pdMS_TO_TICKS(msec);
  (*p_error) = (xSemaphoreTake(sem_hdl, ticks) ? TUSB_ERROR_NONE : TUSB_ERROR_OSAL_TIMEOUT);
}

static inline void osal_semaphore_reset_isr(osal_semaphore_t const sem_hdl)
{
  xSemaphoreTakeFromISR(sem_hdl, NULL);
}

//--------------------------------------------------------------------+
// MUTEX API (priority inheritance)
//--------------------------------------------------------------------+
typedef SemaphoreHandle_t osal_mutex_t;

#define osal_mutex_create(x) xSemaphoreCreateMutex()

static inline bool osal_mutex_release(osal_mutex_t mutex_hdl)
{
  return xSemaphoreGive(mutex_hdl);
}

static inline void osal_mutex_wait(osal_mutex_t mutex_hdl, uint32_t msec, tusb_error_t *p_error)
{
  uint32_t const ticks = (msec == OSAL_TIMEOUT_WAIT_FOREVER) ? portMAX_DELAY : pdMS_TO_TICKS(msec);
  (*p_error) = (xSemaphoreTake(mutex_hdl, ticks) ? TUSB_ERROR_NONE : TUSB_ERROR_OSAL_TIMEOUT);
}


#ifdef __cplusplus
 }
#endif

#endif /* _TUSB_OSAL_FREERTOS_H_ */

/** @} */
/** @} */

