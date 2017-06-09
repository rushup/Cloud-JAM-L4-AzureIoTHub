  /**
  ******************************************************************************
  * @file    wifi_globals.h
  * @author  Central LAB
  * @version V2.0.1
  * @date    06-April-2016
  * @brief   Header File for storing all the global variables of WiFi module
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
#include "wifi_const.h"
#include "event_buffer.h"
#include "ring_buffer.h"
#include "wifi_module.h"

#ifndef __WIFI_GLOBALS_H
#define __WIFI_GLOBALS_H

/** @addtogroup MIDDLEWARES
* @{
*/ 

/** @addtogroup NUCLEO_WIFI_MODULE_Private_Macros
  * @{
  */

/** @addtogroup NUCLEO_WIFI_MODULE_Private_Variables
  * @{
  */

/* Private variables ---------------------------------------------------------*/

/***********All Buffers**************/
extern char UserDataBuff[MAX_BUFFER_GLOBAL];   /* Used to store data that is to be send in callback to user */
extern uint8_t pop_buffer[MAX_BUFFER_GLOBAL];
extern wifi_event_TypeDef element;
extern wifi_scan *wifi_scanned_list;          // [MAX_WIFI_SCAN_NETWORK]

extern UART_HandleTypeDef UartWiFiHandle;
extern WiFi_Config_HandleTypeDef WiFi_Config_Variables;
extern WiFi_Counter_Variables_t WiFi_Counter_Variables;
extern WiFi_Control_Variables_t WiFi_Control_Variables;
extern __IO IO_status_flag_typedef IO_status_flag;

extern volatile uint8_t wifi_connected;            //Set once if wifi is connected for first time
extern volatile uint8_t wifi_client_connected;     //Set once if client is connected
extern volatile uint8_t wifi_client_disconnected;  //Set once if client is dis-connected

extern wifi_bool open_sockets[8];              //Max open sockets allowed is 8. Each array element depicts one socket (true=open, false=closed)

#if defined (USE_STM32F4XX_NUCLEO) || (USE_STM32L4XX_NUCLEO)
extern uint8_t WiFi_AT_Cmd_Buff[2048];
#else
extern uint8_t WiFi_AT_Cmd_Buff[1024];
#endif

extern TIM_HandleTypeDef TimHandle,PushTimHandle;

/* Exported macro ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

#endif /* __WIFI_GLOBALS_H */

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/

