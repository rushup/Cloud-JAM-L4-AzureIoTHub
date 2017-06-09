  /**
  ******************************************************************************
  * @file    wifi_module.h
  * @author  Central LAB 
  * @version V2.0.1
  * @date    17-May-2016
  * @brief   Header file for Wi-Fi module
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __WIFI_MODULE_H
#define __WIFI_MODULE_H

#ifdef __cplusplus
 extern "C" {
#endif
   
/* Includes ------------------------------------------------------------------*/
#include "stm32_spwf_wifi.h"
#include "wifi_const.h"   
#include "wifi_interface.h"
#include "event_buffer.h"
#include "ring_buffer.h"

/** @addtogroup MIDDLEWARES
* @{
*/ 


/** @addtogroup  NUCLEO_WIFI_MODULE 
  * @brief Wi-Fi_driver modules
  * @{
  */ 


/** @addtogroup NUCLEO_WIFI_MODULE_Private_Macros
  * @{
  */
   
//#define USART3_INT_MODE
#define USART3_POLLING_MODE
 /**
  * @}
  */


/** @addtogroup NUCLEO_WIFI_MODULE_Private_Variables
  * @{
  */
/* Private variables ---------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/

/******* Wi-Fi Configuration Setting Parameters *****************/
    
/**
  * @brief  WiFi config values structure definition
  */    
typedef struct
{
  uint8_t blink_led;
  char* console1_speed;
  char* wifi_ssid;
  char* Wifi_Sec_key;
  uint8_t wifi_mode;
  uint8_t sleep_enabled;
  uint8_t standby_enabled;
  uint8_t standby_time;
  uint8_t wifi_powersave;
  uint8_t wifi_operational_mode;
  uint8_t wifi_listen_interval;
  uint8_t wifi_beacon_wakeup;
  uint8_t wifi_priv_mode;
  uint8_t ip_use_dhcp;
  uint8_t ip_use_httpd;
  char* ip_hostname;
  uint8_t* IBSS_IP_Addr;
  uint8_t* IBSS_DefaultGateway;
  uint8_t* IBSS_IP_DNS_Addr;
  uint8_t* IBSS_IP_Mask;  
  char* ip_ipaddr;
}WiFi_Config_HandleTypeDef;


/**
  * @brief  WiFi control variables structure definition
  */ 
#pragma pack(1)
typedef struct
{
  wifi_bool switch_by_default_to_command_mode;
  wifi_bool queue_wifi_wind_message;
  wifi_bool http_req_pending;
  wifi_bool WiFi_Configuration_Done;
  wifi_bool stop_event_dequeue;
  wifi_bool Standby_Timer_Running;
  wifi_bool trigger_wakeup_callback;
  wifi_bool Deep_Sleep_Enabled;
  wifi_bool Standby_Enabled;
  wifi_bool Scan_Ongoing;
  wifi_bool AT_Cmd_Ongoing;
  wifi_bool AT_Cmd_Processing;
  wifi_bool Uartx_Rx_Processing;
  wifi_bool Client_Connected;
  wifi_bool Client_Disconnected;
  wifi_bool start_sock_read;
  wifi_bool enable_receive_data_chunk;
  wifi_bool data_pending_sockD;
  wifi_bool enable_sock_read;
  wifi_bool enable_query;
  wifi_bool enable_fw_update_read;
  wifi_bool Q_Contains_Message;
  wifi_bool enable_receive_http_response;
  wifi_bool enable_receive_file_response;
  wifi_bool enable_receive_wifi_scan_response;
  wifi_bool prevent_push_WIFI_event;
  wifi_bool event_deQ_x_wind64;
  wifi_bool do_not_reset_push_WIFI_event;
  wifi_bool message_pending;
  wifi_bool Pending_SockON_Callback;
  wifi_bool Pending_SockD_Callback;
  wifi_bool SockON_Server_Closed_Callback;
  wifi_bool Client_Socket_Close_Cmd;
  wifi_bool standby_resume_callback;
  wifi_bool HTTP_Data_available;
  wifi_bool FILE_Data_available;
  wifi_bool resume_receive_data;
  wifi_bool Deep_Sleep_Timer;
  wifi_bool Deep_Sleep_Callback;
  wifi_bool enable_timeout_timer;
  wifi_bool enable_sock_data;
  wifi_bool enable_SockON_Server_Closed_Callback; //enable SockON_Server_Closed_Callback in case of WIND:58, not when user requests for socket close.
}WiFi_Control_Variables_t;
#pragma pack()

/**
  * @brief  WiFi mode enumeration definition
  */ 
typedef enum 
{
  WiFi_IDLE_MODE =0,
  WiFi_STA_MODE,
  WiFi_IBSS_MODE,
  WiFi_MiniAP_MODE
} WiFi_Mode_TypeDef;

/********** Wi-Fi Indications*************/


/**
  * @brief  WiFi wind state structure definition
  */ 
#pragma pack(1) 
typedef struct 
{
  wifi_bool WiFiHWFailure;
  wifi_bool HardFault;
  wifi_bool StackOverflow;
  wifi_bool MallocFailed;
  wifi_bool InitFailure;
  wifi_bool StartFailed;
  wifi_bool PS_Mode_Failure;
  wifi_bool HeapTooSmall;
  wifi_bool WiFiSignalLOW;
  wifi_bool WiFiUp;
  wifi_bool WiFiStarted_MiniAPMode;
  wifi_bool WiFiAPClientJoined;
  wifi_bool WiFiAPClientLeft;
  wifi_bool WiFiException;  
  wifi_bool WiFiHWStarted;
  wifi_bool WiFiPowerDown;
  wifi_bool WiFiDeauthentication;
  
  /*Wifi Connection Errors*/
  wifi_bool WiFiJoinFailed;
  wifi_bool WiFiScanBlewUp;
  wifi_bool WiFiScanFailed;
  wifi_bool WiFiDeAuth;
  wifi_bool WiFiDisAssociation;
  
  /*Wifi packet lost INDs*/
  wifi_bool WiFiUnHandledInd;
  wifi_bool WiFiRXMgmt;
  wifi_bool WiFiRXData;
  wifi_bool WiFiRxUnk;   
  wifi_bool WiFiSockdDataLost;
  
} WiFi_WIND_State_TypeDef;
#pragma pack()


/**
  * @brief  WiFi WIND indication enumeration definition
  */ 
typedef enum {
  Console_Active          = 0,
  Poweron                 = 1,
  WiFi_Reset,
  Watchdog_Running,
  Heap_Too_Small,
  WiFi_Hardware_Dead      = 5,
  Watchdog_Terminating,
  SysTickConfigure,
  Hard_Fault              = 8,   
  StackOverflow,
  MallocFailed,
  Error,
  WiFi_PS_Mode_Failure    = 12,
  CopyrightInfo,
  WiFi_BSS_Regained       = 14,
  WiFi_Signal_LOW         = 15,
  WiFi_Signal_OK          = 16,
  FW_update               = 17,
  Encryption_key_Not_Recognized,
  WiFi_Join               = 19,
  JOINFAILED              = 20,
  WiFi_Scanning           = 21,
  SCANBLEWUP,
  SCANFAILED,
  WiFi_Up                 = 24,
  WiFi_Association_Successful   = 25,
  WiFi_Started_MiniAP_Mode      = 26,
  Start_Failed                  = 27,
  WiFi__MiniAP_Associated       = 28,
  WiFi_BSS_LOST                 = 30,
  WiFi_EXCEPTION          = 31,    
  WiFi_Hardware_Started   = 32,
  WiFi_NETWORK_LOST,
  WiFi_Unhandled_Event,
  Scan_Complete           = 35,
  WiFi_UNHANDLED_IND,
  WiFi_UNHANDLED,
  WiFi_Powered_Down,
  WiFi_MiniAP_Mode        = 39,
  WiFi_Deauthentication   = 40,     
  WiFi_Disassociation,
  RX_MGMT,
  RX_DATA,
  RX_UNK,
  DOT11_AUTHILLEGAL,
  Creating_PSK            = 46,  
  WPA_Terminated          = 49,  
  WPA_Supplicant_Failed,
  WPA_Handshake_Complete  = 51,
  GPIO_line,
  Wakeup,
  Factory_debug,
  SockON_Data_Pending           = 55,
  Remote_Configuration          = 57,
  SockON_Server_Socket_Closed   = 58,
  In_Command_Mode         = 59,
  In_Data_Mode            = 60,
  Incoming_socket_client  = 61,
  Outgoing_socket_client  = 62,
  SockD_Dropping_Data     = 63,
  SockD_Pending_Data      = 64,
  Low_Power_Mode_Enabled  = 66,
  Going_Into_Standby      = 67,
  Resuming_From_Standby   = 68,
  Going_Into_DeepSleep    = 69,
  Resuming_From_DeepSleep = 70,
  WiFi_MiniAP_Disassociated     = 72,
  Rejected_Found_Network  = 74,
  Undefine_Indication     = 0xFF
} WiFi_Indication_t;

typedef enum WiFi_Indication_t WiFi_Indication;

/**
  * @brief  WiFi power state enumeration definition
  */ 
typedef enum
{ 
  Active_State,
  PowerSave_State,    
  Sleep_State=3,
  StandBy_State=4
} WiFi_Power_State_t;

/**
  * @brief Structure definition containing instances of other structures
  */ 
typedef struct
{
  buffer_t big_buff;
  wifi_event_buffer event_buff;
  wifi_event_TypeDef wifi_event;
  wifi_event_TypeDef * DeQed_wifi_event;
  HAL_StatusTypeDef receive_status;
  GPIO_InitTypeDef GPIO_InitStruct;
}wifi_instances_t;


/**
  * @brief  WiFi control variables structure definition
  */ 
typedef struct
{
  uint8_t  Socket_Open_ID;
  uint8_t  sockon_query_id;
  uint8_t  sockon_id_user;
  uint8_t  user_scan_number;
  uint8_t  curr_sockID;
  uint8_t  closed_socket_id;
  uint8_t  remote_socket_closed_id;
  uint8_t  client_socket_close_id;
  uint8_t  gpio_value;
  uint8_t  gpio_dir;
  uint8_t  no_of_open_client_sockets;
  uint8_t * curr_hostname;
  uint8_t * curr_path;
  uint8_t * curr_pURL;
  uint8_t * curr_protocol;
  uint8_t * curr_filename;
  uint8_t * temp;       //pop buffer temporary
  
  char * curr_data;
  
  uint16_t curr_DataLength;
  
  uint32_t wind64_DQ_wait;
  uint32_t UserDataBuff_index;
  uint32_t Socket_Data_Length;
  uint32_t SockON_Data_Length;
  uint32_t number_of_bytes;
  uint32_t interim_number_of_bytes;
  uint32_t sock_total_count;
  uint32_t chunk_size;
  uint32_t message_size;
  uint32_t pop_queue_length;
  uint32_t pop_buffer_size;
  uint32_t last_process_buffer_index;
  uint32_t epoch_time;
  uint32_t curr_port_number;
  uint32_t sleep_count;
  uint32_t standby_time;
  uint32_t timeout_tick;
  uint8_t scanned_ssids;
  uint8_t get_cfg_value[64];
  uint8_t uart_byte[1];              /* Used to store one byte that is popped from the ring buffer */
  uint8_t client_MAC_address[17];    //current client MAC address store
  WiFi_Status_t AT_RESPONSE;

  /* Keeps a track of the socket ID that needs to be closed.
     Each array element depicts one socket(true = socket needs to be closed) */
  wifi_bool socket_close_pending[8];
}WiFi_Counter_Variables_t;

/**
  * @brief  All __IO type variable structure definition
  */ 
typedef struct
{
  __IO wifi_bool sock_read_ongoing;
  __IO wifi_bool prevent_push_OK_event;
  __IO wifi_bool Timer_Running;
  __IO wifi_bool enable_dequeue;
  __IO wifi_bool WiFi_Enabled;
  __IO wifi_bool data_mode;
  __IO wifi_bool command_mode;
  __IO uint32_t tickcount;
  __IO uint8_t wifi_ready;       //Set once if wifi is ready for first time
  __IO wifi_bool client_socket_close_ongoing;
  __IO uint32_t WIND64_count;
  __IO wifi_bool AT_Response_Received;

  __IO ITStatus UartReady;
  __IO ITStatus Uart2Ready;
  __IO ITStatus TxUartReady;
  __IO WiFi_WIND_State_TypeDef WiFi_WIND_State; //describes the current WIND number in processing
} IO_status_flag_typedef;


#ifdef WIFI_USE_VCOM
void console_input(void);
#endif

void PowerUp_WiFi_Module(void);
void WiFi_Module_Init(void);
void WiFi_Application(void);

/******* Wi-Fi AT CMD SET ****************/
WiFi_Status_t Attention_Cmd(void);
WiFi_Status_t USART_Transmit_AT_Cmd(uint16_t size);
WiFi_Status_t USART_Receive_AT_Resp(void);
WiFi_Status_t Save_Current_Setting(void);
WiFi_Status_t Restore_Default_Setting(void);
WiFi_Status_t SET_SSID(char* ssid);
WiFi_Status_t GET_SSID(void);
WiFi_Status_t SET_Configuration_Value(char* sVar_name,uint32_t aValue);
WiFi_Status_t GET_Configuration_Value(char* sVar_name,uint32_t *aValue);
WiFi_Status_t SET_Configuration_Addr(char* sVar_name,char* addr);
WiFi_Status_t Display_Help_Text(void);
WiFi_Status_t SET_Power_State(WiFi_Power_State_t state);
WiFi_Status_t Read_WiFi_SSID(char *string);
WiFi_Status_t Open_Serial_Port(void);
WiFi_Status_t WaitForResponse(uint16_t alength);
WiFi_Status_t config_init_value(char* sVar_name,uint8_t aValue);
WiFi_Status_t config_init_addr(char* sVar_name,char* addr);
void Process_Wind_Indication(uint8_t *process_buff_ptr);
void Process_DeQed_Wind_Indication(wifi_event_TypeDef * L_DeQued_wifi_event);
void Reset_AT_CMD_Buffer(void);
void USART2_SendBuffer(USART_TypeDef* USARTx, uint8_t *pData, uint8_t length);

char* Delete_Colon(char* );
WiFi_Status_t Read_WiFi_Mode(char *string);
WiFi_Status_t Read_WiFi_SecKey(char *string);

WiFi_Status_t Write_WiFi_SSID(char *string);
WiFi_Status_t Write_WiFi_SecKey(char *string);
WiFi_Status_t SET_WiFi_SecKey(char* seckey);
void PrintErrorMsg (void);
void Print_Msg(char * msgBuff,uint8_t length);
void Error_Handler(void);
void WiFi_Receive_Indication_Msg(void);
char *search_buffer(char *pSourceBuff, uint16_t sourceBuffLen, char *pSearchStringBuff, uint16_t seartchStringLen);

void ResetBuffer(void);
void Start_Timer(void);
void Stop_Timer(void);
void Request_Time_Out(void);
void Start_DeepSleep_Timer(void);
void Stop_DeepSleep_Timer(void);

void HTTP_Read_Data(void);
WiFi_Status_t Socket_Read(uint16_t DataLength);
void Read_Socket_Data(void);
void Socket_Pending_Data(void);
void WiFi_switch_to_command_mode(void);
void WiFi_switch_to_data_mode(void);
void WiFi_Configuration(void);
void Set_WiFi_Counter_Variables(void);
void Set_WiFi_Control_Variables(void);
void WiFi_Variables_Init(void);

void Receive_Data(void);
void Process_Buffer(uint8_t * ptr);
void Process_WiFi(void);
void Stop_Dequeue(void);
void Resume_Dequeue(void);
void wait_for_command_mode(void);
void Wifi_SysTick_Isr(void);
void RX_EXTI_Isr(uint16_t GPIO_Pin);
void Wifi_TIM_Handler(TIM_HandleTypeDef *htim);
void Queue_Http_Event(uint8_t * hostname, uint8_t * path, uint32_t port_number,uint8_t * pURL_path);
void Queue_Client_Write_Event(uint8_t sock_id, uint16_t DataLength, char * pData);
void Queue_Wifi_FW_Update_Event(uint8_t * hostname, uint8_t * filename_path, uint32_t port_number);
void Queue_Wifi_File_Event(uint8_t * pHostName, uint8_t * pFileName, uint32_t port_number);
void Queue_Client_Open_Event(uint8_t * hostname, uint32_t port_number, uint8_t * protocol);
void Queue_Client_Close_Event(uint8_t sock_id);
void Wait_For_Sock_Read_To_Complete(void);
void WiFi_HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartHandleArg);
void WiFi_HAL_UART_TxCpltCallback(UART_HandleTypeDef *UartHandleArg);
void WiFi_HAL_UART_ErrorCallback(UART_HandleTypeDef *UartHandle);

/*----------------------------- Private variables -------------------------------------*/

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
  
#ifdef __cplusplus
  }
#endif
#endif  /* __WIFI_MODULE_H */
