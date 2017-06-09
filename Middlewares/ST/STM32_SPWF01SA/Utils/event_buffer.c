/**
 ******************************************************************************
 * @file    event_buffer.c
 * @author  Central LAB
 * @version V2.1.0
 * @date    17-May-2016
 * @brief   Implements the Event Buffer management of the Wi-Fi module
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT(c) 2015 STMicroelectronics</center></h2>
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *   1. Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright notice,
 *      this list of conditions and the following disclaimer in the documentation
 *      and/or other materials provided with the distribution.
 *   3. Neither the name of STMicroelectronics nor the names of its contributors
 *      may be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************
 */

#include <stdio.h>
#include <stdlib.h>
#include "event_buffer.h"
#include "wifi_module.h"
#include "wifi_globals.h"

/** @addtogroup BSP
* @{
*/ 

/** @defgroup  NUCLEO_WIFI_BUFFER_MGMT
  * @brief Wi-Fi_driver modules
  * @{
  */ 

/** @defgroup NUCLEO_WIFI_BUFFER_MGMT_Private_Defines
  * @{
  */

/**
  * @}
  */

/** @defgroup NUCLEO_WIFI_BUFFER_MGMT_Private_Variables
  * @{
  */

#define ELEMENT_SIZE sizeof(wifi_event_TypeDef)   //1

#ifdef USE_STM32L0XX_NUCLEO
static wifi_event_TypeDef event_buffer[10];  /* Used to store various events. Its size varies in case of L0 and other Boards. */
#else
static wifi_event_TypeDef event_buffer[50];
#endif

/**
  * @}
  */

/** @defgroup NUCLEO_WIFI_BUFFER_MGMT_Private_Functions
  * @{
  */

/**
  * @brief  init
  *         Initialize a circular buffer of type wifi_event_buffer
  * @param  *buffer : pointer to event buffer
  * @param  size    : Number of events in Event_Buffer
  * @retval None
  */
void event_init(wifi_event_buffer *buffer, int size) 
{
    uint32_t element_size;
    element_size = sizeof(wifi_event_TypeDef);
    buffer->size = element_size*size;
    buffer->start = 0;
    buffer->count = 0;
    buffer->end = 0;
    buffer->element = event_buffer;
}

/**
  * @brief  full
  *         indicates if the given buffer is full or not
  * @param  *buffer : pointer to event buffer
  * @retval status of event_buffer (=1 if full =0 if otherwise)
  */
int event_full(wifi_event_buffer *buffer) 
{
  int bufsize = buffer->size;
  if (buffer->count == (bufsize/ELEMENT_SIZE)) 
    { 
      return 1;
    } 
  else 
    {
      return 0;
    }
}

/**
  * @brief  empty
  *         indicates if the given buffer is empty or not
  * @param  *buffer : pointer to event buffer
  * @retval status of event_buffer (=1 if empty =0 if otherwise)
  */
int event_empty(wifi_event_buffer *buffer) {
    if (buffer->count == 0) 
      {
        return 1;
      } 
    else 
      {
        return 0;
      }
}

/**
  * @brief  push_buffer
  *         pushes the data structure onto the circular buffer (queues it)
  * @param  *buffer : pointer to event buffer
  * @param  data : data to be Queued in event_buffer
  * @retval None
  */
void push_eventbuffer_queue(wifi_event_buffer *buffer, wifi_event_TypeDef data) 
{
  int bufsize;
  uint32_t index;
  
  if (event_full(buffer))
    {
      return;
    } 
  else 
    {
      index=buffer->end/ELEMENT_SIZE;
      buffer->element[index].data_length = data.data_length;
      buffer->element[index].wind64_pending_packet_no = data.wind64_pending_packet_no;
      buffer->element[index].enc = data.enc;
      buffer->element[index].ok_eval = data.ok_eval;
      buffer->element[index].socket_id = data.socket_id;
      buffer->element[index].wind = data.wind;
      buffer->element[index].event = data.event;
      buffer->element[index].event_pop = data.event_pop;
      buffer->count++;
      buffer->end = buffer->end + ELEMENT_SIZE;
      
      //wrap around if max size is reached
      bufsize = (buffer->size);
      if (buffer->end >= bufsize) 
        {
          buffer->end = 0;
        }
    }
}

/**
  * @brief  pop_buffer_queue
  *         dequeues the circular buffer
  * @param  *buffer : pointer to event buffer
  * @retval None
  */ 
wifi_event_TypeDef * pop_eventbuffer_queue(wifi_event_buffer *buffer) 
{
  int bufsize;
  uint32_t index;

  if (event_empty(buffer)) 
    {
      //printf("\r\Event Buffer Empty!!\r\n");
      return NULL;
    } 
  else
    {
      /* First in First Out*/
      index=buffer->start/ELEMENT_SIZE;
      element.data_length = buffer->element[index].data_length;
      element.enc = buffer->element[index].enc;
      element.ok_eval = buffer->element[index].ok_eval;
      element.socket_id = buffer->element[index].socket_id;
      element.wind64_pending_packet_no = buffer->element[index].wind64_pending_packet_no;
      element.wind = buffer->element[index].wind;
      element.event = buffer->element[index].event;
      element.event_pop = buffer->element[index].event_pop;
      buffer->start = buffer->start + ELEMENT_SIZE;
      buffer->count--;

      bufsize = (buffer->size);
      if (buffer->start >= bufsize)
        {
          buffer->start = 0;
        }
      return &element;
    }
}

/**
  * @brief  reset_event
  *         sets an event to default values
  * @param  *r_event : event to reset
  * @retval None
  */ 
void reset_event(wifi_event_TypeDef *r_event)
{
  r_event->data_length = 0;
  r_event->enc         = WIFI_FALSE;
  r_event->event       = WIFI_NO_EVENT; 
  r_event->ok_eval     = WIFI_FALSE;
  r_event->socket_id   = 9;              //Socket ID 0-7
  r_event->wind64_pending_packet_no = 9; //Max pending packets = 4
  r_event->wind        = 99;             //any default value
  r_event->event_pop   = WIFI_TRUE;
}

/**
  * @}
  */ 

/**
  * @}
  */ 


/**
  * @}
  */ 

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
