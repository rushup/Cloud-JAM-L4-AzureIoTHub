/**
 ******************************************************************************
 * @file    wifi_module.c
 * @author  Central LAB
 * @version V2.1.0
 * @date    17-May-2016
 * @brief   Enable Wi-Fi functionality using AT cmd set
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
#include "wifi_module.h"
#include "wifi_globals.h"
#include "TargetFeatures.h"

/** @addtogroup MIDDLEWARES
* @{
*/ 

/** @defgroup  NUCLEO_WIFI_MODULE
  * @brief Wi-Fi driver modules
  * @{
  */

/** @defgroup NUCLEO_WIFI_MODULE_Private_Defines
  * @{
  */

/**
  * @}
  */

/** @addtogroup NUCLEO_WIFI_MODULE_Private_Variables
  * @{
  */

wifi_instances_t wifi_instances;

/***********All Buffers**************/

#if defined (__CC_ARM)
size_t strnlen (const char* s, size_t maxlen);

size_t strnlen (const char* s, size_t maxlen)
  {
    size_t len = 0;
    while ((len <= maxlen) && (*s))
      {
          s++;
          len++;
      }
    return len;
  }
#endif

// [DLI]
#ifdef WIFI_USE_VCOM

uint8_t console_listen_char[1];
uint8_t console_input_char[1];
uint8_t console_send_char[1];
uint8_t console_echo_char[1];
__IO ITStatus console_send_ready = RESET;
__IO ITStatus console_echo_ready = SET;
__IO ITStatus console_push_ready = RESET;

// Virtual-COM UART
void console_input() 
{
  HAL_UART_Receive_IT(&UartMsgHandle, (uint8_t *)console_input_char, 1);
}

void wifi_vcom() 
{
  if (console_push_ready == SET) 
    {
       push_buffer_queue(&wifi_instances.big_buff, WiFi_Counter_Variables.uart_byte);
       console_push_ready = RESET;
       HAL_UART_Receive_IT(&UartWiFiHandle, (uint8_t *)WiFi_Counter_Variables.uart_byte, 1);
    }
 if(console_echo_ready == SET) 
   {
      WiFi_Counter_Variables.temp = pop_buffer_queue(&wifi_instances.big_buff);
      if(WiFi_Counter_Variables.temp != NULL) 
        {
          console_echo_ready = RESET;
          HAL_UART_Transmit_IT(&UartMsgHandle, WiFi_Counter_Variables.temp, 1);
        }
   }
}

#endif

/**
  * @}
  */
  
/** @defgroup NUCLEO_WIFI_MODULE_Private_Functions
  * @{
  */

/**
  * @brief  WiFi_Module_Init
  *         Initialize wifi module
  * @param  None
  * @retval None
  */
void WiFi_Module_Init(void)
{
#ifdef WIFI_USE_VCOM
  console_input();
#endif  
  init(&wifi_instances.big_buff, RINGBUF_SIZE);//Init the ring buffer
  IO_status_flag.wifi_ready = 0; //reset to get user callback on HW started
  wifi_connected = 0; //reset to get user callback on WiFi UP
  Receive_Data();
  Set_WiFi_Counter_Variables( );
  Set_WiFi_Control_Variables( );
//  WiFi_Variables_Init( );
  
#ifdef USE_STM32L0XX_NUCLEO
  event_init(&wifi_instances.event_buff, 10); //max 15 events can be Q'ed (Event Buffer is of size 15)
#else
  event_init(&wifi_instances.event_buff, 50); //max 50 events can be Q'ed (Event Buffer is of size 50)
#endif
  
#ifndef WIFI_USE_VCOM
  Start_Timer();  
  memset(open_sockets,0x00, 8); //init the open socket array
  
  if(HAL_TIM_Base_Start_IT(&TimHandle) != HAL_OK)//Start the TIM timer
    {
      #if DEBUG_PRINT
      printf("Error");
      #endif
      Error_Handler();
    }
#endif
}

/**
* @brief  Set_WiFi_Control_Variables
*         Sets the default value of some control Variables
* @param  None
* @retval None
*/
void Set_WiFi_Control_Variables(void)
{
  IO_status_flag.enable_dequeue                      = WIFI_TRUE;
  IO_status_flag.command_mode                        = WIFI_TRUE;
  IO_status_flag.wifi_ready                          = WIFI_FALSE;
  IO_status_flag.UartReady                           = RESET;
  IO_status_flag.TxUartReady                         = RESET;
  IO_status_flag.Uart2Ready                          = RESET;
  IO_status_flag.WIND64_count                        = 0;
  IO_status_flag.AT_Response_Received                         = WIFI_FALSE;
  
  WiFi_Control_Variables.switch_by_default_to_command_mode    = WIFI_TRUE;
  WiFi_Control_Variables.queue_wifi_wind_message              = WIFI_TRUE;
  WiFi_Control_Variables.enable_timeout_timer                 = WIFI_FALSE;
  //enable SockOn_Server_Closed_Callback when wind:58 received, not when user requests for socket close.
  WiFi_Control_Variables.enable_SockON_Server_Closed_Callback = WIFI_TRUE;
}

/**
* @brief  Set_WiFi_Counter_Variables
*         Sets the default value of some counter Variables
* @param  None
* @retval None
*/
void Set_WiFi_Counter_Variables(void)
{
  WiFi_Counter_Variables.no_of_open_client_sockets = 0;
  WiFi_Counter_Variables.wind64_DQ_wait            = 0;
  WiFi_Counter_Variables.Socket_Data_Length        = 0;
  WiFi_Counter_Variables.number_of_bytes           = 0;
  WiFi_Counter_Variables.interim_number_of_bytes   = 0;
  WiFi_Counter_Variables.sock_total_count          = 0;
  WiFi_Counter_Variables.pop_buffer_size           = 0;
  WiFi_Counter_Variables.last_process_buffer_index = 5;
  WiFi_Counter_Variables.epoch_time                = 0;
  WiFi_Counter_Variables.sleep_count               = 0;
  WiFi_Counter_Variables.standby_time              = 0;
  WiFi_Counter_Variables.scanned_ssids             = 0;
  WiFi_Counter_Variables.timeout_tick              = 0;
}

/**
* @brief  WiFi_Configuration
*         Default Wifi configuration parameters
* @param  None
* @retval None
*/
void WiFi_Configuration()
{
  WiFi_Config_Variables.blink_led               = 1;
  WiFi_Config_Variables.ip_use_dhcp             = 1;

  /* Set the network privacy mode 
    (0=none, 1=WEP, 2=WPA-Personal (TKIP/AES) or WPA2-Personal (TKIP/AES)) */  
  WiFi_Config_Variables.wifi_mode               =  WiFi_STA_MODE;
  WiFi_Config_Variables.wifi_priv_mode          =  WPA_Personal;
  WiFi_Config_Variables.wifi_ssid               =  "NETGEAR54" ;
  WiFi_Config_Variables.Wifi_Sec_key            =  "12341234";
  
   /*Power Management Settings*/
  WiFi_Config_Variables.sleep_enabled           = 0;//0=disabled, 1=enabled
  WiFi_Config_Variables.standby_enabled         = 1;
  WiFi_Config_Variables.standby_time            = 10;//in seconds
  WiFi_Config_Variables.wifi_powersave          = 1;//0=Active, 1=PS Mode, 2=Fast-PS Mode
  WiFi_Config_Variables.wifi_operational_mode   = 11;//11= Doze mode, 12= Quiescent mode
  WiFi_Config_Variables.wifi_listen_interval    = 0; //Wakeup every n beacon
  WiFi_Config_Variables.wifi_beacon_wakeup      = 1;         
}

/**
* @brief  wifi_reset
*         Reset WiFi module using PC12 gpio pin
* @param  None
* @retval None
*/
void wifi_reset(void)
{  
  RESET_WAKEUP_GPIO_CLK_ENABLE();
  __GPIOA_CLK_ENABLE();

  wifi_instances.GPIO_InitStruct.Pin       = WiFi_RESET_GPIO_PIN;
  wifi_instances.GPIO_InitStruct.Mode      = GPIO_MODE_OUTPUT_PP;
  wifi_instances.GPIO_InitStruct.Pull      = GPIO_PULLUP;
  wifi_instances.GPIO_InitStruct.Speed     = GPIO_SPEED_HIGH;

  HAL_GPIO_Init(WiFi_RESET_GPIO_PORT, &wifi_instances.GPIO_InitStruct);

  IO_status_flag.WiFi_WIND_State.WiFiHWStarted = WIFI_FALSE;
  wifi_connected = 0; //reset wifi_connected to get user callback
  memset((void*)&IO_status_flag.WiFi_WIND_State,0x00,sizeof(IO_status_flag.WiFi_WIND_State)); /*reset the WIND State?*/

  /* ===   RESET PIN - PC12   ===*/
  HAL_GPIO_WritePin(WiFi_RESET_GPIO_PORT, WiFi_RESET_GPIO_PIN, GPIO_PIN_RESET);  
  HAL_Delay(200);  
  /* ===   SET PIN - PC12   ===*/
  HAL_GPIO_WritePin(WiFi_RESET_GPIO_PORT, WiFi_RESET_GPIO_PIN, GPIO_PIN_SET);
  HAL_Delay(50);
  HAL_GPIO_DeInit(WiFi_RESET_GPIO_PORT, WiFi_RESET_GPIO_PIN); 

  while(IO_status_flag.WiFi_WIND_State.WiFiHWStarted != WIFI_TRUE) 
    {
      __NOP(); //nothing to do
    }
}

/**
* @brief  PowerUp_WiFi_Module
*         Power up Wi-Fi module,SET GPIO PA0 pin 
* @param  None
* @retval None
*/
void PowerUp_WiFi_Module(void)
{
  /* ===   SET PIN - PC12   ===*/
  HAL_GPIO_WritePin(WiFi_RESET_GPIO_PORT, WiFi_RESET_GPIO_PIN, GPIO_PIN_SET);
}

/**
* @brief  Receive_Data
*         Receive data from UART port
* @param  uint8_t number of bytes to be received
* @retval None
*/
void Receive_Data(void)
{
  HAL_GPIO_WritePin(WiFi_USART_RTS_GPIO_PORT, WiFi_USART_RTS_PIN, GPIO_PIN_RESET);//Assert RTS
  wifi_instances.receive_status = HAL_UART_Receive_IT(&UartWiFiHandle, (uint8_t *)WiFi_Counter_Variables.uart_byte, 1);
  if(wifi_instances.receive_status!=HAL_OK)
    {
      #if DEBUG_PRINT
      printf("HAL_UARTx_Receive_IT Error");
      #endif
    }
  else 
    {
      WiFi_Control_Variables.Uartx_Rx_Processing = WIFI_TRUE;
    }
}

/**
* @brief  Period elapsed callback in non blocking mode
*         This timer is used for calling back User registered functions with information
* @param  htim : TIM handle
* @retval None
*/
void Wifi_TIM_Handler(TIM_HandleTypeDef *htim)
{
  /**********************************************************************
  *                                                                     *
  *       Be careful not to make a blocking                             *
  *       call from this function, see                                  *
  *       example Socket_Read() and Socket_Close()                      *
  *                                                                     *
  **********************************************************************/
    WiFi_Status_t status = WiFi_MODULE_SUCCESS;

    if(WiFi_Control_Variables.stop_event_dequeue == WIFI_FALSE)
      {
          __disable_irq();
          wifi_instances.DeQed_wifi_event = pop_eventbuffer_queue(&wifi_instances.event_buff);
          __enable_irq();

          if(wifi_instances.DeQed_wifi_event!=NULL && wifi_instances.DeQed_wifi_event->event_pop == WIFI_TRUE)
            {
              switch(wifi_instances.DeQed_wifi_event->event)
                {
                  case WIFI_WIND_EVENT:
                          Process_DeQed_Wind_Indication(wifi_instances.DeQed_wifi_event);
                          break;

                  case WIFI_SOCK_ID_EVENT:
                          /*check ID and update SocketID array*/
                          WiFi_Counter_Variables.no_of_open_client_sockets++;
                          WiFi_Control_Variables.enable_timeout_timer = WIFI_FALSE;
                          WiFi_Counter_Variables.timeout_tick = 0;
                          if(WiFi_Counter_Variables.no_of_open_client_sockets > 8)  //Max number of clients is 8
                            {
                              IO_status_flag.AT_Response_Received = WIFI_TRUE;
                              WiFi_Counter_Variables.AT_RESPONSE = WiFi_NOT_SUPPORTED;     
                              break;
                            }
                          open_sockets[wifi_instances.DeQed_wifi_event->socket_id]  = WIFI_TRUE;
                          WiFi_Counter_Variables.Socket_Open_ID = wifi_instances.DeQed_wifi_event->socket_id;
                          IO_status_flag.AT_Response_Received = WIFI_TRUE;
                          WiFi_Counter_Variables.AT_RESPONSE = WiFi_MODULE_SUCCESS;
                          break;

                  case WIFI_HTTP_EVENT:
                          Reset_AT_CMD_Buffer();  

                          if(WiFi_Counter_Variables.curr_pURL) 
                            {
                                sprintf((char*)WiFi_AT_Cmd_Buff,AT_HTTPPOST_REQUEST,WiFi_Counter_Variables.curr_pURL);
                            }
                          else 
                            {
                                if(WiFi_Counter_Variables.curr_port_number!=0)
                                  sprintf((char*)WiFi_AT_Cmd_Buff,"AT+S.HTTPGET=%s,%s,%d\r",WiFi_Counter_Variables.curr_hostname, WiFi_Counter_Variables.curr_path, (int)WiFi_Counter_Variables.curr_port_number);
                                else 
                                  sprintf((char*)WiFi_AT_Cmd_Buff,"AT+S.HTTPGET=%s,%s\r",WiFi_Counter_Variables.curr_hostname, WiFi_Counter_Variables.curr_path);
                            }

                          status = USART_Transmit_AT_Cmd(strlen((char*)WiFi_AT_Cmd_Buff));
                          if(status == WiFi_MODULE_SUCCESS) 
                            {
                                WiFi_Counter_Variables.timeout_tick = 0;
                                WiFi_Control_Variables.enable_timeout_timer = WIFI_TRUE;
                                WiFi_Control_Variables.stop_event_dequeue = WIFI_TRUE;
                                WiFi_Control_Variables.http_req_pending   = WIFI_TRUE;                            
                            }
                          else
                            {
                                #if DEBUG_PRINT
                                  printf("\r\n ERR DURING HTTP COMMAND TRANSFER \r\n");
                                #endif
                                WiFi_Counter_Variables.AT_RESPONSE = WiFi_AT_CMD_RESP_ERROR;
                                IO_status_flag.AT_Response_Received = WIFI_TRUE;
                            }
                          break;

                  case WIFI_CLIENT_SOCKET_WRITE_EVENT:
                          Reset_AT_CMD_Buffer();
                          /* AT+S.SOCKW=00,11<cr> */

                          sprintf((char*)WiFi_AT_Cmd_Buff,AT_SOCKET_WRITE, WiFi_Counter_Variables.curr_sockID, WiFi_Counter_Variables.curr_DataLength);
                          status = USART_Transmit_AT_Cmd(strlen((char*)WiFi_AT_Cmd_Buff));

                          if(status == WiFi_MODULE_SUCCESS) 
                            {
                                Reset_AT_CMD_Buffer();
                                memcpy((char*)WiFi_AT_Cmd_Buff, (char*)WiFi_Counter_Variables.curr_data, WiFi_Counter_Variables.curr_DataLength);
                                status = USART_Transmit_AT_Cmd(WiFi_Counter_Variables.curr_DataLength);
                            }
                          if(status != WiFi_MODULE_SUCCESS)
                            {
                                #if DEBUG_PRINT
                                  printf("\r\n ERR In Socket Write\r\n");
                                #endif
                                IO_status_flag.AT_Response_Received = WIFI_TRUE;
                                WiFi_Counter_Variables.AT_RESPONSE = WiFi_AT_CMD_RESP_ERROR;
                            }
                          break;

                  case WIFI_CLIENT_SOCKET_OPEN_EVENT:
                          Reset_AT_CMD_Buffer();

                          /* AT+S.SOCKON = myserver,1234,t <cr> */  
                          sprintf((char*)WiFi_AT_Cmd_Buff,AT_SOCKET_OPEN,WiFi_Counter_Variables.curr_hostname,(int)WiFi_Counter_Variables.curr_port_number,WiFi_Counter_Variables.curr_protocol);        
                          status = USART_Transmit_AT_Cmd(strlen((char*)WiFi_AT_Cmd_Buff));
                          if(status != WiFi_MODULE_SUCCESS)
                            {
                                #if DEBUG_PRINT
                                  printf("\r\n ERR During Socket Open \r\n");
                                #endif
                                IO_status_flag.AT_Response_Received = WIFI_TRUE;
                                WiFi_Counter_Variables.AT_RESPONSE = WiFi_AT_CMD_RESP_ERROR;
                            }
                          else
                            {
                                WiFi_Counter_Variables.timeout_tick = 0;
                                WiFi_Control_Variables.enable_timeout_timer = WIFI_TRUE;
                            }
                          break;

                  case WIFI_CLIENT_SOCKET_CLOSE_EVENT:
                          if(open_sockets[wifi_instances.DeQed_wifi_event->socket_id])
                            {
                                Reset_AT_CMD_Buffer();

                                 /* AT+S.SOCKC=00<cr> */
                                sprintf((char*)WiFi_AT_Cmd_Buff,AT_SOCKET_CLOSE,wifi_instances.DeQed_wifi_event->socket_id);
                                status = USART_Transmit_AT_Cmd(strlen((char*)WiFi_AT_Cmd_Buff));
                                if(status == WiFi_MODULE_SUCCESS)
                                  {
                                    WiFi_Counter_Variables.AT_RESPONSE = WiFi_MODULE_SUCCESS;
                                    WiFi_Control_Variables.stop_event_dequeue          = WIFI_TRUE;
                                    WiFi_Counter_Variables.remote_socket_closed_id     = wifi_instances.DeQed_wifi_event->socket_id;
                                    
                                    //for making changes in the value of open_sockets[sock_id] if no error is returned
                                    IO_status_flag.client_socket_close_ongoing = WIFI_TRUE;
                                    
                                    //prevent the OK received after socket close command to be Q'ed
                                    IO_status_flag.prevent_push_OK_event       = WIFI_TRUE;
                                  }
                                else
                                  {
                                    #if DEBUG_PRINT
                                      printf("\r\n ERR During Socket Close \r\n");
                                    #endif
                                  }
                            }
                          else
                            printf("\r\n Socket already close");
                          break; 

                  case WIFI_FILE_EVENT:
                          Reset_AT_CMD_Buffer();

                          if(WiFi_Counter_Variables.curr_filename == NULL)
                            {
                                /* AT+S.FSL */
                                sprintf((char*)WiFi_AT_Cmd_Buff,AT_DISPLAY_FILE_NAME);
                            }
                          else if(WiFi_Counter_Variables.curr_hostname == NULL)
                            {
                                /* AT+S.FSP=/index.html  */
                                sprintf((char*)WiFi_AT_Cmd_Buff,AT_DISPLAY_FILE_CONTENT,WiFi_Counter_Variables.curr_filename);
                            }
                          else
                            {
                                /* AT+S.HTTPDFSUPDATE=%s,/outfile.img  */
                                sprintf((char*)WiFi_AT_Cmd_Buff,AT_DOWNLOAD_IMAGE_FILE,WiFi_Counter_Variables.curr_hostname,WiFi_Counter_Variables.curr_filename,(int)WiFi_Counter_Variables.curr_port_number);
                            }
                          status = USART_Transmit_AT_Cmd(strlen((char*)WiFi_AT_Cmd_Buff));
                          if(status == WiFi_MODULE_SUCCESS)
                            {
                                WiFi_Control_Variables.enable_receive_http_response = WIFI_TRUE;
                                WiFi_Control_Variables.enable_receive_data_chunk    = WIFI_TRUE;
                                WiFi_Control_Variables.enable_receive_file_response = WIFI_TRUE;
                            }
                          else
                            {
                              #if DEBUG_PRINT
                                printf("\r\n ERR DURING FILE OPERATION \r\n");
                              #endif
                              IO_status_flag.AT_Response_Received = WIFI_TRUE;
                              WiFi_Counter_Variables.AT_RESPONSE = WiFi_AT_CMD_RESP_ERROR;
                            }
                          break;

                  case WIFI_FW_UPDATE_EVENT:
                          Reset_AT_CMD_Buffer();
                          sprintf((char*)WiFi_AT_Cmd_Buff,AT_FWUPDATE,WiFi_Counter_Variables.curr_hostname,WiFi_Counter_Variables.curr_filename,(int)WiFi_Counter_Variables.curr_port_number);
                          status = USART_Transmit_AT_Cmd(strlen((char*)WiFi_AT_Cmd_Buff));
                          if(status == WiFi_MODULE_SUCCESS)
                            {
                              WiFi_Control_Variables.enable_fw_update_read      = WIFI_TRUE;
                              WiFi_Control_Variables.enable_receive_data_chunk  = WIFI_TRUE;
                            }
                          else 
                            {
                              #if DEBUG_PRINT
                                printf("\r\n ERR DURING FIRMWARE UPDATE \r\n");
                              #endif
                              IO_status_flag.AT_Response_Received = WIFI_TRUE;
                              WiFi_Counter_Variables.AT_RESPONSE = WiFi_AT_CMD_RESP_ERROR;
                            }
                          break;

                  case WIFI_ERROR_EVENT:
                          #if DEBUG_PRINT
                            printf("\r\n ERR!\r\n");
                          #endif
                          IO_status_flag.AT_Response_Received = WIFI_TRUE;
                          WiFi_Counter_Variables.AT_RESPONSE = WiFi_AT_CMD_RESP_ERROR;
                          break;

                  case WIFI_OK_EVENT:
                  case WIFI_GCFG_EVENT:
                  case WIFI_GPIO_EVENT:
                          IO_status_flag.AT_Response_Received = WIFI_TRUE;
                          WiFi_Counter_Variables.AT_RESPONSE = WiFi_MODULE_SUCCESS; 
                          break;

                  case WIFI_STANDBY_CONFIG_EVENT:
                          #if DEBUG_PRINT
                              printf("\r\nGoing into standby..\r\n");  
                          #endif
                          break;

                  case WIFI_RESUME_CONFIG_EVENT:
                          #if DEBUG_PRINT
                              printf("\r\nResuming from standby..\r\n");
                          #endif
                          IO_status_flag.AT_Response_Received = WIFI_TRUE;//let main run-on
                          break;
                          
                  case WIFI_NO_EVENT:
                          break;
               }
            }
      }
      /* If data is pending on client socket SOCKON, make read requests*/
      if(WiFi_Control_Variables.start_sock_read == WIFI_TRUE)
              {
                  Socket_Read(WiFi_Counter_Variables.Socket_Data_Length);
                  WiFi_Control_Variables.start_sock_read = WIFI_FALSE;
              }

      /* Call Query, after notification for TLS is received */
      else if(WiFi_Control_Variables.enable_query == WIFI_TRUE && IO_status_flag.enable_dequeue == WIFI_TRUE)
              {
                  //@TBD: Flushing the buffer may be detrimental if we have genuine follow on WIND55?
                  Socket_Pending_Data();
                  WiFi_Control_Variables.enable_query = WIFI_FALSE;
              }

      else if(WiFi_Control_Variables.Pending_SockON_Callback==WIFI_TRUE)//for client socket
              {
                  //Now callback to user with user_data pointer <UserDataBuff>              
                  ind_wifi_socket_data_received(WiFi_Counter_Variables.sockon_id_user, (uint8_t *)UserDataBuff, WiFi_Counter_Variables.message_size, WiFi_Counter_Variables.chunk_size);
                  memset(UserDataBuff, 0x00, MAX_BUFFER_GLOBAL);//Flush the buffer
                  Resume_Dequeue();
                  WiFi_Control_Variables.Pending_SockON_Callback=WIFI_FALSE;
              }

      else if(WiFi_Control_Variables.Pending_SockD_Callback == WIFI_TRUE)//for server socket
              {
                  //Now callback to user with user_data pointer <UserDataBuff>
                  ind_wifi_socket_data_received(9, (uint8_t *)UserDataBuff, WiFi_Counter_Variables.message_size, WiFi_Counter_Variables.chunk_size);
                  memset(UserDataBuff, 0x00, MAX_BUFFER_GLOBAL); //Flush the buffer
                  Resume_Dequeue();
                  WiFi_Control_Variables.Pending_SockD_Callback=WIFI_FALSE;      
              }

      else if(WiFi_Control_Variables.Client_Socket_Close_Cmd == WIFI_TRUE)//for client socket
              {
                  // Q the close socket event
                  if(open_sockets[WiFi_Counter_Variables.client_socket_close_id])
                    {
                      Queue_Client_Close_Event(WiFi_Counter_Variables.client_socket_close_id);
                    }
                  WiFi_Control_Variables.Client_Socket_Close_Cmd = WIFI_FALSE;
              }

      else if(WiFi_Control_Variables.SockON_Server_Closed_Callback==WIFI_TRUE)//for client socket
              {
                  //callback the user
                  ind_wifi_socket_client_remote_server_closed(&WiFi_Counter_Variables.closed_socket_id);
                  WiFi_Control_Variables.SockON_Server_Closed_Callback = WIFI_FALSE;
              }

      else if(WiFi_Control_Variables.HTTP_Data_available == WIFI_TRUE)
              {
                  ind_wifi_http_data_available((uint8_t *)UserDataBuff,WiFi_Counter_Variables.UserDataBuff_index);
                  memset(UserDataBuff, 0x00, MAX_BUFFER_GLOBAL);//Flush the buffer
                  Resume_Dequeue();
                  WiFi_Control_Variables.HTTP_Data_available=WIFI_FALSE;
              }

      else if (WiFi_Control_Variables.FILE_Data_available == WIFI_TRUE)
              {
                  ind_wifi_file_data_available((uint8_t *) UserDataBuff);
                  memset(UserDataBuff, 0x00, MAX_BUFFER_GLOBAL);//Flush the buffer
                  Resume_Dequeue();
                  WiFi_Control_Variables.FILE_Data_available = WIFI_FALSE;
              }
      else if(WiFi_Control_Variables.Client_Connected == WIFI_TRUE)
              {
                  ind_socket_server_client_joined();
                  WiFi_Control_Variables.Client_Connected = WIFI_FALSE;
              }

      else if(WiFi_Control_Variables.Client_Disconnected == WIFI_TRUE)
              {
                  ind_socket_server_client_left();
                  WiFi_Control_Variables.Client_Disconnected = WIFI_FALSE;
              }

      //Make callbacks from here to user for pending events

      if(IO_status_flag.WiFi_WIND_State.WiFiHWStarted==WIFI_TRUE)
          {
              if(IO_status_flag.wifi_ready == 2)//Twice reset for User Callback
                  {
                      IO_status_flag.wifi_ready++;
                      ind_wifi_on();//Call this once only...This if for wifi_on (instead of console active
                  }
          }

      if(IO_status_flag.WiFi_WIND_State.WiFiUp == WIFI_TRUE)
          {
              if(wifi_connected == 0)
                  {
                      wifi_connected = 1;
                      ind_wifi_connected();//wifi connected
                  }    
              IO_status_flag.WiFi_WIND_State.WiFiUp = WIFI_FALSE;
          }

      else if(IO_status_flag.WiFi_WIND_State.WiFiStarted_MiniAPMode == WIFI_TRUE)
          {
              ind_wifi_ap_ready();
              IO_status_flag.WiFi_WIND_State.WiFiStarted_MiniAPMode = WIFI_FALSE;
          }

      else if(IO_status_flag.WiFi_WIND_State.WiFiAPClientJoined == WIFI_TRUE)
          {
                ind_wifi_ap_client_joined(WiFi_Counter_Variables.client_MAC_address);
                IO_status_flag.WiFi_WIND_State.WiFiAPClientJoined = WIFI_FALSE;
          }

      else if(IO_status_flag.WiFi_WIND_State.WiFiAPClientLeft == WIFI_TRUE)
          {
                ind_wifi_ap_client_left(WiFi_Counter_Variables.client_MAC_address);
                IO_status_flag.WiFi_WIND_State.WiFiAPClientLeft = WIFI_FALSE;
          }

      else if(WiFi_Control_Variables.Deep_Sleep_Callback == WIFI_TRUE)
          {
                ind_wifi_resuming();
                WiFi_Control_Variables.Deep_Sleep_Callback = WIFI_FALSE;
          }

      else if(WiFi_Control_Variables.standby_resume_callback == WIFI_TRUE)
          {
                ind_wifi_resuming();
                WiFi_Control_Variables.standby_resume_callback = WIFI_FALSE;
          }

      else if(IO_status_flag.WiFi_WIND_State.WiFiHWFailure==WIFI_TRUE)
          {      
                IO_status_flag.WiFi_WIND_State.WiFiHWFailure=WIFI_FALSE;
                ind_wifi_error(WiFi_HW_FAILURE_ERROR);//call with error number      
          }

      else if(IO_status_flag.WiFi_WIND_State.HardFault==WIFI_TRUE)
          {      
                IO_status_flag.WiFi_WIND_State.HardFault=WIFI_FALSE;
                ind_wifi_error(WiFi_HARD_FAULT_ERROR);//call with error number      
          }

      else if(IO_status_flag.WiFi_WIND_State.StackOverflow==WIFI_TRUE)
          {      
                IO_status_flag.WiFi_WIND_State.StackOverflow=WIFI_FALSE;
                ind_wifi_error(WiFi_STACK_OVERFLOW_ERROR);//call with error number      
          }

      else if(IO_status_flag.WiFi_WIND_State.MallocFailed==WIFI_TRUE)
          {      
                IO_status_flag.WiFi_WIND_State.MallocFailed=WIFI_FALSE;
                ind_wifi_error(WiFi_MALLOC_FAILED_ERROR);//call with error number      
          }

      else if(IO_status_flag.WiFi_WIND_State.InitFailure==WIFI_TRUE)
          {      
                IO_status_flag.WiFi_WIND_State.InitFailure=WIFI_FALSE;
                ind_wifi_error(WiFi_INIT_ERROR);//call with error number      
          }

      else if(IO_status_flag.WiFi_WIND_State.StartFailed==WIFI_TRUE)
          {      
                IO_status_flag.WiFi_WIND_State.StartFailed=WIFI_FALSE;
                ind_wifi_error(WiFi_START_FAILED_ERROR);//call with error number      
          }
      
      else if(IO_status_flag.WiFi_WIND_State.WiFiException==WIFI_TRUE)
          {      
                IO_status_flag.WiFi_WIND_State.WiFiException=WIFI_FALSE;
                ind_wifi_error(WiFi_EXCEPTION_ERROR);//call with error number      
          }

      else if(IO_status_flag.WiFi_WIND_State.PS_Mode_Failure==WIFI_TRUE)
          {      
                IO_status_flag.WiFi_WIND_State.PS_Mode_Failure=WIFI_FALSE;
                ind_wifi_warning(WiFi_POWER_SAVE_WARNING);//call with error number      
          }

      else if(IO_status_flag.WiFi_WIND_State.HeapTooSmall==WIFI_TRUE)
          {      
                IO_status_flag.WiFi_WIND_State.HeapTooSmall=WIFI_FALSE;
                ind_wifi_warning(WiFi_HEAP_TOO_SMALL_WARNING);//call with error number      
          }
      
      else if(IO_status_flag.WiFi_WIND_State.WiFiSignalLOW==WIFI_TRUE)
          {      
                IO_status_flag.WiFi_WIND_State.WiFiSignalLOW=WIFI_FALSE;
                ind_wifi_warning(WiFi_SIGNAL_LOW_WARNING);//call with error number      
          }    
      
      else if(IO_status_flag.WiFi_WIND_State.WiFiDeauthentication == WIFI_TRUE)
          {
                IO_status_flag.WiFi_WIND_State.WiFiDeauthentication = WIFI_FALSE;
                ind_wifi_connection_error(WiFi_DE_AUTH);
          }
      
      else if(IO_status_flag.WiFi_WIND_State.WiFiDisAssociation == WIFI_TRUE)
          {
                IO_status_flag.WiFi_WIND_State.WiFiDisAssociation = WIFI_FALSE;
                ind_wifi_connection_error(WiFi_DISASSOCIATION);
          }
      
      else if(IO_status_flag.WiFi_WIND_State.WiFiJoinFailed == WIFI_TRUE)
          {
                IO_status_flag.WiFi_WIND_State.WiFiJoinFailed = WIFI_FALSE;
                ind_wifi_connection_error(WiFi_JOIN_FAILED);
          }
      
      else if(IO_status_flag.WiFi_WIND_State.WiFiScanBlewUp == WIFI_TRUE)
          {
                IO_status_flag.WiFi_WIND_State.WiFiScanBlewUp = WIFI_FALSE;
                ind_wifi_connection_error(WiFi_SCAN_BLEWUP);  //@TBD to check if user made call, so not call callback if true
          }
      
      else if(IO_status_flag.WiFi_WIND_State.WiFiScanFailed == WIFI_TRUE)
          {
                IO_status_flag.WiFi_WIND_State.WiFiScanFailed = WIFI_FALSE;
                ind_wifi_connection_error(WiFi_SCAN_FAILED);  //@TBD to check if user made call, so not call callback if true
          }
      
      else if(IO_status_flag.WiFi_WIND_State.WiFiUnHandledInd == WIFI_TRUE)
          {
                IO_status_flag.WiFi_WIND_State.WiFiUnHandledInd = WIFI_FALSE;
                ind_wifi_packet_lost(WiFi_UNHANDLED_IND_ERROR);  //@TBD to check if user made call, so not call callback if true
          }
      
      else if(IO_status_flag.WiFi_WIND_State.WiFiRXMgmt == WIFI_TRUE)
          {
                IO_status_flag.WiFi_WIND_State.WiFiRXMgmt = WIFI_FALSE;
                ind_wifi_packet_lost(WiFi_RX_MGMT);  //@TBD to check if user made call, so not call callback if true
          }
      
      else if(IO_status_flag.WiFi_WIND_State.WiFiRXData == WIFI_TRUE)
          {
                IO_status_flag.WiFi_WIND_State.WiFiRXData = WIFI_FALSE;
                ind_wifi_packet_lost(WiFi_RX_DATA);  //@TBD to check if user made call, so not call callback if true
          }  
      
      else if(IO_status_flag.WiFi_WIND_State.WiFiRxUnk == WIFI_TRUE)
          {
                IO_status_flag.WiFi_WIND_State.WiFiRxUnk = WIFI_FALSE;
                ind_wifi_packet_lost(WiFi_RX_UNK);  //@TBD to check if user made call, so not call callback if true
          }  
      
      else if(IO_status_flag.WiFi_WIND_State.WiFiSockdDataLost == WIFI_TRUE)
          {
                IO_status_flag.WiFi_WIND_State.WiFiSockdDataLost = WIFI_FALSE;
                ind_wifi_socket_server_data_lost();  //@TBD to check if user made call, so not call callback if true
          }
}

/**
* @brief  Start_Timer
*         Start Timer 
* @param  None
* @retval None
*/
void Start_Timer()
{
  IO_status_flag.tickcount = WIFI_FALSE;
  IO_status_flag.Timer_Running = WIFI_TRUE;
}

/**
* @brief  Stop_Timer
*         Stop Timer request
* @param  None
* @retval None
*/
void Stop_Timer()
{  
  IO_status_flag.tickcount      = WIFI_FALSE;  
  IO_status_flag.Timer_Running  = WIFI_FALSE;    
  IO_status_flag.UartReady      = SET;
}

/**
* @brief  Stop_Dequeue
*         Stop dequeuing data from the ring buffer
* @param  None
* @retval None
*/
void Stop_Dequeue()
{
  IO_status_flag.enable_dequeue = WIFI_FALSE;
}

/**
* @brief  Resume_Dequeue
*         Resume dequeuing data from the ring buffer
* @param  None
* @retval None
*/
void Resume_Dequeue()
{
  IO_status_flag.enable_dequeue = WIFI_TRUE;
}

/**
* @brief  Wifi_SysTick_Isr
*         Function called every SysTick to process buffer
* @param  None
* @retval None
*/
void Wifi_SysTick_Isr()
{
    //Check if Data is Paused
    if((IO_status_flag.Timer_Running) && (IO_status_flag.enable_dequeue==WIFI_TRUE) /*&& ((tickcount++) >= PROCESS_WIFI_TIMER)*/)
      {
          Process_WiFi();
      }

    if(WiFi_Control_Variables.resume_receive_data == WIFI_TRUE)
      {
          if(is_half_empty(&wifi_instances.big_buff))
            {
                WiFi_Control_Variables.resume_receive_data = WIFI_FALSE;
                Receive_Data();
            }
      }

    if(WiFi_Control_Variables.Standby_Timer_Running) // module is in sleep and after expiry RX will be conf as EXTI
      {
          if((WiFi_Counter_Variables.standby_time++) >= EXTI_CONF_TIMER)
            {
                WiFi_Control_Variables.Standby_Timer_Running=WIFI_FALSE;
                WiFi_Counter_Variables.standby_time = 0;
            }
      }
    if(WiFi_Control_Variables.enable_timeout_timer)     // module will timeout when no response from server received
      {
          WiFi_Counter_Variables.timeout_tick++;
          //wait for 20 seconds before timeout
          if(WiFi_Counter_Variables.timeout_tick > 20000)       //wait for 20s before timeout
            {
                #if DEBUG_PRINT
                  printf("\r\n Timeout! No response received.\r\n");
                #endif
                WiFi_Counter_Variables.timeout_tick         = 0;
                WiFi_Control_Variables.enable_timeout_timer = WIFI_FALSE;
                WiFi_Counter_Variables.AT_RESPONSE          = WiFi_AT_CMD_RESP_ERROR; // Timeout if no response received.
                IO_status_flag.AT_Response_Received = WIFI_TRUE;
                 //re-enable event Q after 200ms
                WiFi_Control_Variables.stop_event_dequeue = WIFI_FALSE;
            }
      }

    /*A Resume WIND:70 has come and triggered this
    So checking here if after that resume we fall back to sleep (another WIND69) within SLEEP_RESUME_PREVENT time.
    If yes, we assume it is a false resume and hence do nothing and go back to sleep
    If no WIND69 (going into sleep) has come, we can assume the resume was genuine and then enable the callback
    */
    if((WiFi_Control_Variables.Deep_Sleep_Timer) && (WiFi_Counter_Variables.sleep_count++) >= SLEEP_RESUME_PREVENT)
      {
          if(WiFi_Control_Variables.Deep_Sleep_Enabled == WIFI_TRUE)//which means we have received another WIND69 in the 2 seconds
            {
                //do nothing, go back to sleep
                WiFi_Control_Variables.Deep_Sleep_Enabled = WIFI_TRUE;
                WiFi_Control_Variables.Deep_Sleep_Callback = WIFI_FALSE;
            }
          else if (WiFi_Control_Variables.Deep_Sleep_Enabled == WIFI_FALSE) //which means we have not received any WIND69 during the last 2 seconds
            {
                //enable the user callback as it is a genuine WIND70
                WiFi_Control_Variables.Deep_Sleep_Callback = WIFI_TRUE;
            }
          Stop_DeepSleep_Timer();
      }
}

/**
* @brief  WiFi_HAL_UART_TxCpltCallback
*         Tx Transfer completed callback
* @param  UsartHandle: UART handle 
* @retval None
*/
void WiFi_HAL_UART_TxCpltCallback(UART_HandleTypeDef *UartHandleArg)
{
#ifdef WIFI_USE_VCOM
  if (UartHandleArg==&UartMsgHandle)
    console_echo_ready = SET;
#else
  /* Set transmission flag: transfer complete */
  IO_status_flag.TxUartReady = SET; 
#endif
}

/**
* @brief  WiFi_HAL_UART_RxCpltCallback
*         Rx Transfer completed callback
* @param  UsartHandle: UART handle 
* @retval None
*/
void WiFi_HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartHandleArg)
{
#ifdef WIFI_USE_VCOM
  if (UartHandleArg==&UartWiFiHandle)
#endif  
  {
    #ifndef WIFI_USE_VCOM
      WiFi_Control_Variables.Uartx_Rx_Processing = WIFI_FALSE;
      Stop_Timer();
      __disable_irq();
      push_buffer_queue(&wifi_instances.big_buff, WiFi_Counter_Variables.uart_byte);
      __enable_irq();
      Start_Timer();
    #else
        console_push_ready = SET;
    #endif

    #ifndef WIFI_USE_VCOM  
      if(is_half_full(&wifi_instances.big_buff))
        {
          WiFi_Control_Variables.resume_receive_data = WIFI_TRUE;
          HAL_GPIO_WritePin(WiFi_USART_RTS_GPIO_PORT, WiFi_USART_RTS_PIN, GPIO_PIN_SET);//De-assert RTS
        } 
      else
        {
          if(WiFi_Control_Variables.AT_Cmd_Processing == WIFI_FALSE)
            {
              //call Rx only if TX is not under processing (AT command)
              wifi_instances.receive_status = HAL_UART_Receive_IT(&UartWiFiHandle, (uint8_t *)WiFi_Counter_Variables.uart_byte, 1);
              if(wifi_instances.receive_status!=HAL_OK)
                {
                  #if DEBUG_PRINT 
                  printf(" WiFi_HAL_UART_RxCpltCallback HAL_UARTx_Receive_IT Error");
                  #endif
                }
              else 
                {
                  WiFi_Control_Variables.Uartx_Rx_Processing = WIFI_TRUE;
                }
            }
        }
    #endif
  }
#ifdef WIFI_USE_VCOM
  else
    {
      console_send_char[0] = console_input_char[0];
      console_input();
      console_send_ready = SET;
      HAL_UART_Transmit_IT(&UartWiFiHandle, (uint8_t*)console_send_char, 1);
    }
#endif
}  
  
/**
* @brief  USART_Receive_AT_Resp
*         Receive and check AT cmd response
* @param  None
* @retval WiFi_Status_t : Response of AT cmd  
*/

WiFi_Status_t USART_Receive_AT_Resp( )
{
  while(IO_status_flag.AT_Response_Received != WIFI_TRUE) {
		__NOP(); //nothing to do
	}
  IO_status_flag.AT_Response_Received = WIFI_FALSE;
  return WiFi_Counter_Variables.AT_RESPONSE;
}

/**
  * @brief  UART error callbacks
  * @param  UsartHandle: UART handle
  * @note   This example shows a simple way to report transfer error, and you can
  *         add your own implementation.
  * @retval None
  */
void WiFi_HAL_UART_ErrorCallback(UART_HandleTypeDef *UartHandle)
{
    Error_Handler();
}

/**
* @brief  Process_WiFi
*         Pop a byte from the circular buffer and send the byte for processing
*         This function should be called from main or should be run with a periodic timer
* @param  None
* @retval None
*/
void Process_WiFi(void)
{
    __disable_irq();
    WiFi_Counter_Variables.temp = pop_buffer_queue(&wifi_instances.big_buff);   //contents of temp(pop_buffer) will not change till another de-queue is made
    __enable_irq();

    if(WiFi_Counter_Variables.temp!=NULL) 
      {
        Process_Buffer(WiFi_Counter_Variables.temp);
      }

   if(WiFi_Control_Variables.event_deQ_x_wind64)//if de-Q is stopped due to WIND64 wait
     {
       WiFi_Counter_Variables.wind64_DQ_wait++;//1ms for each count
       if(WiFi_Counter_Variables.wind64_DQ_wait>50)//wait for 50ms for example
         {
           WiFi_Counter_Variables.wind64_DQ_wait=0;
           WiFi_Control_Variables.event_deQ_x_wind64 = WIFI_FALSE;
           //re-enable event Q after 50ms
           WiFi_Control_Variables.stop_event_dequeue = WIFI_FALSE;
         }
     }
}

/**
* @brief  Process_Buffer
*         Process and construct a Wind Line buffer
* @param  ptr: pointer to one single byte
* @retval None
*/

void Process_Buffer(uint8_t * ptr)
{ 
  static uint32_t Fillptr=0;
  static uint8_t index, chan_value;
  unsigned char rxdata = 0;
  int rssi_value = 0;
  char SocketId_No[2];
  char databytes_No[4];
  char * pStr;
  static uint8_t HTTP_Runway_Buff[6];  //Used to store the last 6 bytes in between User Callbacks during HTTP tx
  static uint8_t process_buffer[MAX_BUFFER_GLOBAL];

  rxdata =  *(ptr+0);  
  //printf(&rxdata);    //check prints for debug...to be removed or kept in DEBUG statement
  if(WiFi_Control_Variables.enable_receive_data_chunk == WIFI_FALSE)
    process_buffer[Fillptr++] = rxdata;
   reset_event(&wifi_instances.wifi_event);

    if((process_buffer[Fillptr-2]==0xD) && (process_buffer[Fillptr-1]==0xA) 
       && !WiFi_Control_Variables.enable_receive_http_response && !IO_status_flag.sock_read_ongoing)
      {
          if((strstr((const char *)process_buffer,"WIND:")) != NULL)
          {
              //end of msg received. Will not receive any other msg till we process this.
              Stop_Timer();
              #if defined (USE_STM32L0XX_NUCLEO) || (USE_STM32F4XX_NUCLEO) || (USE_STM32L4XX_NUCLEO)
              __disable_irq();
              #endif
              Process_Wind_Indication(&process_buffer[0]);
              #if defined (USE_STM32L0XX_NUCLEO) || (USE_STM32F4XX_NUCLEO) || (USE_STM32L4XX_NUCLEO)
              __enable_irq();
              #endif
              Start_Timer();

              if(!WiFi_Control_Variables.prevent_push_WIFI_event)
                {
                    __disable_irq();
                    push_eventbuffer_queue(&wifi_instances.event_buff, wifi_instances.wifi_event);
                     __enable_irq();
                    reset_event(&wifi_instances.wifi_event);
                }
                  
              else if(!WiFi_Control_Variables.queue_wifi_wind_message)  //if we do not want to queue a WIND: message
                {
                    reset_event(&wifi_instances.wifi_event);
                    WiFi_Control_Variables.prevent_push_WIFI_event = WIFI_FALSE;
                    WiFi_Control_Variables.queue_wifi_wind_message = WIFI_TRUE;
                }
              if (!WiFi_Control_Variables.do_not_reset_push_WIFI_event) 
                WiFi_Control_Variables.prevent_push_WIFI_event = WIFI_FALSE;

              Fillptr=0;
              if(WiFi_Control_Variables.enable_sock_read)
                WiFi_Control_Variables.Q_Contains_Message = WIFI_FALSE;
              else
                return;
          }

          else if((strstr((const char *)process_buffer,"\r\nOK\r\n")) != NULL)
          {
              /*Now Check to which AT Cmd response this OK belongs to so that correct parsing can be done*/

              // SOCKON ID (Open a client socket)
              if(((pStr=(strstr((const char *)process_buffer,"ID: "))) != NULL))      
                {
                    SocketId_No[0]    = *(pStr + 4) ;
                    SocketId_No[1]    = *(pStr + 5) ;
                    wifi_instances.wifi_event.socket_id = (((SocketId_No[0] - '0') * 10 ) + (SocketId_No[1] - '0'));
                    wifi_instances.wifi_event.event     =  WIFI_SOCK_ID_EVENT;
                    __disable_irq();
                    push_eventbuffer_queue(&wifi_instances.event_buff, wifi_instances.wifi_event);
                     __enable_irq();
                    reset_event(&wifi_instances.wifi_event);
                 }

                // DATALEN from SOCKQ
               else if((pStr=(strstr((const char *)process_buffer,"DATALEN: "))) != NULL)
                 {
                    //Find the DataLength and do a socket read
                    databytes_No[0] = *(pStr + 9);
                    databytes_No[1] = *(pStr + 10);
                    databytes_No[2] = *(pStr + 11);
                    databytes_No[3] = *(pStr + 12);

                    if( databytes_No[1] == '\r')
                      {
                        WiFi_Counter_Variables.Socket_Data_Length = databytes_No[0] - '0'; 
                      }
                    else if( databytes_No[2] == '\r')
                      {
                        WiFi_Counter_Variables.Socket_Data_Length = (((databytes_No[0] - '0') * 10 ) + (databytes_No[1] - '0'));
                      }
                    else if( databytes_No[3] == '\r')
                      {
                        WiFi_Counter_Variables.Socket_Data_Length = (((databytes_No[0] - '0') * 100 ) + ((databytes_No[1] - '0') * 10 ) + (databytes_No[2] - '0'));
                      }
                    else //it's a 4-digit number
                      {
                        WiFi_Counter_Variables.Socket_Data_Length  = ((databytes_No[0] - '0') * 1000 ) + ((databytes_No[1] - '0') * 100 ) + ((databytes_No[2] - '0') * 10) + (databytes_No[3] - '0');
                      }
                    if(WiFi_Counter_Variables.Socket_Data_Length != 0)
                      {
                        WiFi_Control_Variables.start_sock_read = WIFI_TRUE;
                      }
                    else if(WiFi_Counter_Variables.Socket_Data_Length == 0)  //no data remaining to be read
                      {
                        if(WiFi_Counter_Variables.socket_close_pending[WiFi_Counter_Variables.sockon_query_id])
                          {
                            // Q socket_close event for that socket for which sock_close command could not be processed earlier due to ERROR: pending data.
                             if(open_sockets[WiFi_Counter_Variables.sockon_query_id])
                              {
                                Queue_Client_Close_Event(WiFi_Counter_Variables.sockon_query_id);
                              }
                            WiFi_Counter_Variables.socket_close_pending[WiFi_Counter_Variables.sockon_query_id] = WIFI_FALSE;
                          }
                        WiFi_Control_Variables.stop_event_dequeue = WIFI_FALSE;  //continue popping events if nothing to read
                      }
                 }

               else if((pStr = (char *)(strstr((const char *)process_buffer," = "))) != NULL)
                 {
                    // AT command GCFG
                    wifi_instances.wifi_event.event = WIFI_GCFG_EVENT;
                    //we need to copy only the value in get_cfg_value variable
                    memcpy(WiFi_Counter_Variables.get_cfg_value, pStr+3, (strlen(pStr)-11));
                    __disable_irq();
                    push_eventbuffer_queue(&wifi_instances.event_buff, wifi_instances.wifi_event);
                     __enable_irq();
                    reset_event(&wifi_instances.wifi_event);
                 }

               else
                 {
                    //This is a standalone OK
                    /*Cases possible
                    - TLSCERT,TLSCERT2, TLSDOMAIN, SETTIME
                    - S.SOCKW, SOCKR, S.SOCKC, S.SOCKD (open a server socket)
                    - File Operations
                    - S.GPIOC and S.GPIOW
                    */
                    //Push a simple OK Event, if this is an OK event required to be pushed to Q
                    if(IO_status_flag.prevent_push_OK_event)
                      {
                          //This OK is not to be handled, hence the pop action on OK completion to be done here                              
                          if(IO_status_flag.client_socket_close_ongoing) //OK received is of the sock close command
                            {
                                if(WiFi_Counter_Variables.no_of_open_client_sockets > 0)
                                  WiFi_Counter_Variables.no_of_open_client_sockets--;
                                IO_status_flag.prevent_push_OK_event                         = WIFI_FALSE;
                                open_sockets[WiFi_Counter_Variables.remote_socket_closed_id] = WIFI_FALSE;
                                //socket ID for which OK is received.
                                WiFi_Counter_Variables.closed_socket_id                      = WiFi_Counter_Variables.remote_socket_closed_id;
                                IO_status_flag.client_socket_close_ongoing                   = WIFI_FALSE;

                                //User Callback not required in case if sock_close is called by User (Only in WIND:58)
                                if(WiFi_Control_Variables.enable_SockON_Server_Closed_Callback)
                                  {
                                    // User callback after successful socket Close
                                    WiFi_Control_Variables.SockON_Server_Closed_Callback         = WIFI_TRUE;
                                  }
                                else
                                    WiFi_Control_Variables.enable_SockON_Server_Closed_Callback  = WIFI_TRUE;
                                WiFi_Control_Variables.stop_event_dequeue                        = WIFI_FALSE;
                            }
                      }

                    else
                      {
                          wifi_instances.wifi_event.ok_eval = WIFI_TRUE;
                          wifi_instances.wifi_event.event = WIFI_OK_EVENT;
                          __disable_irq();
                          push_eventbuffer_queue(&wifi_instances.event_buff, wifi_instances.wifi_event);
                           __enable_irq();
                          reset_event(&wifi_instances.wifi_event);
                      }
                    IO_status_flag.prevent_push_OK_event = WIFI_FALSE;
                 }
              Fillptr=0;
              memset(process_buffer, 0x00, MAX_BUFFER_GLOBAL);

              if(WiFi_Control_Variables.enable_sock_read)
                WiFi_Control_Variables.Q_Contains_Message = WIFI_FALSE;
              else
                return;
          }

         else if((((strstr((const char *)process_buffer,"ERROR"))) != NULL))
          {
            char tmpBuff[32];
            char tmpBuff2[64];
            sprintf(tmpBuff,"\r\n%s: Pending data\r\n","ERROR");
            sprintf(tmpBuff2,"\r\n%s: Data mode not available\r\n","ERROR");

              //This is an ERROR
              //There can be only ONE outstanding AT command and hence this ERROR belongs to that
              //HTTP -> ERROR: host not found
              //@TBD: Check all Errors Possible here???
              if((strstr((const char *)process_buffer,tmpBuff)) != NULL) //if Error after sock close command and not 'OK'
                {
                    printf("\r\nERR: Socket could not be closed..PENDING DATA\r\n");
                    //prevent the OK received after socket close command to be Q'ed
                    IO_status_flag.prevent_push_OK_event        = WIFI_FALSE;     
                    IO_status_flag.client_socket_close_ongoing  = WIFI_FALSE;
                    
                    //when error while user trying to close a socket, so now callback required whenever the socket gets closed.
                    if(!WiFi_Control_Variables.enable_SockON_Server_Closed_Callback)
                      {
                        //enable the SockON_Server_Closed_Callback
                        WiFi_Control_Variables.enable_SockON_Server_Closed_Callback = WIFI_TRUE;
                      }
                    //close the socket for which ERROR is received afterwards, after reading the data on that socket
                    WiFi_Counter_Variables.socket_close_pending[WiFi_Counter_Variables.remote_socket_closed_id] = WIFI_TRUE;
                    WiFi_Control_Variables.stop_event_dequeue = WIFI_FALSE;
                }
               else if((strstr((const char *)process_buffer,tmpBuff2)) != NULL)
                {
                    WiFi_Control_Variables.data_pending_sockD = WIFI_FALSE;
                    WiFi_Control_Variables.stop_event_dequeue = WIFI_FALSE;
                    WiFi_Control_Variables.enable_sock_read   = WIFI_FALSE;
                    WiFi_Control_Variables.enable_receive_data_chunk = WIFI_FALSE;
                    WiFi_Counter_Variables.chunk_size         = 0;
                    WiFi_Counter_Variables.message_size       = 0;
                }
              else
                {
                    wifi_instances.wifi_event.event = WIFI_ERROR_EVENT;
                    __disable_irq();
                    push_eventbuffer_queue(&wifi_instances.event_buff, wifi_instances.wifi_event);
                     __enable_irq();
                    reset_event(&wifi_instances.wifi_event);
                    if(WiFi_Control_Variables.stop_event_dequeue)
                      /*ERROR:Illegal Socket ID*/
                      WiFi_Control_Variables.stop_event_dequeue = WIFI_FALSE;//continue popping events if nothing to read
                    if(WiFi_Control_Variables.enable_timeout_timer)
                      {
                        WiFi_Control_Variables.enable_timeout_timer = WIFI_FALSE;
                        WiFi_Counter_Variables.timeout_tick = 0;
                      }
                }
              Fillptr=0;
              memset(process_buffer, 0x00, MAX_BUFFER_GLOBAL);

              if(WiFi_Control_Variables.enable_sock_read)
                WiFi_Control_Variables.Q_Contains_Message = WIFI_FALSE;
              else
                return;
          }

         else if((((strstr((const char *)process_buffer,"GPIO "))) != NULL)) 
          {
              // Receive GPIO Read
              pStr = (char *) strstr((const char *)process_buffer,"= 0,");                                  
              if(pStr != NULL)
                {
                    WiFi_Counter_Variables.gpio_value = 0;
                } 
              else 
                {
                  WiFi_Counter_Variables.gpio_value = 1;   
                }
              pStr = (char *) strstr((const char *)process_buffer,"out");                                  
              if(pStr != NULL)
                {
                    WiFi_Counter_Variables.gpio_dir= 0;    //out
                }
              else
                {
                    WiFi_Counter_Variables.gpio_dir= 1;    //in
                }
              //Push GPIO Read Event on Event_Queue
              wifi_instances.wifi_event.event = WIFI_GPIO_EVENT;
              __disable_irq();
              push_eventbuffer_queue(&wifi_instances.event_buff, wifi_instances.wifi_event);
               __enable_irq();
              reset_event(&wifi_instances.wifi_event);
              Fillptr=0;
              memset(process_buffer, 0x00, MAX_BUFFER_GLOBAL);

              if(WiFi_Control_Variables.enable_sock_read)
                WiFi_Control_Variables.Q_Contains_Message = WIFI_FALSE;
              else
                return;
          }

         else
           {
             //the data of wind:55 has \r\n...\r\n or before the actual data arrival there is a message having \r\n..\r\n and not matching any of the above conditions.
             if(WiFi_Control_Variables.enable_sock_read && WiFi_Control_Variables.Q_Contains_Message && !WiFi_Control_Variables.message_pending)
                {
                   /* data is D Q'ed in pop_buffer and we return from here(process_buffer()) without processing this data. Next time we enter in this procedure the data will be overe written in pop_buffer
                        To avoid this, rewinding is necessary. */
                    if(WiFi_Counter_Variables.pop_buffer_size)
                      {
                        __disable_irq();
                         rewind_buffer_queue(&wifi_instances.big_buff,WiFi_Counter_Variables.pop_buffer_size); //in this the case of rewinding past the buffer->end can happen
                         __enable_irq();
                         memset(ptr, 0x00, WiFi_Counter_Variables.pop_buffer_size);
                      }
                    //used for bypassing enable_receive_data_chunk part for the first time \r\n..\r\n is received in case of socket data
                    WiFi_Control_Variables.enable_sock_data   = WIFI_TRUE;
                    WiFi_Control_Variables.Q_Contains_Message = WIFI_FALSE;
                    WiFi_Counter_Variables.pop_queue_length   = Fillptr;
                }

              //if in data mode, reset on \r\n
              if(!IO_status_flag.sock_read_ongoing && IO_status_flag.data_mode)
                {
                    Fillptr = 0;
                    memset(process_buffer, 0x00, MAX_BUFFER_GLOBAL);
                }
           }
      }

    else if (WiFi_Control_Variables.http_req_pending)   //HTTP Response Check
      {
        char tmp_Buff[16];
        sprintf(tmp_Buff,"200 %s","OK\r");
          
           if((strstr((const char *)&process_buffer[0],tmp_Buff)) != NULL || (strstr((const char *)&process_buffer[0],"200 \r")) != NULL 
                  || (strstr((const char *)&process_buffer[0],"400 Bad Request\r")) != NULL || (strstr((const char *)&process_buffer[0],"400 \r")) != NULL 
                  || (strstr((const char *)&process_buffer[0],"401 Unauthorized\r")) != NULL || (strstr((const char *)&process_buffer[0],"403 Forbidden\r")) != NULL 
                  || (strstr((const char *)&process_buffer[0],"404 Not Found\r")) != NULL || (strstr((const char *)&process_buffer[0],"408 Request Timeout\r")) != NULL  
                  || (strstr((const char *)&process_buffer[0],"500 Internal Server Error\r")) != NULL  || (strstr((const char *)&process_buffer[0],"502 Bad Gateway\r")) != NULL 
                  || (strstr((const char *)&process_buffer[0],"504 Gateway Timeout\r")) != NULL)
            {
                WiFi_Control_Variables.enable_receive_http_response = WIFI_TRUE;
                WiFi_Control_Variables.enable_receive_data_chunk    = WIFI_TRUE;
                WiFi_Control_Variables.http_req_pending             = WIFI_FALSE;
                WiFi_Counter_Variables.last_process_buffer_index = 5;
                WiFi_Control_Variables.enable_timeout_timer      = WIFI_FALSE; //stop the timeout timer as response from server received.
                WiFi_Counter_Variables.timeout_tick              = 0;
                return;
            }
      }

    else if ((process_buffer[Fillptr-1]==0x09) && (process_buffer[Fillptr-2]==':') && (process_buffer[Fillptr-3]=='1'))//<ht> Horizontal Tab for Scan Result?
      {
          WiFi_Control_Variables.enable_receive_wifi_scan_response = WIFI_TRUE;
      }
    else if(Fillptr >= MAX_BUFFER_GLOBAL-1)
      {
        #if DEBUG_PRINT
        printf("\rJust Looping with unhandled data!\r\n");
        #endif          
        Fillptr=0;          
        memset(process_buffer, 0x00, MAX_BUFFER_GLOBAL); 
      }
    
        //Check Process Buffer for any pending message
        if(WiFi_Control_Variables.enable_receive_data_chunk && !WiFi_Control_Variables.enable_sock_data)
          {            
            WiFi_Counter_Variables.pop_queue_length = WiFi_Counter_Variables.pop_buffer_size;
            char tmpBuff[16];
            sprintf(tmpBuff,"%s: ","ERROR");
            if(Fillptr + WiFi_Counter_Variables.pop_queue_length > MAX_BUFFER_GLOBAL-1)
            {
               uint32_t length = (Fillptr + WiFi_Counter_Variables.pop_queue_length) - (MAX_BUFFER_GLOBAL - 1);
              __disable_irq();
               rewind_buffer_queue(&wifi_instances.big_buff,length);
               __enable_irq();
               memset(ptr+((MAX_BUFFER_GLOBAL-1) - Fillptr), 0x00, length);
               WiFi_Counter_Variables.pop_queue_length = (MAX_BUFFER_GLOBAL-1) - Fillptr;
            }
            memcpy(process_buffer+Fillptr,(char const *)pop_buffer, WiFi_Counter_Variables.pop_queue_length);
            Fillptr = Fillptr + WiFi_Counter_Variables.pop_queue_length;
            
             // do nothing in this case as we do not know what data is coming next.
            if(Fillptr == 1 && process_buffer[0]=='\r')
              return;

            if((strstr((const char *)process_buffer,tmpBuff)) != NULL)
            {
                 WiFi_Control_Variables.Q_Contains_Message = WIFI_FALSE;
                 WiFi_Control_Variables.message_pending = WIFI_FALSE;
            }
            else if(!IO_status_flag.sock_read_ongoing && !WiFi_Control_Variables.enable_receive_http_response) 
            {
                if(process_buffer[0]!='\0') 
                {
                  // in case of space " " SockON_Data_Length = 2 as only \r\n, so No Q_contains_message.
                  if(((process_buffer[0]==0xD) && (process_buffer[1]==0xA)) && WiFi_Counter_Variables.SockON_Data_Length != 2)
                  {
                    WiFi_Control_Variables.message_pending = WIFI_TRUE;
                    if((pStr = (strstr((const char *)process_buffer+2,"\r\n"))) != NULL) 
                    {
                          // process buffer has complete message
                          int wind_length = ((uint8_t *)pStr - (uint8_t *)process_buffer)+2;
                          if(strstr((const char *)process_buffer+2,"DATALEN:")) 
                          {
                                pStr = strstr((const char *)process_buffer + wind_length,"\r\nOK\r\n"); //find OK, as DATALEN has to be terminated by OK
                                if(pStr!=NULL)
                                {
                                    wind_length = ((uint8_t *)pStr-(uint8_t *)process_buffer)+6;
                                }
                          }

                          if(Fillptr - wind_length)  //rewind only if extra bytes present in process_buffer
                          {
                                __disable_irq();
                              rewind_buffer_queue(&wifi_instances.big_buff, Fillptr - wind_length);             
                              __enable_irq();
                              memset(process_buffer + wind_length,0x00,Fillptr - wind_length);
                              Fillptr = wind_length;
                          }
                          WiFi_Control_Variables.message_pending = WIFI_FALSE;
                      }
                      WiFi_Control_Variables.Q_Contains_Message = WIFI_TRUE;
                    
                      //if in /r/n.../r/n the data between a pair of /r/n is more than 511
                      if(Fillptr == 511 && WiFi_Control_Variables.message_pending)
                        {
                            WiFi_Control_Variables.message_pending    = WIFI_FALSE;
                            WiFi_Control_Variables.Q_Contains_Message = WIFI_FALSE;
                            WiFi_Counter_Variables.pop_queue_length   = 511;
                        }
                    }
                }
            }
          }

        /*********************************************************************************************
         *                                                                                           *
         *                             Socket Read Is Enabled.                                       *
         *                                                                                           *
         ********************************************************************************************/
        
        
        if(!WiFi_Control_Variables.Q_Contains_Message && WiFi_Control_Variables.enable_sock_read)     /*read is enabled*/
        {
            IO_status_flag.sock_read_ongoing = WIFI_TRUE;
            WiFi_Counter_Variables.sock_total_count = WiFi_Counter_Variables.sock_total_count + WiFi_Counter_Variables.pop_queue_length;
            char tmpBuff[16];
            char tmpBuff2[32];
            char tmpBuff3[32];
            char tmpBuff4[32];
            sprintf(tmpBuff,"%s: ","ERROR");
            sprintf(tmpBuff2,"\r\n%s: Too many sockets\r\n","ERROR");
            sprintf(tmpBuff3,"\r\n%s: Pending data\r\n","ERROR");
            sprintf(tmpBuff4,"\r\n%s: Socket error\r\n","ERROR");

            if(WiFi_Control_Variables.enable_sock_data)
              WiFi_Control_Variables.enable_sock_data = WIFI_FALSE;
            
            /* Check for "ERROR: Not enough data in buffer " */
            pStr = (char *) strstr((const char *)&process_buffer + WiFi_Counter_Variables.last_process_buffer_index - 5,tmpBuff);
            if (pStr != NULL)
              {
                  if((process_buffer[Fillptr-2]==0xD) && (process_buffer[Fillptr-1]==0xA)) //check if end of message received
                    {
                      if((pStr = (char *)strstr((const char *)&process_buffer,tmpBuff2)) !=NULL)
                        {
                           #if DEBUG_PRINT
                             printf("\r\nERROR: TOO MANY SOCKETS \r\n");
                           #endif

                            if(*(pStr+27)!='\0')
                              {
                                  int len = (uint8_t *)pStr - (uint8_t *)process_buffer;
                                  int extra_bytes = Fillptr - (len+27);
                                  __disable_irq();
                                  rewind_buffer_queue(&wifi_instances.big_buff, extra_bytes);
                                  __enable_irq();
                              }
                            Fillptr=0;
                            WiFi_Counter_Variables.sock_total_count = 0;
                            WiFi_Counter_Variables.last_process_buffer_index = 5;
                            memset(process_buffer, 0x00, MAX_BUFFER_GLOBAL);
                            IO_status_flag.AT_Response_Received = WIFI_TRUE;
                            WiFi_Counter_Variables.AT_RESPONSE = WiFi_AT_CMD_RESP_ERROR;
                            return;
                        }
                      else if((pStr = (char *)strstr((const char *)&process_buffer,tmpBuff3)) !=NULL)
                        {
                           #if DEBUG_PRINT
                             printf("\r\nERROR: PENDING DATA \r\n");
                           #endif

                            if(*(pStr+23)!='\0')
                              {
                                  int len = (uint8_t *)pStr - (uint8_t *)process_buffer;
                                  int extra_bytes = Fillptr - (len+23);
                                  __disable_irq();
                                  rewind_buffer_queue(&wifi_instances.big_buff, extra_bytes);
                                  __enable_irq();
                              }
                            WiFi_Counter_Variables.socket_close_pending[WiFi_Counter_Variables.remote_socket_closed_id] = WIFI_TRUE;
                        }
                      
                      else if((pStr = (char *)strstr((const char *)&process_buffer,tmpBuff4)) !=NULL)
                        {
                            #if DEBUG_PRINT
                             printf("\r\nERROR: Socket error\r\n");
                            #endif

                           if(*(pStr+23)!='\0')
                            {
                                int len = (uint8_t *)pStr - (uint8_t *)process_buffer;
                                int extra_bytes = Fillptr - (len+23);
                                __disable_irq();
                                rewind_buffer_queue(&wifi_instances.big_buff, extra_bytes);
                                __enable_irq();
                            }
                        }

                      #if DEBUG_PRINT
                        printf("\rERROR DURING SOCK READ\r\n");
                      #endif
                      WiFi_Counter_Variables.sock_total_count = 0;
                      WiFi_Counter_Variables.Socket_Data_Length = 0;
                      WiFi_Counter_Variables.SockON_Data_Length = 0;
                      WiFi_Counter_Variables.last_process_buffer_index = 5;
                      WiFi_Control_Variables.enable_sock_read = WIFI_FALSE;
                      WiFi_Control_Variables.stop_event_dequeue = WIFI_FALSE;
                      WiFi_Control_Variables.enable_receive_data_chunk = WIFI_FALSE;
                      IO_status_flag.WIND64_count=0;
                      Fillptr=0;
                      IO_status_flag.sock_read_ongoing = WIFI_FALSE;
                      if(WiFi_Control_Variables.data_pending_sockD)
                        {
                          WiFi_Control_Variables.data_pending_sockD = WIFI_FALSE;
                          WiFi_Counter_Variables.number_of_bytes=0;
                          WiFi_Control_Variables.switch_by_default_to_command_mode=WIFI_TRUE;
                          WiFi_switch_to_command_mode(); //switch by default
                        }

                      memset(process_buffer, 0x00, MAX_BUFFER_GLOBAL);
                      return;
                   }
              }
            
            //In case of "ERROR" we wait till \r\n is received, till that do not update last_process_buffer_index variable
            if(Fillptr > 5 && pStr == NULL) 
              WiFi_Counter_Variables.last_process_buffer_index = Fillptr;

            /*now check if end of msg received*/
            if(WiFi_Counter_Variables.sock_total_count >= WiFi_Counter_Variables.SockON_Data_Length)
              {
                #if DEBUG_PRINT
                 printf("\nReached SockON_Data_Length \r\n");
                #endif

                 //rewind the buffer, if some extra bytes along with required data
                if(WiFi_Counter_Variables.sock_total_count > WiFi_Counter_Variables.SockON_Data_Length)
                  {
                    int databytes = WiFi_Counter_Variables.sock_total_count - WiFi_Counter_Variables.SockON_Data_Length;
                    
                    __disable_irq();
                    rewind_buffer_queue(&wifi_instances.big_buff, databytes);
                    __enable_irq();

                    memset(process_buffer+(Fillptr - databytes), 0x00, databytes);
                    Fillptr = Fillptr - databytes;
                  }
                WiFi_Counter_Variables.chunk_size = Fillptr;
                WiFi_Counter_Variables.message_size = WiFi_Counter_Variables.SockON_Data_Length;
                memcpy(UserDataBuff, process_buffer, Fillptr);
                Fillptr = 0;
                WiFi_Counter_Variables.sock_total_count = 0;
                WiFi_Counter_Variables.Socket_Data_Length = 0;
                WiFi_Counter_Variables.SockON_Data_Length = 0;
                WiFi_Counter_Variables.pop_queue_length = 0;
                WiFi_Counter_Variables.last_process_buffer_index = 5;
                WiFi_Control_Variables.Q_Contains_Message = WIFI_FALSE;
                WiFi_Control_Variables.enable_receive_data_chunk = WIFI_FALSE;

                if(WiFi_Control_Variables.data_pending_sockD)
                  {
                    WiFi_Counter_Variables.number_of_bytes=0;
                    WiFi_Counter_Variables.sockon_id_user = 0;
                    if(IO_status_flag.WIND64_count>0)
                        IO_status_flag.WIND64_count--;                         //decrease the number of pending WIND:64 Events
                    if(IO_status_flag.WIND64_count==0) 
                      {
                        WiFi_Control_Variables.switch_by_default_to_command_mode=WIFI_TRUE;
                        WiFi_switch_to_command_mode();              //switch by default
                      }
                    WiFi_Control_Variables.enable_query = WIFI_FALSE;
                  }
                else
                  {
                    //do we have more data?
                    WiFi_Control_Variables.enable_query = WIFI_TRUE;
                    WiFi_Counter_Variables.sockon_id_user = WiFi_Counter_Variables.sockon_query_id;
                    /*@TODO: Do not need to prevent OK push in case of server socket*/
                    IO_status_flag.prevent_push_OK_event = WIFI_TRUE; //prevent the qeueuing of the OK after this read operation
                  }

                WiFi_Control_Variables.enable_sock_read = WIFI_FALSE;
                IO_status_flag.sock_read_ongoing = WIFI_FALSE;
                Stop_Dequeue();                           //Stop dequeue till user callback returns

                if(WiFi_Control_Variables.data_pending_sockD)
                  {
                    WiFi_Control_Variables.Pending_SockD_Callback = WIFI_TRUE;
                    WiFi_Control_Variables.stop_event_dequeue = WIFI_FALSE;
                    WiFi_Control_Variables.data_pending_sockD = WIFI_FALSE;
                  }
                else
                  {
                    WiFi_Control_Variables.Pending_SockON_Callback = WIFI_TRUE;   //set this to callback to user with User Buffer pointer
                  }
                memset(process_buffer, 0x00, MAX_BUFFER_GLOBAL);
                return;
              }

            //if Process_buffer is completely filled, callback to user with User Buffer pointer
            if(Fillptr >= MAX_BUFFER_GLOBAL-1)
              {
                WiFi_Counter_Variables.last_process_buffer_index = 5;
                WiFi_Counter_Variables.sockon_id_user = WiFi_Counter_Variables.sockon_query_id;
                WiFi_Counter_Variables.message_size = WiFi_Counter_Variables.SockON_Data_Length;
                WiFi_Counter_Variables.chunk_size = MAX_BUFFER_GLOBAL-1;
                memcpy(UserDataBuff, process_buffer, Fillptr);
                Fillptr = 0;
                WiFi_Counter_Variables.last_process_buffer_index = 5;
                Stop_Dequeue();
                memset(process_buffer, 0x00, MAX_BUFFER_GLOBAL);
                if(WiFi_Control_Variables.data_pending_sockD)
                  {
                    WiFi_Control_Variables.Pending_SockD_Callback = WIFI_TRUE; //callback to user with User Buffer pointer, in case of server socket
                  }
                else
                  {
                    WiFi_Control_Variables.Pending_SockON_Callback = WIFI_TRUE; //set this to callback to user with User Buffer pointer
                  }
              }
            return;
        }

         /********************************************************************************************
         *                                                                                           *
         *                             HTTP Response Is Enabled.                                     *
         *                                                                                           *
         ********************************************************************************************/

        if (WiFi_Control_Variables.enable_receive_http_response) // http response enabled
        {
            IO_status_flag.sock_read_ongoing = WIFI_TRUE;
            if((pStr = (strstr((const char *)process_buffer + WiFi_Counter_Variables.last_process_buffer_index - 5,"\r\nOK\r\n"))) != NULL)  
              {
                  #if DEBUG_PRINT
                    printf("\r\nOK\r\n");         //http response completed
                  #endif

                  if(*(pStr+7) != '\0') //extra data apart from the required HTTP Response
                    {
                        int len = (uint8_t *)pStr - (uint8_t *)process_buffer;
                        int extra_bytes = Fillptr - (len+6);
                        __disable_irq();
                        rewind_buffer_queue(&wifi_instances.big_buff, extra_bytes);
                        __enable_irq();
                        
                        memset(process_buffer+len+7, 0x00, extra_bytes);
                        Fillptr = Fillptr - extra_bytes;
                    }
                  
                  WiFi_Counter_Variables.chunk_size = Fillptr;
                  memcpy(UserDataBuff, process_buffer, Fillptr);
                  WiFi_Counter_Variables.UserDataBuff_index = Fillptr;
                  IO_status_flag.AT_Response_Received = WIFI_TRUE;
                  WiFi_Control_Variables.enable_receive_data_chunk = WIFI_FALSE;
                  WiFi_Control_Variables.enable_receive_http_response = WIFI_FALSE;
                  Stop_Dequeue();
                  WiFi_Counter_Variables.AT_RESPONSE = WiFi_MODULE_SUCCESS;
                  IO_status_flag.sock_read_ongoing = WIFI_FALSE;
                  WiFi_Control_Variables.Q_Contains_Message = WIFI_FALSE;
                  WiFi_Control_Variables.stop_event_dequeue = WIFI_FALSE;
                  WiFi_Counter_Variables.last_process_buffer_index = 5;
                  memset(process_buffer, 0x00, Fillptr);
                  Fillptr=0;

                  if(WiFi_Control_Variables.enable_receive_file_response)
                    WiFi_Control_Variables.FILE_Data_available = WIFI_TRUE;
                  else
                    WiFi_Control_Variables.HTTP_Data_available=WIFI_TRUE;
              }

            else if(((strstr((const char *)process_buffer + WiFi_Counter_Variables.last_process_buffer_index-5,"ERROR"))) != NULL)
              {
                  #if DEBUG_PRINT
                  printf("\r\nERROR\r\n");
                  #endif
                  IO_status_flag.AT_Response_Received = WIFI_TRUE;
                  WiFi_Counter_Variables.AT_RESPONSE = WiFi_AT_CMD_RESP_ERROR;
                  IO_status_flag.sock_read_ongoing = WIFI_FALSE;
                  WiFi_Control_Variables.enable_receive_data_chunk = WIFI_FALSE;
                  WiFi_Control_Variables.Q_Contains_Message = WIFI_FALSE;
                  Fillptr=0;
                  WiFi_Counter_Variables.last_process_buffer_index = 5;
                  WiFi_Control_Variables.enable_receive_http_response = WIFI_FALSE;
                  memset(process_buffer, 0x00, MAX_BUFFER_GLOBAL);

                  if(WiFi_Control_Variables.enable_receive_file_response)
                    WiFi_Control_Variables.FILE_Data_available = WIFI_FALSE;
                  else
                    WiFi_Control_Variables.HTTP_Data_available=WIFI_FALSE;
              }

            if(Fillptr > 5)
                  WiFi_Counter_Variables.last_process_buffer_index = Fillptr;

            if(Fillptr == MAX_BUFFER_GLOBAL-1 )
              {
                  memcpy(UserDataBuff, process_buffer, Fillptr);
                  memcpy(&HTTP_Runway_Buff, &UserDataBuff[505], 6); //for storing the last 6 bytes of Userdatabuff, in case 'OK' is splitted
                  memset(&UserDataBuff[505], 0x00, 6);
                  memset(process_buffer, 0x00, MAX_BUFFER_GLOBAL);
                  memcpy(&process_buffer, &HTTP_Runway_Buff, 6);
                  memset(HTTP_Runway_Buff, 0x00, 6);
                  Fillptr = 6;
                  WiFi_Counter_Variables.last_process_buffer_index = 5;
                  WiFi_Counter_Variables.UserDataBuff_index = 505;
                  Stop_Dequeue();

                  if(WiFi_Control_Variables.enable_receive_file_response)
                    WiFi_Control_Variables.FILE_Data_available = WIFI_TRUE;
                  else
                    WiFi_Control_Variables.HTTP_Data_available = WIFI_TRUE;
              }
            return;
        }
    
     /*********************************************************************************************
      *                                                                                           *
      *                             WiFi Scan Is Enabled.                                       *
      *                                                                                           *
      ********************************************************************************************/
    
    if((!WiFi_Control_Variables.Q_Contains_Message) && WiFi_Control_Variables.enable_receive_wifi_scan_response)
      {
            /*now check if end of msg received*/
            if((process_buffer[Fillptr-2]==0xD) && (process_buffer[Fillptr-1]==0xA))
              {
                if(WiFi_Counter_Variables.scanned_ssids < WiFi_Counter_Variables.user_scan_number)
                  {
                      pStr = (char *) strstr((const char *)&process_buffer,"CHAN:");            
                      if(pStr != NULL)
                          {
                              databytes_No[0] = *(pStr + 6) ;
                              databytes_No[1] = *(pStr + 7) ;
                    
                              chan_value = (((databytes_No[0] - '0') * 10 ) + (databytes_No[1] - '0'));
                          }

                      wifi_scanned_list[WiFi_Counter_Variables.scanned_ssids].channel_num = chan_value;

                      pStr = (char *) strstr((const char *)&process_buffer,"RSSI:");            
                      if(pStr != NULL)
                          {
                              databytes_No[0] = *(pStr + 7) ;
                              databytes_No[1] = *(pStr + 8) ;
                    
                              rssi_value = (((databytes_No[0] - '0') * 10 ) + (databytes_No[1] - '0'));
                          }

                      wifi_scanned_list[WiFi_Counter_Variables.scanned_ssids].rssi = -(rssi_value);

                      pStr = (char *) strstr((const char *)&process_buffer,"SSID:");
                      if(pStr != NULL)
                          {
                              index = 7;
                              while(*(pStr + index) != 0x27)
                                  {
                                      wifi_scanned_list[WiFi_Counter_Variables.scanned_ssids].ssid[index-7] = *(pStr + index);
                                      index++;
                                      if(index==35) break; //max ssid lenght is 30 characters
                                  }                                
                          }

                      pStr = (char *) strstr((const char *)&process_buffer,"WPA ");            
                      if(pStr != NULL)
                          {
                              wifi_scanned_list[WiFi_Counter_Variables.scanned_ssids].sec_type.wpa = WIFI_TRUE;
                          } else
                              wifi_scanned_list[WiFi_Counter_Variables.scanned_ssids].sec_type.wpa = WIFI_FALSE;
                      
                      pStr = (char *) strstr((const char *)&process_buffer,"WPA2 ");            
                      if(pStr != NULL)
                          {
                              wifi_scanned_list[WiFi_Counter_Variables.scanned_ssids].sec_type.wpa2 = WIFI_TRUE;
                          } else
                              wifi_scanned_list[WiFi_Counter_Variables.scanned_ssids].sec_type.wpa2 = WIFI_FALSE;
                      
                      pStr = (char *) strstr((const char *)&process_buffer,"WPS ");            
                      if(pStr != NULL)
                          {
                              wifi_scanned_list[WiFi_Counter_Variables.scanned_ssids].sec_type.wps = WIFI_TRUE;
                          } else
                              wifi_scanned_list[WiFi_Counter_Variables.scanned_ssids].sec_type.wps = WIFI_FALSE;

                      WiFi_Counter_Variables.scanned_ssids++;//increment total_networks
                  }

                //end of one line from SCAN result       
                pStr = (char *) strstr((const char *)&process_buffer,"ERROR");
                if(pStr != NULL)
                  {
                      #if DEBUG_PRINT
                        printf("ERROR Scan Failed"); 
                      #endif
                      memset(process_buffer, 0x00, MAX_BUFFER_GLOBAL);
                      Fillptr=0;
                      IO_status_flag.AT_Response_Received = WIFI_TRUE;
                      WiFi_Control_Variables.enable_receive_wifi_scan_response = WIFI_FALSE;
                      WiFi_Control_Variables.Scan_Ongoing = WIFI_FALSE; //Enable next scan
                      WiFi_Counter_Variables.AT_RESPONSE = WiFi_AT_CMD_RESP_ERROR;
                      return;
                  }

                #if DEBUG_PRINT
                printf((const char*)process_buffer);
                #endif

                if(((strstr((const char *)process_buffer,"OK\r\n"))) != NULL /*|| scanned_ssids==10*/)/*Max 10 networks supported*/
                  {
                      //print and go for next line
                      //If Any part of scan line contains "OK" this will exit!!
                      #if DEBUG_PRINT
                        printf("\r\nOK\r\n");   
                      #endif
                      WiFi_Control_Variables.Scan_Ongoing = WIFI_FALSE; //Enable next scan             
                      WiFi_Counter_Variables.scanned_ssids=0;                       
                      Fillptr=0;
                      IO_status_flag.AT_Response_Received = WIFI_TRUE;
                      WiFi_Control_Variables.enable_receive_wifi_scan_response = WIFI_FALSE;
                      WiFi_Counter_Variables.AT_RESPONSE = WiFi_MODULE_SUCCESS;
                      memset(process_buffer, 0x00, MAX_BUFFER_GLOBAL);    
                      return;
                  }

                Fillptr=0;
                memset(process_buffer, 0x00, MAX_BUFFER_GLOBAL);          
              }

            if(Fillptr>=MAX_BUFFER_GLOBAL-1)
              {
                #if DEBUG_PRINT
                printf("\rHTTP: process_buffer Max Buffer Size reached\r\n");
                #endif          
                Fillptr=0;          
                memset(process_buffer, 0x00, MAX_BUFFER_GLOBAL); 
              }
      }

    
     /*********************************************************************************************
      *                                                                                           *
      *                             FW Update Is Enabled.                                         *
      *                                                                                           *
      ********************************************************************************************/
    
    if((!WiFi_Control_Variables.Q_Contains_Message) && WiFi_Control_Variables.enable_fw_update_read)
      {
            #ifdef DEBUG_PRINT
            printf("*");     //print * till finish    
            #endif

            pStr = (char *) strstr((const char *)process_buffer,"Complete!");

            if(pStr != NULL)
              {
                #ifdef DEBUG_PRINT
                printf("\r\nUpdate complete\r\n");
                #endif
                IO_status_flag.AT_Response_Received = WIFI_TRUE;
                WiFi_Counter_Variables.AT_RESPONSE = WiFi_MODULE_SUCCESS;
                WiFi_Control_Variables.enable_fw_update_read = WIFI_FALSE;
                Fillptr=0;
                memset(process_buffer, 0x00, MAX_BUFFER_GLOBAL);
              }
            if(Fillptr>=MAX_BUFFER_GLOBAL)
              {
                Fillptr=0;
                memset(process_buffer, 0x00, MAX_BUFFER_GLOBAL);
              }

            //No change of state till we get "+WIND:17:F/W update complete!"
      }    

}

/**
* @brief  Process_Wind_Indication_Cmd
*         Process Wind indication before queueing
* @param  process_buff_ptr: pointer of WiFi indication buffer
* @retval None
*/

void Process_Wind_Indication(uint8_t *process_buff_ptr)
{
  char * process_buffer_ptr = (char*)process_buff_ptr; //stores the starting address of process_buffer.
  char * ptr_offset = (char*)process_buff_ptr;
  char Indication_No[2]; 
  char databytes_No[4]; 
  #if DEBUG_PRINT
  printf((const char*)process_buff_ptr);
  #endif
  int i;

  WiFi_Indication_t Wind_No = Undefine_Indication;

  if(ptr_offset != NULL)
    {
      ptr_offset = (char *) strstr((const char *)process_buffer_ptr,"WIND:");

      if(ptr_offset != NULL)
        {
            Indication_No[0] = *(ptr_offset + 5);
            Indication_No[1] = *(ptr_offset + 6);

            if( Indication_No[1] == ':')
              {
                /* Convert char to integer */
                Wind_No = (WiFi_Indication_t)(Indication_No[0] - '0'); 
              }
            else
              {
                /* Convert char to integer */
                Wind_No = (WiFi_Indication_t)(((Indication_No[0] - '0') * 10 ) + (Indication_No[1] - '0'));
              }

            wifi_instances.wifi_event.wind = Wind_No;
            wifi_instances.wifi_event.event = WIFI_WIND_EVENT;

            switch (Wind_No)
            {
                case SockON_Data_Pending: /*WIND:55*/
                    /*+WIND:55:Pending Data:%d:%d*/   
                    ptr_offset = (char *) strstr((const char *)process_buffer_ptr,"Data:");

                    /*Need to find out which socket ID has data pending*/
                    databytes_No[0] = *(ptr_offset + 5);

                    wifi_instances.wifi_event.socket_id = (databytes_No[0] - '0'); //Max number of sockets is 8 (so single digit)
                    break;

                case SockON_Server_Socket_Closed:
                    //Find the id of the socket closed
                    databytes_No[0] = *(ptr_offset + 22) ;
                    wifi_instances.wifi_event.socket_id = databytes_No[0] - '0'; //Max number of sockets is 8 (so single digit)
                    break;      

                case SockD_Pending_Data:
                    if(WiFi_Control_Variables.prevent_push_WIFI_event) 
                      {
                        #ifdef DEBUG_PRINT
                        printf(">>IG\r\n");
                        #endif
                        return; ///exit if push is prevented
                      }

                    /* @TODO: Do something to delay the de-Q of pending packets so that any further
                              pending WIND64 packets are allowed to arrive and get Q-ed */

                    if(WiFi_Control_Variables.event_deQ_x_wind64)      //which means there is already a previous WIND64
                      {
                        WiFi_Counter_Variables.wind64_DQ_wait=0;         //reset the timer
                      }
                    else
                      {
                        WiFi_Control_Variables.stop_event_dequeue = WIFI_TRUE;//Stop the event de-Q
                        WiFi_Control_Variables.event_deQ_x_wind64 = WIFI_TRUE;//Set the flag
                      }

                    //Start Reading data from Client Here.    
                    // +WIND:64:Sockd Pending Data:1:130:130
                    ptr_offset = (char *) strstr((const char *)process_buffer_ptr,"Data:");
                    if(ptr_offset != NULL)
                      {
                        //Store the packet number
                        databytes_No[0] = *(ptr_offset + 5) ;
                        wifi_instances.wifi_event.wind64_pending_packet_no = databytes_No[0] - '0';

                        //And now find the data length
                        databytes_No[0] = *(ptr_offset + 8) ;//points to number just after 2nd colon
                        databytes_No[1] = *(ptr_offset + 9) ;
                        databytes_No[2] = *(ptr_offset + 10) ;
                        databytes_No[3] = *(ptr_offset + 11) ;
              
                        if( databytes_No[0] == ':')//then it is a 1 digit number
                          {
                            databytes_No[0] = *(ptr_offset + 7) ;
                            databytes_No[1] = *(ptr_offset + 8) ;
                          }
                        else if(databytes_No[1] == ':')//two digit number
                          {
                            databytes_No[0] = *(ptr_offset + 7) ;
                            databytes_No[1] = *(ptr_offset + 8) ;
                            databytes_No[2] = *(ptr_offset + 9) ;
                          }
                        else if(databytes_No[2] == ':')//three digit number
                          {
                            databytes_No[0] = *(ptr_offset + 7) ;
                            databytes_No[1] = *(ptr_offset + 8) ;
                            databytes_No[2] = *(ptr_offset + 9) ;
                            databytes_No[3] = *(ptr_offset + 10) ;
                          }
                        else if(databytes_No[3] == ':')//four digit number
                          {
                            databytes_No[0] = *(ptr_offset + 7) ;
                            databytes_No[1] = *(ptr_offset + 8) ;
                            databytes_No[2] = *(ptr_offset + 9) ;
                            databytes_No[3] = *(ptr_offset + 10) ;
                          }

                        if( databytes_No[1] == ':')
                          {
                            WiFi_Counter_Variables.interim_number_of_bytes = databytes_No[0] - '0'; 
                          }
                        else if( databytes_No[2] == ':')
                          {
                            WiFi_Counter_Variables.interim_number_of_bytes = (((databytes_No[0] - '0') * 10 ) + (databytes_No[1] - '0'));
                          }
                        else if( databytes_No[3] == ':')
                          {
                            WiFi_Counter_Variables.interim_number_of_bytes = (((databytes_No[0] - '0') * 100 ) + ((databytes_No[1] - '0') * 10 ) + (databytes_No[2] - '0'));
                          }
                        else //it's a 4-digit number
                          {
                            WiFi_Counter_Variables.interim_number_of_bytes = (((databytes_No[0] - '0') * 1000 ) + ((databytes_No[1] - '0') * 100 ) + ((databytes_No[2] - '0') * 10 ) + (databytes_No[3] - '0'));
                          }            

                        /*WIND:64 came after pop of previous event and switch to data mode was issued*/
                        wifi_instances.wifi_event.data_length = WiFi_Counter_Variables.interim_number_of_bytes; // - (730*WIND64_count);
                        IO_status_flag.WIND64_count++;           //Count of the number of queued WIND:64 Events
                        WiFi_Counter_Variables.interim_number_of_bytes = 0;
                      }
                    break;

                case In_Command_Mode:
                    IO_status_flag.command_mode= WIFI_TRUE;
                    IO_status_flag.data_mode= WIFI_FALSE;
                    IO_status_flag.WIND64_count=0;     //reset the WIND64 count since the previous data mode would have consumed all data
                    WiFi_Control_Variables.queue_wifi_wind_message = WIFI_FALSE;
                    WiFi_Control_Variables.prevent_push_WIFI_event = WIFI_TRUE;
                    break;

                case In_Data_Mode:
                    IO_status_flag.command_mode = WIFI_FALSE;
                    IO_status_flag.data_mode = WIFI_TRUE;
                    WiFi_Control_Variables.queue_wifi_wind_message = WIFI_FALSE;
                    WiFi_Control_Variables.prevent_push_WIFI_event = WIFI_TRUE;

                    if(WiFi_Control_Variables.switch_by_default_to_command_mode == WIFI_TRUE)
                      {
                        if(!IO_status_flag.command_mode)
                          {
                            WiFi_switch_to_command_mode();//switch by default
                          }    
                      }
                    if(WiFi_Control_Variables.data_pending_sockD == WIFI_TRUE)
                      {
                         WiFi_Counter_Variables.last_process_buffer_index =5;
                         WiFi_Control_Variables.enable_receive_data_chunk = WIFI_TRUE;       // read data in chunk now from ring buffer
                         WiFi_Control_Variables.enable_sock_read = WIFI_TRUE;//now data will start coming
                        //Set the data-length to read
                        WiFi_Counter_Variables.SockON_Data_Length = WiFi_Counter_Variables.number_of_bytes;
                      }
                    break;

                case WiFi__MiniAP_Associated:
                    //Find out which client joined by parsing the WIND //+WIND:28
                    ptr_offset = (char *) strstr((const char *)process_buffer_ptr,"+WIND:28");
                    for(i=17;i<=33;i++)
                    WiFi_Counter_Variables.client_MAC_address[i-17] = *(ptr_offset + i) ;    
                    IO_status_flag.WiFi_WIND_State.WiFiAPClientJoined = WIFI_TRUE;
                    break;

                case WiFi_MiniAP_Disassociated:
                    //Find out which client left by parsing the WIND //+WIND:72
                    ptr_offset = (char *) strstr((const char *)process_buffer_ptr,"+WIND:72");
                    for(i=17;i<=33;i++)
                    WiFi_Counter_Variables.client_MAC_address[i-17] = *(ptr_offset + i) ;
                    IO_status_flag.WiFi_WIND_State.WiFiAPClientLeft = WIFI_TRUE;
                    break;

                case Remote_Configuration:
                    wifi_connected = 0;
                    WiFi_Control_Variables.queue_wifi_wind_message = WIFI_FALSE;
                    WiFi_Control_Variables.prevent_push_WIFI_event = WIFI_TRUE;
                    break;
                    
                case Going_Into_Standby:
                    wifi_instances.wifi_event.event = WIFI_STANDBY_CONFIG_EVENT;
                    break;
                    
                case Resuming_From_Standby:
                    wifi_instances.wifi_event.event = WIFI_RESUME_CONFIG_EVENT;
                    break;

                case Console_Active:           // Queueing of these events not required.
                case Poweron:
                case WiFi_Reset:
                case Watchdog_Running:
                case Watchdog_Terminating:
                case SysTickConfigure:
                case CopyrightInfo:
                case WiFi_BSS_Regained:
                case WiFi_Signal_OK:
                case FW_update:
                case Encryption_key_Not_Recognized:
                case WiFi_Join:
                case WiFi_Scanning:
                case WiFi_Association_Successful:
                case WiFi_BSS_LOST:
                case WiFi_NETWORK_LOST:
                case WiFi_Unhandled_Event:
                case WiFi_UNHANDLED:
                case WiFi_MiniAP_Mode :
                case DOT11_AUTHILLEGAL:
                case Creating_PSK:
                case WPA_Terminated :
                case WPA_Supplicant_Failed:
                case WPA_Handshake_Complete:
                case GPIO_line:
                case Wakeup:
                case Factory_debug:
                case Low_Power_Mode_Enabled:
                case Rejected_Found_Network:
                    WiFi_Control_Variables.queue_wifi_wind_message = WIFI_FALSE;
                    WiFi_Control_Variables.prevent_push_WIFI_event = WIFI_TRUE;
                    break;

                  //Queue these Events.
                case MallocFailed:                                          
                    if(WiFi_Control_Variables.stop_event_dequeue)
                      WiFi_Control_Variables.stop_event_dequeue = WIFI_FALSE;
                    break;
                case Heap_Too_Small:            
                case WiFi_Hardware_Dead:
                case Hard_Fault:
                case StackOverflow:                   
                case Error:
                case WiFi_PS_Mode_Failure:
                case WiFi_Signal_LOW:
                case JOINFAILED:
                case SCANBLEWUP:
                case SCANFAILED:
                case WiFi_Started_MiniAP_Mode:
                case Start_Failed :
                case WiFi_EXCEPTION :
                case WiFi_Hardware_Started :
                case Scan_Complete:
                case WiFi_UNHANDLED_IND:
                case WiFi_Powered_Down:
                case WiFi_Deauthentication:
                case WiFi_Disassociation:
                case RX_MGMT:
                case RX_DATA:
                case RX_UNK:
                     break;
                default:
                     break;
              }
           memset(process_buffer_ptr, 0x00, MAX_BUFFER_GLOBAL);
        }
     }
}

/**
* @brief  Process_Dequeued_Wind_Indication
*         Process Wind Indication after popping from Queue
* @param  L_DeQued_wifi_event popped event contents
* @retval None
*/

void Process_DeQed_Wind_Indication(wifi_event_TypeDef * L_DeQued_wifi_event)
{
  switch(L_DeQued_wifi_event->wind)
  {
    case Heap_Too_Small:
      IO_status_flag.WiFi_WIND_State.HeapTooSmall=WIFI_TRUE;
      break;
    case WiFi_Hardware_Dead:
      IO_status_flag.WiFi_WIND_State.WiFiHWFailure = WIFI_TRUE;
      break;
    case Hard_Fault:
      IO_status_flag.WiFi_WIND_State.HardFault = WIFI_TRUE;
      break;   
    case StackOverflow:
      IO_status_flag.WiFi_WIND_State.StackOverflow = WIFI_TRUE;
      break;
    case MallocFailed:
      IO_status_flag.WiFi_WIND_State.MallocFailed = WIFI_TRUE;
      break;
    case Error:
      IO_status_flag.WiFi_WIND_State.InitFailure = WIFI_TRUE;
      break;
    case WiFi_PS_Mode_Failure:
      IO_status_flag.WiFi_WIND_State.PS_Mode_Failure = WIFI_TRUE;
      break;
    case WiFi_Signal_LOW:
      IO_status_flag.WiFi_WIND_State.WiFiSignalLOW = WIFI_TRUE;
      break;
    case JOINFAILED :
      IO_status_flag.WiFi_WIND_State.WiFiJoinFailed = WIFI_TRUE;
      break;
    case SCANBLEWUP:
      IO_status_flag.WiFi_WIND_State.WiFiScanBlewUp = WIFI_TRUE;
      break;
    case SCANFAILED:
      IO_status_flag.WiFi_WIND_State.WiFiScanFailed = WIFI_TRUE;
      break;
    case WiFi_Up:
      IO_status_flag.WiFi_WIND_State.WiFiUp = WIFI_TRUE;
      break;
    case WiFi_Started_MiniAP_Mode:
      IO_status_flag.WiFi_WIND_State.WiFiStarted_MiniAPMode = WIFI_TRUE;
      break;
    case Start_Failed :
      IO_status_flag.WiFi_WIND_State.StartFailed = WIFI_TRUE;  
      break;
    case WiFi_EXCEPTION :
      IO_status_flag.WiFi_WIND_State.WiFiException = WIFI_TRUE;
      break;
    case WiFi_Hardware_Started :
      IO_status_flag.wifi_ready++;
      IO_status_flag.WiFi_Enabled = WIFI_TRUE;
      IO_status_flag.WiFi_WIND_State.WiFiHWStarted = WIFI_TRUE;
      /*If this is a start-up after standby*/
      if(WiFi_Control_Variables.trigger_wakeup_callback == WIFI_TRUE)
        {
          WiFi_Control_Variables.trigger_wakeup_callback = WIFI_FALSE;
          WiFi_Control_Variables.Standby_Enabled = WIFI_FALSE;
          WiFi_Control_Variables.standby_resume_callback = WIFI_TRUE;
        }      
      break;
    case Scan_Complete:
      WiFi_Control_Variables.Scan_Ongoing = WIFI_FALSE;
      break;
    case WiFi_UNHANDLED_IND:
      IO_status_flag.WiFi_WIND_State.WiFiUnHandledInd = WIFI_TRUE;
      break;
    case WiFi_Powered_Down:
      IO_status_flag.WiFi_Enabled = WIFI_FALSE;      
      //wifi_ready = 0;
      IO_status_flag.WiFi_WIND_State.WiFiHWStarted = WIFI_FALSE;
      IO_status_flag.WiFi_WIND_State.WiFiPowerDown = WIFI_TRUE;
      break;
    case WiFi_Deauthentication:
      IO_status_flag.WiFi_WIND_State.WiFiDeauthentication = WIFI_TRUE;
      break;
    case WiFi_Disassociation:
      wifi_connected = 0; 
      IO_status_flag.WiFi_WIND_State.WiFiDisAssociation = WIFI_TRUE;
      break;  
    case RX_MGMT:
      IO_status_flag.WiFi_WIND_State.WiFiRXMgmt = WIFI_TRUE;
      break;
    case RX_DATA:
      IO_status_flag.WiFi_WIND_State.WiFiRXData = WIFI_TRUE;
      break;
    case RX_UNK:
      IO_status_flag.WiFi_WIND_State.WiFiRxUnk = WIFI_TRUE;
      break;
    case SockON_Data_Pending:
      /* +WIND:55:Pending Data:%d:%d */
      if (WiFi_Control_Variables.enable_sock_read == WIFI_TRUE)
        {       
            #if DEBUG_PRINT
              printf ("\nAlert!\r\n");
            #endif
            WiFi_Control_Variables.enable_sock_read = WIFI_FALSE;
            WiFi_Control_Variables.enable_receive_data_chunk = WIFI_FALSE;
            //break;
        }
      WiFi_Counter_Variables.sockon_query_id = L_DeQued_wifi_event->socket_id;
      if(open_sockets[WiFi_Counter_Variables.sockon_query_id])
        {
            WiFi_Control_Variables.enable_query = WIFI_TRUE;
            WiFi_Control_Variables.stop_event_dequeue = WIFI_TRUE;
        }
      break;

    case SockON_Server_Socket_Closed:
      WiFi_Counter_Variables.client_socket_close_id = L_DeQued_wifi_event->socket_id;
      WiFi_Control_Variables.Client_Socket_Close_Cmd = WIFI_TRUE;
      break;
    case SockD_Pending_Data:
      WiFi_Counter_Variables.number_of_bytes = L_DeQued_wifi_event->data_length;
      WiFi_Control_Variables.data_pending_sockD = WIFI_TRUE;
      WiFi_Control_Variables.stop_event_dequeue = WIFI_TRUE;             //Stop any more event de-queue
      //WiFi_Control_Variables.enable_receive_data_chunk = WIFI_TRUE;       // read data in chunk now from ring buffer

      if(!IO_status_flag.data_mode)
        {
          if(L_DeQued_wifi_event->wind64_pending_packet_no == 1)
            {   //If this is the first WIND64 pending event de-Q'ed
                WiFi_Control_Variables.switch_by_default_to_command_mode = WIFI_FALSE; //we don't want to switch back to command mode after changing to data mode here
                WiFi_switch_to_data_mode();     //switch by default
            }
          else
            {
                WiFi_Control_Variables.data_pending_sockD = WIFI_FALSE;
                WiFi_Control_Variables.stop_event_dequeue = WIFI_FALSE; 
            }
        }
      else //already data is coming from previous WIND:64
        {
            WiFi_Control_Variables.enable_receive_data_chunk = WIFI_TRUE;
            WiFi_Counter_Variables.last_process_buffer_index =5;
            WiFi_Control_Variables.enable_sock_read = WIFI_TRUE;//n
            WiFi_Counter_Variables.SockON_Data_Length = WiFi_Counter_Variables.number_of_bytes;
        }
      break;
    case Incoming_socket_client:
      WiFi_Control_Variables.Client_Connected = WIFI_TRUE;
      wifi_client_connected=1;  //Set this so that the callback can be made to the user
      break;
    case Outgoing_socket_client:
      WiFi_Control_Variables.Client_Disconnected = WIFI_TRUE;
      wifi_client_disconnected=0;//Set this so that the callback can be made to the user
      wifi_client_connected = 0;
      break;
    case SockD_Dropping_Data:
      IO_status_flag.WiFi_WIND_State.WiFiSockdDataLost = WIFI_TRUE;
      break;
    case Going_Into_Standby:
      WiFi_Control_Variables.Standby_Enabled = WIFI_TRUE;
      break;
    case Resuming_From_Standby:
      WiFi_Control_Variables.Standby_Enabled = WIFI_FALSE;
      WiFi_Control_Variables.standby_resume_callback = WIFI_TRUE;
      break;    
    case Going_Into_DeepSleep:
      WiFi_Control_Variables.Deep_Sleep_Enabled = WIFI_TRUE;    
      break;
    case Resuming_From_DeepSleep:
      WiFi_Control_Variables.Deep_Sleep_Enabled = WIFI_FALSE;
      Start_DeepSleep_Timer();
      break;
    default:
      break;
  }
}

/**
* @brief  Queue_Http_Get_ Event
*         Queue an HTTP-Request Event (GET/POST)
* @param  hostname hostname for HTTP-GET/POST
* @param  path path for HTTP-GET
* @param  port_number port_number for HTTP-GET
* @param  pURL_path full URL for HTTP-POST
* @retval None
*/

void Queue_Http_Event(uint8_t * hostname, uint8_t * path, uint32_t port_number, uint8_t * pURL_path) 
{
   Wait_For_Sock_Read_To_Complete();
   if(pURL_path == NULL)
    {
      WiFi_Counter_Variables.curr_hostname = hostname;
      WiFi_Counter_Variables.curr_path = path;
      WiFi_Counter_Variables.curr_port_number = port_number;
    }
    else
      WiFi_Counter_Variables.curr_pURL = pURL_path;

    wifi_instances.wifi_event.event = WIFI_HTTP_EVENT;
    __disable_irq();
    push_eventbuffer_queue(&wifi_instances.event_buff, wifi_instances.wifi_event);
     __enable_irq();
    
    reset_event(&wifi_instances.wifi_event);
}

/**
* @brief  Queue_Client_Write_Event
*         Queues a Client Socket write event.
* @param  sock_id socket ID to write to
* @param  DataLength length of the data to be written
* @param  pData pointer to data
* @retval None
*/
void Queue_Client_Write_Event(uint8_t sock_id, uint16_t DataLength, char * pData)
{
    Wait_For_Sock_Read_To_Complete();
    WiFi_Counter_Variables.curr_DataLength = DataLength;
    WiFi_Counter_Variables.curr_data = pData;
    WiFi_Counter_Variables.curr_sockID = sock_id;

    wifi_instances.wifi_event.event = WIFI_CLIENT_SOCKET_WRITE_EVENT;
    __disable_irq();
    push_eventbuffer_queue(&wifi_instances.event_buff, wifi_instances.wifi_event);
    __enable_irq();

    reset_event(&wifi_instances.wifi_event);
}

/**
* @brief Queue_Wifi_File_Image_Create_Event
*        Queue a File Image Create Event
* @param pHostName hostname
* @param pFileName filename within host
* @param port_number port number to connect to
* @retval None
*/
void Queue_Wifi_File_Event(uint8_t * pHostName, uint8_t * pFileName, uint32_t port_number)
{
    Wait_For_Sock_Read_To_Complete();
    wifi_instances.wifi_event.event = WIFI_FILE_EVENT;
    WiFi_Counter_Variables.curr_filename = pFileName;
    WiFi_Counter_Variables.curr_hostname = pHostName;
    WiFi_Counter_Variables.curr_port_number = port_number;

    __disable_irq();
    push_eventbuffer_queue(&wifi_instances.event_buff, wifi_instances.wifi_event);
    __enable_irq();

    reset_event(&wifi_instances.wifi_event);
}

/**
* @brief Queue_Wifi_FW_Update_Event
*        Queue a Firmware update Event
* @param hostname hostname
* @param filename_path filename and path within host
* @param port_number port number to connect to
* @retval None
*/
void Queue_Wifi_FW_Update_Event(uint8_t * hostname, uint8_t * filename_path, uint32_t port_number)
{
    Wait_For_Sock_Read_To_Complete();
    wifi_instances.wifi_event.event = WIFI_FW_UPDATE_EVENT;
    WiFi_Counter_Variables.curr_filename = filename_path;
    WiFi_Counter_Variables.curr_hostname = hostname;
    WiFi_Counter_Variables.curr_port_number = port_number;

    __disable_irq();
    push_eventbuffer_queue(&wifi_instances.event_buff, wifi_instances.wifi_event);
    __enable_irq();

    reset_event(&wifi_instances.wifi_event);
}

/**
* @brief Queue_Client_Open_Event
*        Queue a Client Open Event
* @param hostname hostname
* @param port_number port number to connect to
* @param protocol protocol required to connect to server (t for TCP socket, u for UDP socket, s for secure socket)
* @retval void
*/
void Queue_Client_Open_Event(uint8_t * hostname, uint32_t port_number, uint8_t * protocol)
{
    Wait_For_Sock_Read_To_Complete();
    WiFi_Counter_Variables.curr_hostname = hostname;
    WiFi_Counter_Variables.curr_port_number = port_number;
    WiFi_Counter_Variables.curr_protocol = protocol;

    wifi_instances.wifi_event.event = WIFI_CLIENT_SOCKET_OPEN_EVENT;
     __disable_irq();
    push_eventbuffer_queue(&wifi_instances.event_buff, wifi_instances.wifi_event);
    __enable_irq();

    reset_event(&wifi_instances.wifi_event);
}

/**
* @brief Queue_Client_Close_Event
*        Queue a Client Close Event
* @param sock_id socket ID to close
* @retval void
*/
void Queue_Client_Close_Event(uint8_t sock_id)
{
    Wait_For_Sock_Read_To_Complete();
    
    wifi_instances.wifi_event.event = WIFI_CLIENT_SOCKET_CLOSE_EVENT;
    wifi_instances.wifi_event.socket_id = sock_id;
     __disable_irq();
    push_eventbuffer_queue(&wifi_instances.event_buff, wifi_instances.wifi_event);
    __enable_irq();

    reset_event(&wifi_instances.wifi_event);
}

/**
* @brief Wait_For_Sock_Read_To_Complete
*        Wait till sock read is over and the OK of read arrives
* @param None
* @retval None
*/
void Wait_For_Sock_Read_To_Complete(void)
{
  //wait if read is ongoing or read OK is yet to arrive
  while(IO_status_flag.sock_read_ongoing == WIFI_TRUE || 
       (IO_status_flag.prevent_push_OK_event == WIFI_TRUE && IO_status_flag.client_socket_close_ongoing != WIFI_TRUE)) // to make sure the prevent_push_OK_event is of socket read and not of socket close.
    {
        __NOP(); //nothing to do
    }
}

/**
* @brief  Reset_AT_CMD_Buffer
*         Clear USART2 Rx buffer and Wi-Fi AT cmd buffer
* @param  None
* @retval None
*/
void Reset_AT_CMD_Buffer()
{
  memset(WiFi_AT_Cmd_Buff, 0x00, sizeof WiFi_AT_Cmd_Buff); 
}

#ifdef  USE_FULL_ASSERT

/**
* @brief  Reports the name of the source file and the source line number
*         where the assert_param error has occurred.
* @param  file: pointer to the source file name
* @param  line: assert_param error line source number
* @retval None
*/
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
  ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  
  /* Infinite loop */
  while (1)
    {
      __NOP(); //nothing to do
    }
}

#endif

/**
* @brief  Read_WiFi_Mode
*         Read Wi-Fi mode 0: idle,1 =STA,2 =IBSS,3 =MiniAP
* @param  string : return wifi mode type
* @retval return status of AT cmd request
*/
WiFi_Status_t Read_WiFi_Mode(char *string)
{
  WiFi_Status_t status = WiFi_MODULE_SUCCESS;
  char *mode = "wifi_mode";
  char *pStr;
  
    /* AT+S.GCFG=wifi_mode */
  Reset_AT_CMD_Buffer();
  
  /* AT : send AT command */  
  sprintf((char*)WiFi_AT_Cmd_Buff,AT_GET_CONFIGURATION_VALUE,mode);  
  
  status = USART_Transmit_AT_Cmd(strlen((char*)WiFi_AT_Cmd_Buff));
  if(status == WiFi_MODULE_SUCCESS)
    {
      status = USART_Receive_AT_Resp( );
    }
  
  pStr = (char *) strstr((const char *)&WiFi_Counter_Variables.get_cfg_value,"wifi_mode = ");
  if(pStr != NULL)
    {
      string[0] = *(pStr + 12) ;
    }

  return status ;
}

/**
* @brief  Write_WiFi_SSID
*         Store SSID in flash memory of WiFi module
* @param  string : pointer of SSID
* @retval return status of AT cmd request
*/
WiFi_Status_t Write_WiFi_SSID(char *string)
{
  WiFi_Status_t status = WiFi_MODULE_SUCCESS;  
  Reset_AT_CMD_Buffer(); 
  
  /* AT+S.SSIDTXT=abcd <ExampleSSID> //set SSID */
  sprintf((char*)WiFi_AT_Cmd_Buff,AT_SET_SSID,string);  
  
  status = USART_Transmit_AT_Cmd(strlen((char*)WiFi_AT_Cmd_Buff));
  if(status == WiFi_MODULE_SUCCESS)
    {
      status = USART_Receive_AT_Resp( );
    }

  /* AT&W :Save the settings on the flash memory */
  Reset_AT_CMD_Buffer();
  Save_Current_Setting();

  return status;
}

/**
* @brief  Write_WiFi_SecKey
*         Store security key in flash memory of WiFi module
* @param  string : pointer of security key
* @retval return status of AT cmd request
*/
WiFi_Status_t Write_WiFi_SecKey(char *string)
{
  WiFi_Status_t status = WiFi_MODULE_SUCCESS;  
  Reset_AT_CMD_Buffer(); 
  
  /* AT+S.SCFG=wifi_wpa_psk_text,helloworld : set password */
  sprintf((char*)WiFi_AT_Cmd_Buff,"AT+S.SCFG=wifi_wpa_psk_text,%s\r",string);
  
  status = USART_Transmit_AT_Cmd(strlen((char*)WiFi_AT_Cmd_Buff));
  if(status == WiFi_MODULE_SUCCESS)
    {
      status = USART_Receive_AT_Resp( );
    }
  
  /* AT&W :Save the settings on the flash memory */
  Reset_AT_CMD_Buffer();
  Save_Current_Setting();
  
  return status;     
}

/**
* @brief  PrintErrorMsg
*         Print error message on UART terminal
* @param  None
* @retval None
*/
void PrintErrorMsg (void)
{
  Print_Msg("error in AT cmd",sizeof("error in AT cmd"));
}

/**
  * @brief  Print_Msg
  *         Print messages on UART terminal
  * @param  msgBuff : Contains data that need to be print
  * @param  length  : leghth of the data
  * @retval None
  */
void Print_Msg(char * msgBuff,uint8_t length)
{

}

/**
* @brief  Error_Handler
*         This function is executed in case of error occurrence.
* @param  None
* @retval None
*/
void Error_Handler(void)
{
  /* Turn LED2 on */
  BSP_LED_On(LED2);
  //The following while(1) is commented as it prevents standby functionality
  /*while(1)
  {
    //Error if LED2 is slowly blinking (1 sec. period)
    BSP_LED_Toggle(LED2); 
    HAL_Delay(1000); 
  } */ 
  Receive_Data();
}

/**
* @brief  USART_Transmit_AT_Cmd
*         send AT cmd on UART port of wifi module.
* @param  size size of the AT command
* @retval WiFi_Status_t : status of AT cmd
*/
WiFi_Status_t USART_Transmit_AT_Cmd(uint16_t size)
{
  //Check for Hardware Started
  if(IO_status_flag.WiFi_Enabled == WIFI_FALSE) 
    return WiFi_NOT_READY;
  //Check for Deep-Sleep or Standby Mode, return error if true
  if (WiFi_Control_Variables.Standby_Enabled == WIFI_TRUE || WiFi_Control_Variables.Deep_Sleep_Enabled == WIFI_TRUE)
    return WiFi_IN_LOW_POWER_ERROR;

  WiFi_Control_Variables.AT_Cmd_Processing = WIFI_TRUE;//Stop Any Rx between the TX call

  if (size == 0)
    {
        printf("ERR in USART_Transmit_AT_Cmd!");
        return WiFi_UNHANDLED_IND_ERROR;
    }

#if defined(USART3_INT_MODE)
  if(HAL_UART_Transmit_IT(&UartWiFiHandle, (uint8_t *)WiFi_AT_Cmd_Buff, size)!= HAL_OK)
    {
      Error_Handler();
      return WiFi_HAL_UART_ERROR;
    }
  while (IO_status_flag.UartReady != SET)
    {
      __NOP(); //nothing to do
    }
  IO_status_flag.UartReady = RESET; 

#elif defined(USART3_POLLING_MODE)
  //while(Uartx_Rx_Processing!=WIFI_FALSE);
  if(HAL_UART_Transmit(&UartWiFiHandle, (uint8_t *)WiFi_AT_Cmd_Buff, size, 1000)!= HAL_OK)
    {
      Error_Handler();
      #if DEBUG_PRINT
      printf("HAL_UART_Transmit Error");
      #endif
      return WiFi_HAL_UART_ERROR;
    }

#else
 #error "Please select USART mode in your application (in wifi_module.h file)"
#endif
 
  WiFi_Control_Variables.AT_Cmd_Processing = WIFI_FALSE;//Re-enable Rx for UART
  if(WiFi_Control_Variables.Uartx_Rx_Processing == WIFI_FALSE)
    Receive_Data();//Start receiving Rx from the UART again, if and only if it was stopped in the previous Uartx_Rx_Handler
  return WiFi_MODULE_SUCCESS;
}

/**
* @brief  Start_DeepSleep_Timer
*         start the deep sleep timer.
* @param  None
* @retval void
*/
void Start_DeepSleep_Timer(void)
{
  WiFi_Control_Variables.Deep_Sleep_Timer = WIFI_TRUE;
  WiFi_Counter_Variables.sleep_count = 0;
}

/**
* @brief  Stop_DeepSleep_Timer
*         stop the deep sleep timer.
* @param  None
* @retval void
*/
void Stop_DeepSleep_Timer()
{
  WiFi_Control_Variables.Deep_Sleep_Timer = WIFI_FALSE;
  WiFi_Counter_Variables.sleep_count = 0;
}

/**
* @brief  WiFi_switch_to_command_mode
*         switch to command mode from data mode
* @param  None
* @retval None
*/
void WiFi_switch_to_command_mode(void)
{
  WiFi_Status_t status = WiFi_MODULE_SUCCESS;

  /* AT+S.*/  
  Reset_AT_CMD_Buffer();  

  sprintf((char*)WiFi_AT_Cmd_Buff,AT_DATA_TO_CMD_MODE);   //Notice the lower case

  status = USART_Transmit_AT_Cmd(strlen((char*)WiFi_AT_Cmd_Buff));
  if(status == WiFi_MODULE_SUCCESS)
  {
    //nothing to do
  }
}

/**
* @brief  WiFi_switch_to_data_mode
*         switch to data mode from command mode
* @param  None
* @retval None
*/
void WiFi_switch_to_data_mode(void)
{
  WiFi_Status_t status = WiFi_MODULE_SUCCESS;

  /* AT+S.*/  
  Reset_AT_CMD_Buffer();    

  sprintf((char*)WiFi_AT_Cmd_Buff,AT_CMD_TO_DATA_MODE);   //Notice the upper case

  status = USART_Transmit_AT_Cmd(strlen((char*)WiFi_AT_Cmd_Buff));

  if(status == WiFi_MODULE_SUCCESS)
    {
      //nothing to do
    }
}

/**
* @brief  Attention_Cmd
*         Attention command
* @param  None
* @retval WiFi_Status_t : status of AT cmd Request
*/
WiFi_Status_t Attention_Cmd()
{
  WiFi_Status_t status = WiFi_MODULE_SUCCESS;

  Reset_AT_CMD_Buffer(); 

  /* AT : send AT command */
  sprintf((char*)WiFi_AT_Cmd_Buff,AT_ATTENTION);  

  status = USART_Transmit_AT_Cmd(strlen((char*)WiFi_AT_Cmd_Buff));
  if(status == WiFi_MODULE_SUCCESS)
    {
      status = USART_Receive_AT_Resp( );
    }
  return status; 
}

/**
* @brief  SET_Power_State
*         SET power mode of wifi module
* @param  state : power mode of wi-fi module i.e active,sleep,standby,powersave
* @retval WiFi_Status_t : status of AT cmd Request
*/
WiFi_Status_t SET_Power_State(WiFi_Power_State_t state)
{
  WiFi_Status_t status = WiFi_MODULE_SUCCESS;

#if DEBUG_PRINT  
  printf("\r\n >>Soft Reset Wi-Fi module\r\n");
#endif

  Reset_AT_CMD_Buffer();

  /* AT : send AT command */
  sprintf((char*)WiFi_AT_Cmd_Buff,AT_SET_POWER_STATE,state);  
//  WiFi_WIND_State.WiFiReset = WIFI_FALSE;
  IO_status_flag.WiFi_WIND_State.WiFiHWStarted = WIFI_FALSE;
  status = USART_Transmit_AT_Cmd(strlen((char*)WiFi_AT_Cmd_Buff));
  if(status != WiFi_MODULE_SUCCESS) 
    return status;
  memset((void*)&IO_status_flag.WiFi_WIND_State,0x00,sizeof(IO_status_flag.WiFi_WIND_State)); /*reset the WIND State?*/
  /* AT+CFUN=1 //Soft reset */
  while(IO_status_flag.WiFi_WIND_State.WiFiHWStarted != WIFI_TRUE)
  {
    __NOP(); //nothing to do
  }
  return status;
}

/**
* @brief  Display_Help_Text
*         this function will print a list of all commands supported with a brief help text for each cmd
* @param  None
* @retval WiFi_Status_t : status of AT cmd Request
*/
WiFi_Status_t Display_Help_Text()
{
  WiFi_Status_t status = WiFi_MODULE_SUCCESS;
  Reset_AT_CMD_Buffer();

  /* AT : send AT command */
  sprintf((char*)WiFi_AT_Cmd_Buff,AT_HELP_TEXT);  

  status = USART_Transmit_AT_Cmd(strlen((char*)WiFi_AT_Cmd_Buff));
  if(status == WiFi_MODULE_SUCCESS)
    {
      status = USART_Receive_AT_Resp( );
    }
  return status; 
}

/**
* @brief  GET_Configuration_Value
*         Get a wifi configuration value from the module
* @param  sVar_name : Name of the config variable
*         aValue    : value of config variable to be returned to user
* @retval WiFi_Status_t : status of AT cmd Request
*/
WiFi_Status_t GET_Configuration_Value(char* sVar_name,uint32_t *aValue)
{
  int cfg_value_length;
  WiFi_Status_t status = WiFi_MODULE_SUCCESS;

  Reset_AT_CMD_Buffer(); 

  /* AT : send AT command */  
  sprintf((char*)WiFi_AT_Cmd_Buff,AT_GET_CONFIGURATION_VALUE,sVar_name);   

  status = USART_Transmit_AT_Cmd(strlen((char*)WiFi_AT_Cmd_Buff));
  if(status == WiFi_MODULE_SUCCESS)
    {
      status = USART_Receive_AT_Resp( );
      cfg_value_length = strlen((const char*)WiFi_Counter_Variables.get_cfg_value);
      memcpy(aValue,WiFi_Counter_Variables.get_cfg_value,cfg_value_length);   //copy user pointer to get_cfg_value
      memset(WiFi_Counter_Variables.get_cfg_value, 0x00,cfg_value_length);
    }
  return status; 
}

/**
* @brief  SET_Configuration_Addr
*         Get a wifi configuration address from the module
* @param  sVar_name : Name of the config variable
*         addr    : value of config address to be returned to user
* @retval WiFi_Status_t : status of AT cmd Request
*/
WiFi_Status_t SET_Configuration_Addr(char* sVar_name,char* addr)
{
  WiFi_Status_t status = WiFi_MODULE_SUCCESS;

  Reset_AT_CMD_Buffer(); 

  /* AT : send AT command */
  sprintf((char*)WiFi_AT_Cmd_Buff,AT_SET_CONFIGURATION_ADDRESS,sVar_name,addr);  

  status = USART_Transmit_AT_Cmd(strlen((char*)WiFi_AT_Cmd_Buff));
  if(status == WiFi_MODULE_SUCCESS)
    {
      status = USART_Receive_AT_Resp( );
    }
  return status;
}

/**
* @brief  SET_Configuration_Value
*         SET the value of configuration variable
* @param  sVar_name : Name of the config variable
*         aValue    : value of config variable
* @retval WiFi_Status_t : status of AT cmd Request
*/
WiFi_Status_t SET_Configuration_Value(char* sVar_name,uint32_t aValue)
{
  WiFi_Status_t status = WiFi_MODULE_SUCCESS;

  Reset_AT_CMD_Buffer(); 

  /* AT : send AT command */
  sprintf((char*)WiFi_AT_Cmd_Buff,AT_SET_CONFIGURATION_VALUE,sVar_name,(int)aValue);  

  status = USART_Transmit_AT_Cmd(strlen((char*)WiFi_AT_Cmd_Buff));
  if(status == WiFi_MODULE_SUCCESS)
    {
      status = USART_Receive_AT_Resp( );
    }
  return status; 
}

/**
* @brief  SET_SSID
*         SET SSID in flash memory of Wi-Fi module
* @param  ssid : pointer of SSID
* @retval WiFi_Status_t : status of AT cmd Request
*/
WiFi_Status_t SET_SSID(char* ssid)
{
  WiFi_Status_t status = WiFi_MODULE_SUCCESS;
  
  Reset_AT_CMD_Buffer(); 
  
  /* AT+S.SSIDTXT=abcd <ExampleSSID>  */
  sprintf((char*)WiFi_AT_Cmd_Buff,AT_SET_SSID,ssid);  

  status = USART_Transmit_AT_Cmd(strlen((char*)WiFi_AT_Cmd_Buff));
  if(status == WiFi_MODULE_SUCCESS)
    {
      status = USART_Receive_AT_Resp( );
    }
  return status; 
}


/**
* @brief  SET_WiFi_SecKey
*         SET wifi security key
* @param  seckey : pointer of security key
* @retval WiFi_Status_t : status of AT cmd Request
*/
WiFi_Status_t SET_WiFi_SecKey(char* seckey)
{
  WiFi_Status_t status = WiFi_MODULE_SUCCESS;
  
  Reset_AT_CMD_Buffer(); 
  
  /* AT+S.SCFG=wifi_wpa_psk_text,helloworld : set password */
  sprintf((char*)WiFi_AT_Cmd_Buff,AT_SET_SEC_KEY,seckey);  

  status = USART_Transmit_AT_Cmd(strlen((char*)WiFi_AT_Cmd_Buff));
  if(status == WiFi_MODULE_SUCCESS)
    {
      status = USART_Receive_AT_Resp( );
    }
  return status;    
}


/**
* @brief  Restore_Default_Setting
*         Restore the factory default values of the configuration variables 
*         and writes them to non volatile storage
* @param  None
* @retval WiFi_Status_t : status of AT cmd Request
*/
WiFi_Status_t Restore_Default_Setting()
{
  WiFi_Status_t status = WiFi_MODULE_SUCCESS;

  //Reset_AT_CMD_Buffer(); 

  /* AT&F: restore default setting */
  sprintf((char*)WiFi_AT_Cmd_Buff,AT_RESTORE_DEFAULT_SETTING);  

  status = USART_Transmit_AT_Cmd(strlen((char*)WiFi_AT_Cmd_Buff));

  if(status == WiFi_MODULE_SUCCESS)
    {
      status = USART_Receive_AT_Resp( );
    }
  return status;
}

/**
* @brief  Save_Current_Setting
*         Store the current RAM-based setting to non-volatile storage
* @param  None
* @retval WiFi_Status_t : status of AT cmd Request
*/
WiFi_Status_t Save_Current_Setting()
{
  WiFi_Status_t status = WiFi_MODULE_SUCCESS;
  
  Reset_AT_CMD_Buffer(); 
  
  /* AT&W :Save the settings on the flash memory */
  sprintf((char*)WiFi_AT_Cmd_Buff,AT_SAVE_CURRENT_SETTING);  

  status = USART_Transmit_AT_Cmd(strlen((char*)WiFi_AT_Cmd_Buff));
  if(status == WiFi_MODULE_SUCCESS)
    {
      status = USART_Receive_AT_Resp( );
    }
  return status; 
}

/**
* @brief  ResetBuffer
*         Reset receive data/indication msg buffer
* @param  None
* @retval None
*/
void ResetBuffer()
{  
  
}

/**
* @brief  config_init_value
*         initalize config values before reset
* @param  sVar_name : Name of the config variable
*         aValue    : value of config variable
* @retval None
*/
WiFi_Status_t config_init_value(char* sVar_name,uint8_t aValue)
{
  WiFi_Status_t status = WiFi_MODULE_SUCCESS;
  Reset_AT_CMD_Buffer();   
  sprintf((char*)WiFi_AT_Cmd_Buff,AT_SET_CONFIGURATION_VALUE,sVar_name,aValue);
  if(HAL_UART_Transmit(&UartWiFiHandle, (uint8_t *)WiFi_AT_Cmd_Buff, strlen((char*)WiFi_AT_Cmd_Buff),1000)!= HAL_OK)
    {
      Error_Handler();    
      return WiFi_HAL_UART_ERROR;
    }
  
  status = WaitForResponse(AT_RESP_LEN_OK);
  return status;
}

/**
* @brief  config_init_addr
*         initalize config strings/addresses before reset
* @param  sVar_name : Name of the config variable
*         addr    : value of config address to be returned to user
* @retval None
*/
WiFi_Status_t config_init_addr(char* sVar_name,char* addr)
{
  WiFi_Status_t status = WiFi_MODULE_SUCCESS;
  Reset_AT_CMD_Buffer();   
  sprintf((char*)WiFi_AT_Cmd_Buff,AT_SET_CONFIGURATION_ADDRESS,sVar_name,addr);
  if(HAL_UART_Transmit(&UartWiFiHandle, (uint8_t *)WiFi_AT_Cmd_Buff, strlen((char*)WiFi_AT_Cmd_Buff),1000)!= HAL_OK)
    {
      Error_Handler();    
      return WiFi_HAL_UART_ERROR;
    }
  
  status = WaitForResponse(AT_RESP_LEN_OK);
  return status;

}


/**
* @brief  WaitForResponse
*         Wait for OK response
* @param  alength length of the data to be received
* @retval None
*/
WiFi_Status_t WaitForResponse(uint16_t alength)
{
  uint8_t USART_RxBuffer[64];    //This buffer is only used in the Init phase (to receive "\r\nOK\r\n")
  WiFi_Status_t status = WiFi_MODULE_SUCCESS;
  
  if(alength <= RxBufferSize)
  {
    if(HAL_UART_Receive(&UartWiFiHandle, (uint8_t *)USART_RxBuffer, alength,5000)!= HAL_OK)
      {
        Error_Handler();
        return WiFi_HAL_UART_ERROR;
      }
    if(((strstr((const char *)&USART_RxBuffer,"OK"))) == NULL)
      {
        return WiFi_AT_CMD_RESP_ERROR;
      }
  }
  return status;  
}
/**** Wi-Fi indication call back *************/

#define DEBUG_WEAK
__weak void ind_wifi_warning(WiFi_Status_t warning_code)
{
#ifdef DEBUG_WEAK
  printf("WEAK ind_wifi_warning\r\n");
#endif /* DEBUG_WEAK */
}
	
__weak void ind_wifi_error(WiFi_Status_t error_code)
{
#ifdef DEBUG_WEAK
  printf("WEAK ind_wifi_error\r\n");
#endif /* DEBUG_WEAK */
}

__weak void ind_wifi_connection_error(WiFi_Status_t status_code)
{
#ifdef DEBUG_WEAK
  printf("WEAK ind_wifi_connection_error\r\n");
#endif /* DEBUG_WEAK */
}

__weak void ind_wifi_connected(void)
{
#ifdef DEBUG_WEAK
  printf("WEAK ind_wifi_connected\r\n");
#endif /* DEBUG_WEAK */
}
	
__weak void ind_wifi_ap_ready(void)
{
#ifdef DEBUG_WEAK
  printf("WEAK ind_wifi_ap_ready\r\n");
#endif /* DEBUG_WEAK */
}

__weak void ind_wifi_ap_client_joined(uint8_t * client_mac_address)
{
#ifdef DEBUG_WEAK
  printf("WEAK ind_wifi_ap_client_joined\r\n");
#endif /* DEBUG_WEAK */
}

__weak void ind_wifi_ap_client_left(uint8_t * client_mac_address)
{
#ifdef DEBUG_WEAK
  printf("WEAK ind_wifi_ap_client_left\r\n");
#endif /* DEBUG_WEAK */
}

__weak void ind_wifi_on(void)
{
#ifdef DEBUG_WEAK
  printf("WEAK ind_wifi_on\r\n");
#endif /* DEBUG_WEAK */
}

__weak void ind_wifi_packet_lost(WiFi_Status_t status_code)
{
#ifdef DEBUG_WEAK
  printf("WEAK ind_wifi_packet_lost\r\n");
#endif /* DEBUG_WEAK */
}

__weak void ind_wifi_gpio_changed(void)
{
#ifdef DEBUG_WEAK
  printf("WEAK ind_wifi_gpio_changed\r\n");
#endif /* DEBUG_WEAK */
}

__weak void ind_wifi_socket_data_received(uint8_t socket_id, uint8_t * data_ptr, uint32_t message_size, uint32_t chunk_size)
{
#ifdef DEBUG_WEAK
  printf("WEAK ind_wifi_socket_data_received\r\n");
#endif /* DEBUG_WEAK */
}

__weak void ind_wifi_socket_client_remote_server_closed(uint8_t * socketID)
{
#ifdef DEBUG_WEAK
  printf("WEAK ind_wifi_socket_client_remote_server_closed\r\n");
#endif /* DEBUG_WEAK */
}

__weak void ind_wifi_socket_server_data_lost(void)
{
#ifdef DEBUG_WEAK
  printf("WEAK ind_wifi_socket_server_data_lost\r\n");
#endif /* DEBUG_WEAK */
}

__weak void ind_socket_server_client_joined(void)
{
#ifdef DEBUG_WEAK
  printf("WEAK ind_socket_server_client_joined\r\n");
#endif /* DEBUG_WEAK */
}

__weak void ind_socket_server_client_left(void)
{
#ifdef DEBUG_WEAK
  printf("WEAK ind_socket_server_client_left\r\n");
#endif /* DEBUG_WEAK */
}

__weak void ind_wifi_http_data_available(uint8_t * data_ptr,uint32_t message_size)
{
#ifdef DEBUG_WEAK
  printf("WEAK ind_wifi_http_data_available\r\n");
#endif /* DEBUG_WEAK */
}

__weak void ind_wifi_file_data_available(uint8_t * data_ptr)
{
#ifdef DEBUG_WEAK
  printf("WEAK ind_wifi_file_data_available\r\n");
#endif /* DEBUG_WEAK */
}
__weak void ind_wifi_resuming(void)
{
#ifdef DEBUG_WEAK
  printf("WEAK ind_wifi_resuming\r\n");
#endif /* DEBUG_WEAK */
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

/******************* (C) COPYRIGHT 2015 STMicroelectronics *****END OF FILE****/

