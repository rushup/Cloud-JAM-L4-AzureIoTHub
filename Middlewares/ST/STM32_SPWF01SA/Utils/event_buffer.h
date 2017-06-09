  /**
  ******************************************************************************
  * @file    event_buffer.h
  * @author  Central LAB
  * @version V2.0.1
  * @date    17-May-2016
  * @brief   Header File for Event Buffer management of the Wi-Fi module
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

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include "wifi_module.h"
#ifndef __EVENT_BUFFER_H
#define __EVENT_BUFFER_H

/** @addtogroup BSP
* @{
*/ 

/** @defgroup  NUCLEO_WIFI_BUFFER_MGMT 
  * @brief Wi-Fi_driver modules
  * @{
  */ 

/** @defgroup NUCLEO_WIFI_BUFFER_MGMT_Private_TypesDefinitions
  * @{
  */

/**
  * @}
  */

/** @defgroup NUCLEO_WIFI_BUFFER_MGMT_Private_Defines
  * @{
  */

/**
  * @}
  */

/** @defgroup NUCLEO_WIFI_BUFFER_MGMT_Private_Macros
  * @{
  */

/**
  * @}
  */

/** @defgroup NUCLEO_WIFI_BUFFER_MGMT_Private_Variables
  * @{
  */


/**
  * @brief  WiFi Events enumeration declaration
  */    
typedef enum
{
  WIFI_NO_EVENT,
  WIFI_WIND_EVENT = 0x0001,
  WIFI_SOCK_ID_EVENT,
  WIFI_GCFG_EVENT,
  WIFI_GPIO_EVENT,
  WIFI_OK_EVENT,
  WIFI_ERROR_EVENT,
  WIFI_STANDBY_CONFIG_EVENT,
  WIFI_RESUME_CONFIG_EVENT,
  WIFI_HTTP_EVENT,
  WIFI_CLIENT_SOCKET_WRITE_EVENT,
  WIFI_FILE_EVENT,
  WIFI_CLIENT_SOCKET_OPEN_EVENT,
  WIFI_CLIENT_SOCKET_CLOSE_EVENT,
  WIFI_FW_UPDATE_EVENT,
} WiFi_Events;


/**
  * @brief  Event structure definiton
  */
typedef struct
{
  uint32_t wind;
  uint32_t data_length;
  uint16_t socket_id;
  uint8_t  wind64_pending_packet_no;
  wifi_bool enc;
  wifi_bool ok_eval;
  wifi_bool event_pop;
  WiFi_Events event;
} wifi_event_TypeDef;


/**
  * @brief  Event Buffer structure definiton
  */
struct event_buffer 
{
    volatile int start;  // position of first data from USART
    volatile int end;    // position of last data from USART
    volatile int size;   // Max size in terms of number of data packets (Total Bytes/size of each packet (8 bytes))
    volatile int count;  // number of currently filled data packets (=size if full & =0 if empty)

    /*unsigned main buffer pointer*/
    wifi_event_TypeDef *element;
};

typedef struct event_buffer wifi_event_buffer;

int event_full(wifi_event_buffer *buffer);
int event_empty(wifi_event_buffer *buffer);
void reset_event(wifi_event_TypeDef *r_event);
void event_init(wifi_event_buffer *buffer, int size);
void push_eventbuffer_queue(wifi_event_buffer *buffer, wifi_event_TypeDef data);
wifi_event_TypeDef * pop_eventbuffer_queue(wifi_event_buffer *buffer);

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */
  
/**
  * @}
  */



#endif
