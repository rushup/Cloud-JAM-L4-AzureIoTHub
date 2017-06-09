/**
 ******************************************************************************
 * @file    platform_STM32Cube.c
 * @author  Central LAB
 * @version V3.0.0
 * @date    21-April-2017
 * @brief   Platform specific adapter
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT(c) 2017 STMicroelectronics</center></h2>
 *
 * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *        http://www.st.com/software_license_agreement_liberty_v2
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

#include <stdlib.h>
#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include "STM32CubeInterface.h"
#include "MetaDataManager.h"
#include "azure_c_shared_utility/crt_abstractions.h"
#include "azure_c_shared_utility/platform.h"
#include "TLocalBuffer.h"
#include "OTA.h"
#include "AzureClient_mqtt_DM_TM.h"

#include "azure_c_shared_utility/tls_config.h"
#include "azure_c_shared_utility/tlsio_mbedtls.h"
#include "azure_c_shared_utility/tlsio.h"

#ifdef AZURE_ENABLE_REGISTRATION
#include "RegistrationAgent.h"
#endif /* AZURE_ENABLE_REGISTRATION */

/* M24SR GPIO mapping -------------------------------------------------------------------------*/
#define M24SR_SDA_PIN        GPIO_PIN_9
#define M24SR_SDA_PIN_PORT   GPIOB
#define M24SR_SCL_PIN        GPIO_PIN_8
#define M24SR_SCL_PIN_PORT   GPIOB
#define M24SR_GPO_PIN        GPIO_PIN_6
#define M24SR_GPO_PIN_PORT   GPIOA
#define M24SR_RFDIS_PIN      GPIO_PIN_7
#define M24SR_RFDIS_PIN_PORT GPIOA

/* Exported variables ---------------------------------------------------------*/
TargetFeatures_t TargetBoardFeatures;

/* Socket with mbedTLS ... first for Telemetry, Second one for FOTA */
uint8_t SocketHandleAzure[2]={0xFF,0xFF};

TIM_HandleTypeDef    TimCCHandle;

volatile uint32_t ButtonPressed =0;
volatile uint32_t MEMSInterrupt =0;
volatile uint32_t SendData =0;

uint8_t macaddstart[32];

#if defined (__CC_ARM)
/* For avoiding Keil error */
MDM_knownOsxLicense_t known_OsxLic[]={
  {OSX_END,"LAST",""}/* THIS MUST BE THE LAST ONE */
};
#endif /* (__CC_ARM) */

/* Imported variables --------------------------------------------------------*/
extern const char certificates[];

/* Local variables ---------------------------------------------------------*/

static __IO wifi_state_t wifi_state;
static uint32_t FullOTASize=0;

static int32_t ConnectedToNTP=0;
static int32_t NTPServerClosed=0;


static AZURE1_OTA OTAStatus=OTA_STATUS_NULL;

DEFINE_ENUM_STRINGS(AZURE1_ERROR, AZURE1_ERROR_VALUES)
 
/* Local defines -------------------------------------------------------------*/
//#define DEBUG_OTA_RECEIVE

//2kHz/0.5 For Sensors Data data@2Hz
#define DEFAULT_TIM_CC1_PULSE  4000

/* Defines related to Clock configuration */    
#define RTC_ASYNCH_PREDIV  0x7F   /* LSE as RTC clock */
#define RTC_SYNCH_PREDIV   0x00FF /* LSE as RTC clock */

#define HTTP_GET_REQUEST 0
#define HTTP_HEAD_REQUEST 1

#define FOTA_CHUNK_SIZE 256

/* Local function prototypes --------------------------------------------------*/
static void STM32_Error_Handler(void);
static void Init_MEM1_Sensors(void);
static void SystemClock_Config(void);
static void InitTimers(void);
static void InitRTC(void);
#ifdef USE_STM32L4XX_NUCLEO
static uint32_t GetPage(uint32_t Address);
static uint32_t GetBank(uint32_t Address);
#endif /* USE_STM32L4XX_NUCLEO */

static void onSendCompleteFOTA(void* context, IO_SEND_RESULT send_result);
static void onOpenCompleteFOTA(void* context, IO_OPEN_RESULT open_result);
static void onBytesReceivedFOTA(void* context, const unsigned char* buffer, size_t size);
static void onIoErrorFOTA(void* context);
static void onCloseCompleteOTA(void* context);

/**
  * @brief  Function for Initializing the platform
  * @param  None
  * @retval int Ok/Error (0/1)
  */
int platform_init(void)
{
  WIFI_CredAcc_t WIFI_CredAcc;
  wifi_config config;
  int32_t CounterButtonPress=0;
  WiFi_Status_t status = WiFi_MODULE_SUCCESS;

  /* Init Platform */
  HAL_Init();

  /* Configure the System clock */
  SystemClock_Config();

#ifdef AZURE_ENABLE_PRINTF
  /* UART Initialization */
  if(UART_Global_Init()!=HAL_OK) {
    STM32_Error_Handler();
  } else {
    AZURE_PRINTF("UART Initialized\r\n");
  }
#endif /* AZURE_ENABLE_PRINTF */
  /* I2C Initialization */
  if(I2C_Global_Init()!=HAL_OK) {
    STM32_Error_Handler();
  } else {
    AZURE_PRINTF("I2C  Initialized\r\n");
  }

  /* Initialize button */
  BSP_PB_Init(BUTTON_KEY, BUTTON_MODE_EXTI);

  /* Initialize LED */
  BSP_LED_Init(LED2);

  AZURE_PRINTF("\r\nSTMicroelectronics %s:\r\n"
         "\tVersion %c.%c.%c\r\n"
        "\tSTM32L476RG-Nucleo board"
          "\r\n",
          AZURE_PACKAGENAME,
          AZURE_VERSION_MAJOR,AZURE_VERSION_MINOR,AZURE_VERSION_PATCH);

  AZURE_PRINTF("\tAzure SDK Version %s\r\n",IoTHubClient_GetVersionString());

  /* Reset all the Target's Features */
  memset(&TargetBoardFeatures, 0, sizeof(TargetFeatures_t));
  /* Discovery and Intialize all the Target's Features */
  Init_MEM1_Sensors();

  AZURE_PRINTF("\t(HAL %ld.%ld.%ld_%ld)\r\n"
        "\tCompiled %s %s"
#if defined (__IAR_SYSTEMS_ICC__)
        " (IAR)\r\n"
#elif defined (__CC_ARM)
        " (KEIL)\r\n"
#elif defined (__GNUC__)
        " (openstm32)\r\n"
#endif
           ,
           HAL_GetHalVersion() >>24,
          (HAL_GetHalVersion() >>16)&0xFF,
          (HAL_GetHalVersion() >> 8)&0xFF,
           HAL_GetHalVersion()      &0xFF,
         __DATE__,__TIME__);

  AZURE_PRINTF("\tOTA with one HTTP HEAD + Multiple HTTP one GET of %dBytes\r\n",FOTA_CHUNK_SIZE);

  if(TargetBoardFeatures.HWAdvanceFeatures) {
    InitHWFeatures();
  }
  
  /* Check the BootLoader Compliance */
  if(CheckBootLoaderCompliance()) {
    AZURE_PRINTF("BootLoader Compliant with FOTA procedure\r\n");
  } else {
    AZURE_PRINTF("Err: BootLoader NOT Compliant with FOTA procedure\r\n");
    AzureExit(AZURE_ERR_BL_COMPLIANCE);
  }

  /* Set Full Scale to +/-2g */
  (TargetBoardFeatures.SnsAltFunc ? BSP_ACCELERO_Set_FS_Value_IKS01A2 : BSP_ACCELERO_Set_FS_Value)(TargetBoardFeatures.HandleAccSensor,2.0f);

  /* initialize timers */
  InitTimers();
  AZURE_PRINTF("Init Application's Timers\r\n");

  /* Initialize random number generartion */
#ifdef USE_MBED_TLS
  TargetBoardFeatures.RngHandle.Instance = RNG;
  HAL_RNG_Init(&TargetBoardFeatures.RngHandle);
  AZURE_PRINTF("Init Random Number Generator\r\n");
#endif /* USE_MBED_TLS*/
    
  /* Enabling HW Features... FreeFall */
  EnableHWFreeFall();

  /* Initializes Timers for WIFI */
  Timer_Config();
  AZURE_PRINTF("Init WIFI's Timers\r\n");

  /* Initializes UART1 for WIFI */
  UART_Configuration(115200);
  AZURE_PRINTF("Init WIFI's UART1\r\n");

  /* Set the WIFI Credential */
  {
    char console_input[40];
    MDM_knownGMD_t known_MetaData[]={
      {GMD_WIFI,sizeof(WIFI_CredAcc_t)},
      {GMD_END    ,0}/* THIS MUST BE THE LAST ONE */
    };

    AZURE_PRINTF("--------------------------\r\n");
    AZURE_PRINTF("|    WIFI Credential     |\r\n");
    AZURE_PRINTF("--------------------------\r\n");
    
    /* Check the MetaDataManager */
    InitMetaDataManager((void *)&known_MetaData,MDM_DATA_TYPE_GMD,NULL);

    /* Recall the WIFI Credential saved */
    MDM_ReCallGMD(GMD_WIFI,(void *)&WIFI_CredAcc);

    if(WIFI_CredAcc.ssid[0]==0) {
      sprintf(WIFI_CredAcc.ssid,"%s",AZURE_DEFAULT_SSID);
      sprintf(WIFI_CredAcc.seckey,"%s",AZURE_DEFAULT_SECKEY);
      WIFI_CredAcc.mode = AZURE_DEFAULT_PRIV_MODE;
      AZURE_PRINTF("\tDefault SSID   : %s\r\n",WIFI_CredAcc.ssid);
      AZURE_PRINTF("\tDefault PassWd : %s\r\n",WIFI_CredAcc.seckey);
      AZURE_PRINTF("\tDefault EncMode: %s\r\n",(WIFI_CredAcc.mode == None) ? "Open" : ((WIFI_CredAcc.mode == WEP) ? "WEP" : "WPA2/WPA2-Personal"));
    } else {
      AZURE_PRINTF("\tSaved SSID   : %s\r\n",WIFI_CredAcc.ssid);
      AZURE_PRINTF("\tSaved PassWd : %s\r\n",WIFI_CredAcc.seckey);
      AZURE_PRINTF("\tSaved EncMode: %s\r\n",(WIFI_CredAcc.mode == None) ? "Open" : ((WIFI_CredAcc.mode == WEP) ? "WEP" : "WPA2/WPA2-Personal"));
    }

    AZURE_PRINTF("\r\nWait 3 seconds for allowing User Button Control:");
    AZURE_PRINTF("\r\n\t - Press one time  for adding the WIFI credential from UART or NFC ");
    AZURE_PRINTF("\r\n\t - Press two times for adding the WIFI credential only from NFC\r\n");
    {
      int32_t CountOn,CountOff;
      for(CountOn=0;CountOn<3;CountOn++) {
        BSP_LED_On(LED2);
        for(CountOff=0;CountOff<10;CountOff++) {
          if(CountOff==2) {
            BSP_LED_Off(LED2);
          }
          /* Handle user button */
          if(ButtonPressed) {
            CounterButtonPress++;
            ButtonPressed=0;
          }
          HAL_Delay(100);
        }
      }
    }

    if(CounterButtonPress==1) {
      /* Enabling UART interaction */
      AZURE_PRINTF("\r\nDo you want to change them?(y/n)\r\n");
      scanf("%s",console_input);
      if(console_input[0]=='y') {
        int32_t ReadFromTerminal=1;

        /* Test if the NFC is present */
        if(TT4_Init() == SUCCESS) {
          AZURE_PRINTF("\tX-NUCLEO-NFC01A1 is present\r\n");
          AZURE_PRINTF("\tDo you want to read them from NFC?(y/n)\r\n");
          scanf("%s",console_input);
          if(console_input[0]=='y') {
            sWifiTokenInfo TokenInfo;
            if(TT4_ReadWifiToken(&TokenInfo) == SUCCESS) {
              ReadFromTerminal=0;

              sprintf(WIFI_CredAcc.ssid,"%s",TokenInfo.NetworkSSID);
              sprintf(WIFI_CredAcc.seckey,"%s",TokenInfo.NetworkKey);

              if (strcmp(TokenInfo.AuthenticationType,"NONE")==0) {
                WIFI_CredAcc.mode = None;
              } else if(strcmp(TokenInfo.AuthenticationType,"WEP")==0) {
                WIFI_CredAcc.mode = WEP;
              } else {
                WIFI_CredAcc.mode = WPA_Personal;
              }
            } else {
              AZURE_PRINTF("Err reading the WIFI's credentials from NFC\r\n");
              AZURE_PRINTF("Add them manually\r\n");
            }
          }
        }

        if(ReadFromTerminal==1) {
          AZURE_PRINTF("\r\nEnter the SSID:\r\n");
          scanf("%s",console_input);
          sprintf(WIFI_CredAcc.ssid,"%s",console_input);
          AZURE_PRINTF("\r\nEnter the PassWd:\r\n");
          scanf("%s",console_input);
          sprintf(WIFI_CredAcc.seckey,"%s",console_input);
          AZURE_PRINTF("\r\nEnter the encryption mode(0:Open, 1:WEP, 2:WPA2/WPA2-Personal):\r\n");
          scanf("%s",console_input);
          switch(console_input[0]){
            case '0':
              WIFI_CredAcc.mode = None;
            break;
            case '1':
              WIFI_CredAcc.mode = WEP;
            break;
            case '2':
              WIFI_CredAcc.mode = WPA_Personal;
            break;
            default:
              AZURE_PRINTF("\r\nWrong Entry. Priv Mode is not compatible\r\n");
              AzureExit(AZURE_ERR_WIFI_CRED);
          }
        }
        AZURE_PRINTF("\tNew SSID   : %s\r\n",WIFI_CredAcc.ssid);
        AZURE_PRINTF("\tNew PassWd : %s\r\n",WIFI_CredAcc.seckey);
        AZURE_PRINTF("\tNew EncMode: %s\r\n",(WIFI_CredAcc.mode == None) ? "Open" : ((WIFI_CredAcc.mode == WEP) ? "WEP" : "WPA2/WPA2-Personal"));

        /* Save the WIFI Credential on MetaDataManager */
        MDM_SaveGMD(GMD_WIFI,(void *)&WIFI_CredAcc);

        /* Save the MetaDataManager in Flash */
        EraseMetaDataManager();
        SaveMetaDataManager();

        AZURE_PRINTF("-----------------------------\r\n");
      }
      AZURE_PRINTF("\r\n");
    } else if(CounterButtonPress==2) {
      int32_t Error=0;
      /* Force Read from NFC */
      if(TT4_Init() == SUCCESS) {
        AZURE_PRINTF("\tX-NUCLEO-NFC01A1 is present\r\n");
        sWifiTokenInfo TokenInfo;
        if(TT4_ReadWifiToken(&TokenInfo) == SUCCESS) {
          sprintf(WIFI_CredAcc.ssid,"%s",TokenInfo.NetworkSSID);
          sprintf(WIFI_CredAcc.seckey,"%s",TokenInfo.NetworkKey);

          if (strcmp(TokenInfo.AuthenticationType,"NONE")==0) {
            WIFI_CredAcc.mode = None;
          } else if(strcmp(TokenInfo.AuthenticationType,"WEP")==0) {
            WIFI_CredAcc.mode = WEP;
          } else {
            WIFI_CredAcc.mode = WPA_Personal;
          }
          AZURE_PRINTF("\tNew SSID   : %s\r\n",WIFI_CredAcc.ssid);
          AZURE_PRINTF("\tNew PassWd : %s\r\n",WIFI_CredAcc.seckey);
          AZURE_PRINTF("\tNew EncMode: %s\r\n",(WIFI_CredAcc.mode == None) ? "Open" : ((WIFI_CredAcc.mode == WEP) ? "WEP" : "WPA2/WPA2-Personal"));

          /* Save the WIFI Credential on MetaDataManager */
          MDM_SaveGMD(GMD_WIFI,(void *)&WIFI_CredAcc);

          /* Save the MetaDataManager in Flash */
          EraseMetaDataManager();
          SaveMetaDataManager();

          AZURE_PRINTF("-----------------------------\r\n");
        } else {
          Error=1;
        }
      } else {
        Error=1;
      }
      if(Error) {
        AZURE_PRINTF("Err reading the WIFI's credentials from NFC\r\n");
        AzureExit(AZURE_ERR_NFC);
      }
    }
  }

  /* Initialize the WIFI module */
  wifi_state = wifi_state_idle;
  memset(&config,0,sizeof(wifi_config));
  config.power=wifi_active;
  config.power_level=high;
  config.dhcp=on;//use DHCP IP address
  status = wifi_init(&config);
  if(status!=WiFi_MODULE_SUCCESS) {
    AZURE_PRINTF("Err in Config\r\n");
    AzureExit(AZURE_ERR_WIFI);
  } else {
    AZURE_PRINTF("WIFI module Configured\r\n");
  }

  GET_Configuration_Value("nv_wifi_macaddr",(uint32_t *)macaddstart);
  macaddstart[17]='\0';
  AZURE_PRINTF("WiFi MAC Address is: %s\r\n",macaddstart);

  status = wifi_connect(WIFI_CredAcc.ssid, WIFI_CredAcc.seckey, WIFI_CredAcc.mode);
  if(status!=WiFi_MODULE_SUCCESS) {
    AZURE_PRINTF("Err Connecting to WIFI\r\n");
    AzureExit(AZURE_ERR_WIFI_CON);
  }

  /* Wait WIFI Connection */
  while(wifi_state != wifi_state_connected) {
    HAL_Delay(100);
  }  
  
  /* initialize Real Time Clock */
  InitRTC();

  /* Check if Data stored in BackUp register0: No Need to reconfigure RTC */
  if(HAL_RTCEx_BKUPRead(&TargetBoardFeatures.RtcHandle, RTC_BKP_DR0) != 0x32F2){
    int32_t NTP_OK=0;
    while(!NTP_OK) {
      /* Configure RTC Calendar */
      char ipAddress[] =  "time-d.nist.gov";
      //char ipAddress[] =  "time-c.nist.gov";
      uint8_t tcpPort  =  37;
      uint8_t socketHandle;
      status = wifi_socket_client_open((uint8_t*)ipAddress, tcpPort, "t", &socketHandle);
      if(status!=WiFi_MODULE_SUCCESS) {
        AZURE_PRINTF("Err opening socket with NTP server\r\n");
      } else {
        AZURE_PRINTF("WiFi opened socket with NTP server [%s]\r\n",ipAddress);
        ConnectedToNTP=1;
        NTPServerClosed=1;

        // Wait to receive data
        while(ConnectedToNTP) {
          HAL_Delay(100);
        }

        // Wait socket close
        AZURE_PRINTF("Wait NTP server to close \r\n");
        while(NTPServerClosed) {
          HAL_Delay(100);
        }
        //status = wifi_socket_client_close(socketHandle);
        //if(status!=WiFi_MODULE_SUCCESS) {
        //  AZURE_PRINTF("Err closing socket with NTP server\r\n");
        //} else {
        AZURE_PRINTF("Socket closed with NTP server\r\n");
        NTP_OK = 1;
        //}
      }

      if(!NTP_OK) {
        AZURE_PRINTF("I'll try again to connect to NTP server in 2 seconds\r\n");
        HAL_Delay(2000);
      }
    }
  } else {
    uint8_t aShowTime[50] = {0};
    uint8_t aShowDate[50] = {0};
    RTC_CalendarShow(aShowTime,aShowDate);
    AZURE_PRINTF("Init Real Time Clock %s %s\r\n",aShowDate,aShowTime);
  }

  /* Init the Local Buffer for mbedTLS for Telemetry */
  LocalBufferInit(&localBufferReading[AZURE_SOCKET_TELEMETRY]);
  /* Init the Local Buffer for mbedTLS for Device Management */
  LocalBufferInit(&localBufferReading[AZURE_SOCKET_FOTA]);
  return 0;
}

/**
  * @brief  Function for signaling Error
  * @param  AZURE1_ERROR Value Error Type
  * @retval None
  */
void AzureExit(AZURE1_ERROR Value)
{
  AZURE_PRINTF("\r\nErr Type %s\r\n",ENUM_TO_STRING(AZURE1_ERROR,Value));
  AZURE_PRINTF("\tNecessity to restart the Board\r\n");
  /* Infinite led blinking */
  BSP_LED_Off(LED2);
  while(1) {
    int32_t Counter;
    /* Blinking Error Code... */
    for(Counter=0;Counter<Value;Counter++) {
      BSP_LED_On(LED2);
      HAL_Delay(200);
      BSP_LED_Off(LED2);
      HAL_Delay(400);
    }
    HAL_Delay(4000);
  }
}

/**
  * @brief  Function for starting the timer for sending the Telemetry data to IoT
  * @param  None
  * @retval None
  */
void StartTimer1(void) {
  /* Starting timer */
  if(HAL_TIM_OC_Start_IT(&TimCCHandle, TIM_CHANNEL_1) != HAL_OK){
    /* Starting Error */
    STM32_Error_Handler();
  }

  /* Set the new Capture compare value */
  {
    uint32_t uhCapture = __HAL_TIM_GET_COUNTER(&TimCCHandle);
    /* Set the Capture Compare Register value */
    __HAL_TIM_SET_COMPARE(&TimCCHandle, TIM_CHANNEL_1, (uhCapture + TargetBoardFeatures.TIM_CC1_Pulse));
  }

  AZURE_PRINTF("Channel 1 for Timer 1 started\r\n");
}

/**
  * @brief  Function for pausing the timer for sending the Telemetry data to IoT
  * @param  None
  * @retval None
  */
void StopTimer1(void) {
  /* Stop timer */
  if(HAL_TIM_OC_Stop_IT(&TimCCHandle, TIM_CHANNEL_1) != HAL_OK){
    /* Starting Error */
    STM32_Error_Handler();
  }
  AZURE_PRINTF("Channel 1 for Timer 1 stopped\r\n");
}

/**
  * @brief  Function called when the Socket is closed
  * @param  uint8_t* socketID pointer to the socket handle
  * @retval None
  */
void ind_wifi_socket_client_remote_server_closed(uint8_t * socketID)
{
  if(SocketHandleAzure[AZURE_SOCKET_FOTA]==(*socketID)) {
    if(OTAStatus != OTA_STATUS_END ) {
      AZURE_PRINTF("Err: Socket for WIFI closed before full OTA Trasmission\r\n");
      OTAStatus = OTA_STATUS_ERROR;
    } else {
      SocketHandleAzure[AZURE_SOCKET_FOTA]=0xFF;
    }
  } else {
	// waiting for NTP server to close
	if (NTPServerClosed)
		NTPServerClosed = 0;
    AZURE_PRINTF("WIFI closed =%x\r\n",* socketID);
  }
}

/**
 * @brief  WiFi callback for data received
 * @param  uint8_t pSocketHandle socket handle
 * @param  uint8_t * data_ptr pointer to buffer containing data received 
 * @param  uint32_t message_size number of bytes in message received
 * @param  uint32_t chunk_size numeber of bytes in chunk
 * @retval None
 */
void ind_wifi_socket_data_received(uint8_t pSocketHandle,uint8_t * data_ptr, uint32_t message_size, uint32_t chunk_size)
{
#if 0
  /* Just for Debug */
  AZURE_PRINTF("****** Received buffer for a socket...***** \r\n");
  AZURE_PRINTF("socket received =\r\n");
  AZURE_PRINTF("\tSock=%d\r\n",pSocketHandle);
  //AZURE_PRINTF("\tdata=%s\r\n",data_ptr);
  AZURE_PRINTF("\tmessage_size=%ld\r\n",message_size);
  AZURE_PRINTF("\tchunk_size=%ld\r\n",chunk_size);
#endif

  if(ConnectedToNTP==1) {
    if(message_size==4){
      time_t epochTimeToSetForSystem = SynchronizationAgentConvertNTPTime2EpochTime(data_ptr,message_size);
      if (TimingSystemSetSystemTime(epochTimeToSetForSystem)== 0){
        AZURE_PRINTF("Err Failed to set system time. \r\n");
      } else {
        AZURE_PRINTF("Set UTC Time: %s\r\n",(get_ctime(&epochTimeToSetForSystem)));
        ConnectedToNTP =0;
      }
    }
#ifdef AZURE_ENABLE_REGISTRATION
  } else if (RegistrationRequest==AGENT_START) {
    memcpy(BufferServiceAnswer,data_ptr,chunk_size);
    RegistrationRequest = AGENT_DATA;
#endif /* AZURE_ENABLE_REGISTRATION */
  } else if(SocketHandleAzure[AZURE_SOCKET_FOTA]==pSocketHandle) {
    /* Secure mbedTLS socket for FOTA */
    if(LocalBufferPushBuffer(&localBufferReading[AZURE_SOCKET_FOTA], (void*) data_ptr,chunk_size)==0) {
      AZURE_PRINTF("Err: for Pushing Buffer for FOTA\r\n");
    }
  } else if(SocketHandleAzure[AZURE_SOCKET_TELEMETRY]==pSocketHandle) {
    /* Secure mbedTLS socket for Telemetry */
    if(LocalBufferPushBuffer(&localBufferReading[AZURE_SOCKET_TELEMETRY], (void*) data_ptr,chunk_size)==0) {
      AZURE_PRINTF("Err: for Pushing Buffer for Telemetry\r\n");
    }
  }
}

/**
 * @brief  WiFi callback when it is connected to Access Point
 * @param  None
 * @retval None
 */
void ind_wifi_connected()
{
  AZURE_PRINTF("WiFi connected to AccessPoint\r\n");
  wifi_state = wifi_state_connected;
}

/**
 * @brief  WiFi callback when it is initilized
 * @param  None
 * @retval None
 */
void ind_wifi_on()
{
  AZURE_PRINTF("\r\n\nwifi started and ready...\r\n");
  wifi_state = wifi_state_ready;
}

/** @brief function for providing the default TLS I/O interface
 * @param None
 * @retval IO_INTERFACE_DESCRIPTION* Pointer to the default TLS I/O interface
 */
const IO_INTERFACE_DESCRIPTION* platform_get_default_tlsio(void)
{
#ifdef USE_MBED_TLS
  return tlsio_mbedtls_get_interface_description();
#else /* USE_MBED_TLS */
  return tlsio_STM32Cube_get_interface_description();
#endif /* USE_MBED_TLS */
}

/** @brief Function for de-initializing the platform
 * @param None
 * @retval None
 */
void platform_deinit(void)
{
  /* TODO: Insert here any platform specific one time de-initialization code like.
  This has to undo what has been done in platform_init. */
}


/** @brief Initialize all the MEMS1 sensors
 * @param None
 * @retval None
 */
static void Init_MEM1_Sensors(void)
{
  /* Accelero */
#ifndef IKS01A2
  /* Test if the board is IK01A1 */
  if (BSP_ACCELERO_Init_IKS01A1( ACCELERO_SENSORS_AUTO, &TargetBoardFeatures.HandleAccSensor ) == COMPONENT_OK) {
    AZURE_PRINTF("IKS01A1 board\n\r");
    AZURE_PRINTF("Ok Accelero Sensor\n\r");
    TargetBoardFeatures.SnsAltFunc = 0;
  } else {
#else /* IKS01A2 */
  {
#endif /* IKS01A2 */
    TargetBoardFeatures.SnsAltFunc = 1;
    if (BSP_ACCELERO_Init_IKS01A2( ACCELERO_SENSORS_AUTO, &TargetBoardFeatures.HandleAccSensor ) == COMPONENT_OK){
      AZURE_PRINTF("IKS01A2 board\n\r");
      AZURE_PRINTF("Ok Accelero Sensor\n\r");
    } else {
      AZURE_PRINTF("IKS01A2 or IKS01A1 board not present, Emulation enabled\n\r");
      TargetBoardFeatures.EmulateSns=1;
    }
  }

  if(!TargetBoardFeatures.EmulateSns) {
    /* DS3/DSM or DS0 */
    /* This section works with IKS01A1 or with IKS01A1/A2 Autodiscovery */
#ifndef IKS01A2
    if(!TargetBoardFeatures.SnsAltFunc){
      uint8_t WhoAmI;
      BSP_ACCELERO_Get_WhoAmI(TargetBoardFeatures.HandleAccSensor,&WhoAmI);
      if(LSM6DS3_ACC_GYRO_WHO_AM_I==WhoAmI) {
        AZURE_PRINTF("\tDS3 DIL24 Present\n\r");
        TargetBoardFeatures.HWAdvanceFeatures = 1;
      } else {
        TargetBoardFeatures.HWAdvanceFeatures = 0;
      }
    } else {
#else /* IKS01A2 */
   {
#endif /* IKS01A2 */
      TargetBoardFeatures.HWAdvanceFeatures = 1;
    }

    /* Gyro */
    if((TargetBoardFeatures.SnsAltFunc ? BSP_GYRO_Init_IKS01A2 : BSP_GYRO_Init)( GYRO_SENSORS_AUTO, &TargetBoardFeatures.HandleGyroSensor )==COMPONENT_OK){
      AZURE_PRINTF("Ok Gyroscope Sensor\n\r");
    } else {
      AZURE_PRINTF("Err Gyroscope Sensor\n\r");
      while(1);
    }

    if((TargetBoardFeatures.SnsAltFunc ? BSP_MAGNETO_Init_IKS01A2 : BSP_MAGNETO_Init)( MAGNETO_SENSORS_AUTO, &TargetBoardFeatures.HandleMagSensor )==COMPONENT_OK){
      AZURE_PRINTF("Ok Magneto Sensor\n\r");
    } else {
      AZURE_PRINTF("Err Magneto Sensor\n\r");
      while(1);
    }

    /* Humidity */
    if((TargetBoardFeatures.SnsAltFunc ? BSP_HUMIDITY_Init_IKS01A2 : BSP_HUMIDITY_Init)( HUMIDITY_SENSORS_AUTO, &TargetBoardFeatures.HandleHumSensor )==COMPONENT_OK){
      AZURE_PRINTF("Ok Humidity Sensor\n\r");
    } else {
      AZURE_PRINTF("Err Humidity Sensor\n\r");
    }

    /* Temperature1 */
    if((TargetBoardFeatures.SnsAltFunc ? BSP_TEMPERATURE_Init_IKS01A2 : BSP_TEMPERATURE_Init)( TEMPERATURE_SENSORS_AUTO, &TargetBoardFeatures.HandleTempSensors[TargetBoardFeatures.NumTempSensors] )==COMPONENT_OK){
       AZURE_PRINTF("Ok Temperature Sensor1\n\r");
       TargetBoardFeatures.NumTempSensors++;
    } else {
      AZURE_PRINTF("Err Temperature Sensor1\n\r");
    }

    /* Temperature2 */
#ifndef IKS01A2
    if((TargetBoardFeatures.SnsAltFunc ? BSP_TEMPERATURE_Init_IKS01A2 : BSP_TEMPERATURE_Init) (TargetBoardFeatures.SnsAltFunc ? LPS22HB_T_0: LPS25HB_T_0,&TargetBoardFeatures.HandleTempSensors[TargetBoardFeatures.NumTempSensors] )==COMPONENT_OK){
#else /* IKS01A2 */
    if(BSP_TEMPERATURE_Init_IKS01A2(LPS22HB_T_0,&TargetBoardFeatures.HandleTempSensors[TargetBoardFeatures.NumTempSensors] )==COMPONENT_OK){      
#endif /* IKS01A2 */
      AZURE_PRINTF("Ok Temperature Sensor2\n\r");
      TargetBoardFeatures.NumTempSensors++;
    } else {
      AZURE_PRINTF("Err Temperature Sensor2\n\r");
    }

    /* Pressure */
    if((TargetBoardFeatures.SnsAltFunc ? BSP_PRESSURE_Init_IKS01A2 : BSP_PRESSURE_Init)( PRESSURE_SENSORS_AUTO, &TargetBoardFeatures.HandlePressSensor )==COMPONENT_OK){
      AZURE_PRINTF("Ok Pressure Sensor\n\r");
    } else {
      AZURE_PRINTF("Err Pressure Sensor\n\r");
    }

    /*  Enable all the sensors */
    if(TargetBoardFeatures.HandleAccSensor) {
      if((TargetBoardFeatures.SnsAltFunc ? BSP_ACCELERO_Sensor_Enable_IKS01A2 : BSP_ACCELERO_Sensor_Enable)( TargetBoardFeatures.HandleAccSensor )==COMPONENT_OK) {
        AZURE_PRINTF("Enabled Accelero Sensor\n\r");
      }
    }

    if(TargetBoardFeatures.HandleGyroSensor) {
      if((TargetBoardFeatures.SnsAltFunc ? BSP_GYRO_Sensor_Enable_IKS01A2 : BSP_GYRO_Sensor_Enable)( TargetBoardFeatures.HandleGyroSensor )==COMPONENT_OK) {
        AZURE_PRINTF("Enabled Gyroscope Sensor\n\r");
      }
    }

    if(TargetBoardFeatures.HandleMagSensor) {
      if((TargetBoardFeatures.SnsAltFunc ? BSP_MAGNETO_Sensor_Enable_IKS01A2 : BSP_MAGNETO_Sensor_Enable)( TargetBoardFeatures.HandleMagSensor )==COMPONENT_OK) {
        AZURE_PRINTF("Enabled Magneto Sensor\n\r");
      }
    }

    if(TargetBoardFeatures.HandleHumSensor) {
      if((TargetBoardFeatures.SnsAltFunc ? BSP_HUMIDITY_Sensor_Enable_IKS01A2 : BSP_HUMIDITY_Sensor_Enable) ( TargetBoardFeatures.HandleHumSensor)==COMPONENT_OK) {
        AZURE_PRINTF("Enabled Humidity Sensor\n\r");
      }
    }

    if(TargetBoardFeatures.HandleTempSensors[0]){
      if((TargetBoardFeatures.SnsAltFunc ? BSP_TEMPERATURE_Sensor_Enable_IKS01A2 : BSP_TEMPERATURE_Sensor_Enable)( TargetBoardFeatures.HandleTempSensors[0])==COMPONENT_OK) {
        AZURE_PRINTF("Enabled Temperature Sensor1\n\r");
      }
    }

    if(TargetBoardFeatures.HandleTempSensors[1]){
      if((TargetBoardFeatures.SnsAltFunc ? BSP_TEMPERATURE_Sensor_Enable_IKS01A2 : BSP_TEMPERATURE_Sensor_Enable)( TargetBoardFeatures.HandleTempSensors[1])==COMPONENT_OK) {
        AZURE_PRINTF("Enabled Temperature Sensor2\n\r");
      }
    }

    if(TargetBoardFeatures.HandlePressSensor) {
      if((TargetBoardFeatures.SnsAltFunc ? BSP_PRESSURE_Sensor_Enable_IKS01A2 : BSP_PRESSURE_Sensor_Enable)( TargetBoardFeatures.HandlePressSensor)==COMPONENT_OK) {
        AZURE_PRINTF("Enabled Pressure Sensor\n\r");
      }
    }
  }
}

/**
  * @brief  Send Notification where there is a interrupt from MEMS
  * @param  None
  * @retval None
  */
void MEMSCallback(void)
{
  uint8_t stat = 0;  

  AZURE_PRINTF("MEMSCallback() \r\n");

  if(W2ST_CHECK_HW_FEATURE(W2ST_HWF_FREE_FALL)) {
    /* Check if the interrupt is due to Free Fall */
    (TargetBoardFeatures.SnsAltFunc ? BSP_ACCELERO_Get_Free_Fall_Detection_Status_Ext_IKS01A2 : BSP_ACCELERO_Get_Free_Fall_Detection_Status_Ext)(TargetBoardFeatures.HandleAccSensor,&stat);
    if(stat) {
      /***** Only for testing the RTC. From here */
      uint8_t aShowTime[50] = {0};
      uint8_t aShowDate[50] = {0};
      RTC_CalendarShow(aShowTime,aShowDate);
      AZURE_PRINTF("%s %s ->",aShowDate,aShowTime);
      /* Until here *****/
      AZURE_PRINTF("Free Fall\r\n");
    }
  }

  if(W2ST_CHECK_HW_FEATURE(W2ST_HWF_DOUBLE_TAP)) {
    /* Check if the interrupt is due to Double Tap */
    (TargetBoardFeatures.SnsAltFunc ? BSP_ACCELERO_Get_Double_Tap_Detection_Status_Ext_IKS01A2 : BSP_ACCELERO_Get_Double_Tap_Detection_Status_Ext)(TargetBoardFeatures.HandleAccSensor,&stat);
    if(stat) {
      AZURE_PRINTF("Double Tap\r\n");
    }
  }

  if(W2ST_CHECK_HW_FEATURE(W2ST_HWF_SINGLE_TAP)) {
    /* Check if the interrupt is due to Single Tap */
    (TargetBoardFeatures.SnsAltFunc ? BSP_ACCELERO_Get_Single_Tap_Detection_Status_Ext_IKS01A2 : BSP_ACCELERO_Get_Single_Tap_Detection_Status_Ext)(TargetBoardFeatures.HandleAccSensor,&stat);
    if(stat) {
      AZURE_PRINTF("Single Tap\r\n");
    }
  }

  if(W2ST_CHECK_HW_FEATURE(W2ST_HWF_WAKE_UP)) {
    /* Check if the interrupt is due to Wake Up */
    (TargetBoardFeatures.SnsAltFunc ? BSP_ACCELERO_Get_Wake_Up_Detection_Status_Ext_IKS01A2 : BSP_ACCELERO_Get_Wake_Up_Detection_Status_Ext)(TargetBoardFeatures.HandleAccSensor,&stat);
    if(stat) {
      AZURE_PRINTF("Wake Up\r\n");
    }
  }

  if(W2ST_CHECK_HW_FEATURE(W2ST_HWF_TILT)) {
    /* Check if the interrupt is due to Tilt */
    (TargetBoardFeatures.SnsAltFunc ? BSP_ACCELERO_Get_Tilt_Detection_Status_Ext_IKS01A2 : BSP_ACCELERO_Get_Tilt_Detection_Status_Ext)(TargetBoardFeatures.HandleAccSensor,&stat);
    if(stat) {
      AZURE_PRINTF("Tilt\r\n");
    }
  }

  if(W2ST_CHECK_HW_FEATURE(W2ST_HWF_6DORIENTATION)) {
    /* Check if the interrupt is due to 6D Orientation */
    (TargetBoardFeatures.SnsAltFunc ? BSP_ACCELERO_Get_6D_Orientation_Status_Ext_IKS01A2 : BSP_ACCELERO_Get_6D_Orientation_Status_Ext)(TargetBoardFeatures.HandleAccSensor,&stat);
    if(stat) {
      AccEventType Orientation = GetHWOrientation6D();
      AZURE_PRINTF("6DOrientation=%d\r\n",Orientation);
    }
  }

  if(W2ST_CHECK_HW_FEATURE(W2ST_HWF_PEDOMETER)) {
    /* Check if the interrupt is due to Pedometer */
    (TargetBoardFeatures.SnsAltFunc ? BSP_ACCELERO_Get_Pedometer_Status_Ext_IKS01A2 : BSP_ACCELERO_Get_Pedometer_Status_Ext)(TargetBoardFeatures.HandleAccSensor,&stat);
    if(stat) {
      uint16_t StepCount = GetStepHWPedometer();
       AZURE_PRINTF("StepCount=%d\r\n",StepCount);
    }
  }
}

#ifdef USE_STM32L4XX_NUCLEO
/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow:
  *            System Clock source            = PLL (MSI)
  *            SYSCLK(Hz)                     = 80000000
  *            HCLK(Hz)                       = 80000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 1
  *            APB2 Prescaler                 = 1
  *            MSI Frequency(Hz)              = 4000000
  *            PLL_M                          = 1
  *            PLL_N                          = 40
  *            PLL_R                          = 2
  *            PLL_P                          = 7
  *            PLL_Q                          = 4
  *            Flash Latency(WS)              = 4
  * @param  None
  * @retval None
  */
static void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInit;

  /* MSI is enabled after System reset, activate PLL with MSI as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 40;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLP = 7;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK){
    /* Initialization Error */
    while(1);
  }

  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
     clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK) {
    /* Initialization Error */
    while(1);
  }
  
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RNG;
  PeriphClkInit.RngClockSelection = RCC_RNGCLKSOURCE_PLLSAI1;
  PeriphClkInit.PLLSAI1.PLLSAI1Source = RCC_PLLSOURCE_MSI;
  PeriphClkInit.PLLSAI1.PLLSAI1M = 1;
  PeriphClkInit.PLLSAI1.PLLSAI1N = 16;
  PeriphClkInit.PLLSAI1.PLLSAI1P = RCC_PLLP_DIV7;
  PeriphClkInit.PLLSAI1.PLLSAI1Q = RCC_PLLQ_DIV2;
  PeriphClkInit.PLLSAI1.PLLSAI1R = RCC_PLLR_DIV2;
  PeriphClkInit.PLLSAI1.PLLSAI1ClockOut = RCC_PLLSAI1_48M2CLK;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    /* Initialization Error */
    while(1);
  }
  
}
#endif /* USE_STM32L4XX_NUCLEO */

/**
  * @brief  Output Compare callback in non blocking mode
  * @param  htim : TIM OC handle
  * @retval None
  */
void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim)
{
  uint32_t uhCapture=0;

  /* TIM1_CH1 toggling with frequency = 2Hz */
  if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) {
    uhCapture = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
    /* Set the Capture Compare Register value */
    __HAL_TIM_SET_COMPARE(&TimCCHandle, TIM_CHANNEL_1, (uhCapture + TargetBoardFeatures.TIM_CC1_Pulse));
    SendData=1;
  }
}


/**
* @brief  Function for initializing timers for sending the Telemetry data to IoT hub
 * @param  None
 * @retval None
 */
static void InitTimers(void)
{
  uint32_t uwPrescalerValue;

  /* Timer Output Compare Configuration Structure declaration */
  TIM_OC_InitTypeDef sConfig;

  /* Compute the prescaler value to have TIM3 counter clock equal to 2 KHz */
  uwPrescalerValue = (uint32_t) ((SystemCoreClock / 2000) - 1);

  /* Set TIM1 instance (Motion)*/
  TimCCHandle.Instance = TIM1;
  TimCCHandle.Init.Period        = 65535;
  TimCCHandle.Init.Prescaler     = uwPrescalerValue;
  TimCCHandle.Init.ClockDivision = 0;
  TimCCHandle.Init.CounterMode   = TIM_COUNTERMODE_UP;
  if(HAL_TIM_OC_Init(&TimCCHandle) != HAL_OK) {
    /* Initialization Error */
    STM32_Error_Handler();
  }

 /* Configure the Output Compare channels */
 /* Common configuration for all channels */
  sConfig.OCMode     = TIM_OCMODE_TOGGLE;
  sConfig.OCPolarity = TIM_OCPOLARITY_LOW;

  /* Output Compare Toggle Mode configuration: Channel1 */
  TargetBoardFeatures.TIM_CC1_Pulse = sConfig.Pulse = DEFAULT_TIM_CC1_PULSE;
  if(HAL_TIM_OC_ConfigChannel(&TimCCHandle, &sConfig, TIM_CHANNEL_1) != HAL_OK) {
    /* Configuration Error */
    STM32_Error_Handler();
  }
}


/**
* @brief  Function for initializing the Real Time Clock
 * @param  None
 * @retval None
 */
static void InitRTC(void)
{
  RTC_HandleTypeDef *RtcHandle = &TargetBoardFeatures.RtcHandle;

  /* Configure RTC prescaler and RTC data registers */
  /* RTC configured as follow:
      - Hour Format    = Format 24
      - Asynch Prediv  = Value according to source clock
      - Synch Prediv   = Value according to source clock
      - OutPut         = Output Disable
      - OutPutPolarity = High Polarity
      - OutPutType     = Open Drain
  */

  RtcHandle->Instance = RTC;
  RtcHandle->Init.HourFormat = RTC_HOURFORMAT_24;
  RtcHandle->Init.AsynchPrediv = RTC_ASYNCH_PREDIV;
  RtcHandle->Init.SynchPrediv = RTC_SYNCH_PREDIV;
  RtcHandle->Init.OutPut = RTC_OUTPUT_DISABLE;
  RtcHandle->Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  RtcHandle->Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;

  if(HAL_RTC_Init(RtcHandle) != HAL_OK) {
    STM32_Error_Handler();
  }
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void STM32_Error_Handler(void)
{
  /* User may add here some code to deal with this error */
  while(1){
  }
}

/**
 * @brief  EXTI line detection callback.
 * @param  uint16_t GPIO_Pin Specifies the pins connected EXTI line
 * @retval None
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  switch(GPIO_Pin){
  case KEY_BUTTON_PIN:
    ButtonPressed = 1;
    break;
#ifndef IKS01A2
   case M_INT1_PIN:
#endif /* IKS01A2 */
   case LSM6DSL_INT1_O_PIN:
    MEMSInterrupt=1;
    break;
  }
}


/**
* @brief  Period elapsed callback in non blocking mode
*         This timer is used for calling back User registered functions with information
* @param  htim TIM handle
* @retval None
*/
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  Wifi_TIM_Handler(htim);
}


/**
* @brief  function called after the end of GET/HEAD request for FOTA
* @param  void* context type of request (GET/HEAD)
* @param  IO_SEND_RESULT send_result Results of the request (IO_SEND_OK/...)
* @retval None
*/
static void onSendCompleteFOTA(void* context, IO_SEND_RESULT send_result)
{
  int32_t *TypeOfRequest = (int32_t *)context;
  if (send_result != IO_SEND_OK){
    AZURE_PRINTF("onSendCompleteFOTA Err for HTTP %s\r\n",((*TypeOfRequest) == HTTP_HEAD_REQUEST) ? "HEAD" : "GET"); 
  }
}


/**
* @brief  function called After the Socket open request  for FOTA
* @param  void* context (not Used)
* @param  IO_OPEN_RESULT open_result Results of the open command (IO_OPEN_OK/...)
* @retval None
*/
static void onOpenCompleteFOTA(void* context, IO_OPEN_RESULT open_result)
{  
  if (open_result != IO_OPEN_OK){
    AZURE_PRINTF("onOpenCompleteFOTA Err\r\n"); 
  }
}

/**
* @brief  function called when we are receiving data on the socket open for FOTA
* @param  void* context type of request (GET/HEAD)
* @param  unsigned char* data_ptr pointer to the data received
* @param  size_t size size to the data received
* @retval None
*/
static void onBytesReceivedFOTA(void* context, const unsigned char* data_ptr, size_t size)
{
  int32_t *TypeOfRequest = (int32_t *)context; 
#ifdef USE_STM32L4XX_NUCLEO
  static int32_t PaddingBytes=0;
#endif /* USE_STM32L4XX_NUCLEO */

  if((*TypeOfRequest) == HTTP_HEAD_REQUEST) {
    /* for the HEAD Request */
    uint8_t *StartAddress=NULL;
    OTAStatus = OTA_STATUS_RUNNIG;
#ifdef DEBUG_OTA_RECEIVE
    AZURE_PRINTF("message from HEAD Request Size=%d\r\n",size);
#endif /* DEBUG_OTA_RECEIVE */

    /* Make the parsing of the header */
    if(FullOTASize==0) {
      /* Search the String "Content-Range: bytes" */
      StartAddress = (uint8_t *) strstr( (char *) data_ptr, "Content-Range: bytes");
      if(StartAddress!=NULL) {
          StartAddress = (uint8_t *) strstr((char *) StartAddress, "/");
          if(StartAddress!=NULL) {
            StartAddress++;
            FullOTASize = atoi((char *) StartAddress);
            AZURE_PRINTF("(Content-Range:) Full OTA size=%ld\r\n",FullOTASize);
          } else {
            AZURE_PRINTF("Err... retriving the Full OTA Size...(Content-Range: bytes)\r\n");
            OTAStatus = OTA_STATUS_ERROR;
            return;
          }
      } else {
        /* Search the String "Content-Lenght:" */
        StartAddress = (uint8_t *) strstr( (char *) data_ptr, "Content-Length:");
        if(StartAddress!=NULL) {
          StartAddress+=16; // for moving to the end of "Content-Length: "
          FullOTASize = atoi((char *) StartAddress);
          AZURE_PRINTF("(Content-Length:) Full OTA size=%ld\r\n",FullOTASize);
        } else {
          AZURE_PRINTF("Err... retriving the Full OTA Size...(Content-Length:)\r\n");
          OTAStatus = OTA_STATUS_ERROR;
          return;
        }
      }
      /* Padding the OTA size for L476 Board */
#ifdef USE_STM32L4XX_NUCLEO
      if(FullOTASize&0x7) {
        PaddingBytes = 8 - (FullOTASize&0x7);
        AZURE_PRINTF("\tPaddingBytes=%ld\r\n",PaddingBytes);
        FullOTASize += PaddingBytes;
        AZURE_PRINTF("\tOTA Round=%ld\r\n",FullOTASize);
      }
#endif /* USE_STM32L4XX_NUCLEO */
    } else {
#ifdef DEBUG_OTA_RECEIVE
      AZURE_PRINTF("Consuming remaining bytes from HEAD request\r\n");
#endif /* DEBUG_OTA_RECEIVE */
      /* Search the end of HEAD request... */
      StartAddress = (uint8_t *) strstr( (char *) data_ptr, "\r\n\r\n");      
      if(StartAddress!=NULL) {
        OTAStatus = OTA_STATUS_END;
      }
    }
  } else {
    /* for the GET Request */
    uint8_t *StartAddress=NULL;
    static uint32_t SizeOfUpdate=0;
    uint32_t RealMessageSize=0;
#ifdef USE_STM32L4XX_NUCLEO
    static uint64_t ValueToWrite=0;
    static uint8_t *Pointer= (uint8_t *) &ValueToWrite;
    static int32_t PointerCounter= 0;
    int32_t counter;
#endif /* USE_STM32L4XX_NUCLEO */

#define MCR_OTA_DOUBLE_WORLD(size)                  \
{                                                   \
  for(counter=0;counter<(size);counter++) {         \
    Pointer[PointerCounter] = StartAddress[counter];\
    PointerCounter++;                               \
    /* Write the Double Word */                     \
    if(PointerCounter==8) {                         \
      UpdateFWBlueMS(&SizeOfUpdate,Pointer, 8,1);   \
      PointerCounter=0;                             \
      ValueToWrite=0;                               \
    }                                               \
  }                                                 \
}
    
    if(SizeOfUpdate==0) {
      /* Search the Content-Length: */
     StartAddress = (uint8_t *) strstr( (char *) data_ptr, "Content-Length:");
      if(StartAddress!=NULL) {
        StartAddress+=16; // for moving to the end of "Content-Length: "
        SizeOfUpdate = atoi((char *) StartAddress);
#ifdef DEBUG_OTA_RECEIVE
        AZURE_PRINTF("Chunck OTA size=%ld\r\n",SizeOfUpdate);
#endif /* DEBUG_OTA_RECEIVE */
      } else {
#ifdef DEBUG_OTA_RECEIVE
        AZURE_PRINTF("Starting OTA not found.. move to next chunk\r\n");
#endif /* DEBUG_OTA_RECEIVE */
      }
    }

    /* If we had found the Chunk OTA Size */
    if(SizeOfUpdate!=0) {
      if(OTAStatus==OTA_STATUS_START) {
        /* Search the real Start Postion skipping the Banner */
        StartAddress = (uint8_t *) strstr( (char *) data_ptr, "\r\n\r\n");
        if(StartAddress!=NULL) {
          StartAddress += 4;
          RealMessageSize= size - (StartAddress - data_ptr);
#ifdef DEBUG_OTA_RECEIVE
          AZURE_PRINTF("RealMessageSize=%d\r\n",RealMessageSize);
#endif /* DEBUG_OTA_RECEIVE */

          OTAStatus=OTA_STATUS_RUNNIG;

          MCR_OTA_DOUBLE_WORLD(RealMessageSize);
#ifdef USE_STM32L4XX_NUCLEO
          if(PaddingBytes) {
            if(ReadRemSizeOfUpdate()==8) {
              /* Flash the Padding bytes */
              AZURE_PRINTF("Flashing the last chunk of 8 bytes\r\n");
              SizeOfUpdate+=PaddingBytes; /* This for the next if(SizeOfUpdate==0) condition */
              UpdateFWBlueMS(&SizeOfUpdate,Pointer, 8,1);
              PointerCounter=0; /* This could be avoided */
              ValueToWrite=0;   /* This could be avoided */
            }
          }
#endif /* USE_STM32L4XX_NUCLEO */

          /* Control if we are at the end... */
          if(SizeOfUpdate==0) {
            OTAStatus = OTA_STATUS_END;
#ifdef DEBUG_OTA_RECEIVE
            AZURE_PRINTF("A OTA_STATUS_END\r\n");
          } else {
            AZURE_PRINTF("A Remaining Size of Update=%d\r\n",SizeOfUpdate);
#endif /* DEBUG_OTA_RECEIVE */
          }
        } else {
#ifdef DEBUG_OTA_RECEIVE
          AZURE_PRINTF("Starting OTA not found.. move to next chunk\r\n");
#endif /* DEBUG_OTA_RECEIVE */
        }
      } else {
        StartAddress = (unsigned char *)data_ptr;
#ifdef DEBUG_OTA_RECEIVE
        AZURE_PRINTF("New Message=%d\r\n",size);
#endif /* DEBUG_OTA_RECEIVE */

        MCR_OTA_DOUBLE_WORLD(size);

#ifdef USE_STM32L4XX_NUCLEO
        if(PaddingBytes) {
          if(ReadRemSizeOfUpdate()==8) {
            /* Flash the Padding bytes */
            AZURE_PRINTF("Flashing the last chunk of 8 bytes\r\n");
            SizeOfUpdate+=PaddingBytes; /* This for the next if(SizeOfUpdate==0) condition */
            UpdateFWBlueMS(&SizeOfUpdate,Pointer, 8,1);
            PointerCounter=0; /* This could be avoided */
            ValueToWrite=0;   /* This could be avoided */
          }
        }
#endif /* USE_STM32L4XX_NUCLEO */

        /* Control if we are at the end... */
        if(SizeOfUpdate==0) {
          OTAStatus = OTA_STATUS_END;
#ifdef DEBUG_OTA_RECEIVE
          AZURE_PRINTF("B OTA_STATUS_END\r\n");
        } else {
          AZURE_PRINTF("B Remaining Size of Update=%d\r\n",SizeOfUpdate);
#endif /* DEBUG_OTA_RECEIVE */
        }
      }
    }
  }
}

/**
* @brief  function called when there is one error
* @param  void* context (not Used)
* @retval None
*/
static void onIoErrorFOTA(void* context)
{  
  AZURE_PRINTF("Err: onIoErrorFOTA callback\r\n");
}

/** @brief  function called after the Socket close request  for FOTA
* @param  void* context (not Used)
* @retval None
*/
static void onCloseCompleteOTA(void* context)
{  
  AZURE_PRINTF("onCloseCompleteOTA callback\r\n");
}

/**
  * @brief  Execute the Firmware update after the Command FOTA
  * @param  char *hostname Server address
  * @param  char type 's'/'t'->Secure/NotSecure socket
  * @param  uint32_t port_num Server port number
  * @param  char *path FOTA path
  * @retval AZURE1_OTA FOTA Status Error/Success -> OTA_STATUS_ERROR/OTA_STATUS_NULL
  */
AZURE1_OTA FOTACallback(char * hostname,char type, uint32_t  port_num,char * path)
{
  char    buf[256];
  const IO_INTERFACE_DESCRIPTION* io_interface_description;
  XIO_HANDLE XIO_Instance;
  TLSIO_CONFIG tls_io_config;
  int32_t TypeOfRequest;
  int32_t ChunkNum;

  AZURE_PRINTF("Download FOTA from: HostName=[%s] Type=[%s] port=[%ld] File=[%s]\r\n",hostname,(type=='s') ? "Secure": "NotSecure", port_num,path);

  ReportState(Downloading);
  /* This because we want to be sure that this message reach the IOT hub */
  WaitAllTheMessages();

  /* Interface Description */
  io_interface_description = tlsio_mbedtls_get_interface_description();
  if (io_interface_description == NULL) {
    AZURE_PRINTF("Err: io_interface_description\r\n");
    return OTA_STATUS_ERROR;
  }    
  AZURE_PRINTF("Ok io_interface_description\r\n");

  tls_io_config.hostname = hostname;
  tls_io_config.port = port_num;
  /* XIO_CREATE */
  XIO_Instance = xio_create(io_interface_description, &tls_io_config);
  if(XIO_Instance==NULL) {
    AZURE_PRINTF("Err: xio_create\r\n");
    OTAStatus = OTA_STATUS_ERROR;
    goto FOTA_exit;
  }
  AZURE_PRINTF("Ok xio_create\r\n");

  /* XIO_SETOPTION */
  if(xio_setoption(XIO_Instance,  "TrustedCerts", certificates)!=0) {
    AZURE_PRINTF("Err: xio_setoption\r\n");
    OTAStatus = OTA_STATUS_ERROR;
    goto FOTA_exit;
  }
  AZURE_PRINTF("Ok xio_setoption\r\n");

  /* XIO_OPEN */
  OTAStatus=OTA_STATUS_NULL;
  if(xio_open(XIO_Instance, onOpenCompleteFOTA, NULL, onBytesReceivedFOTA, &TypeOfRequest, onIoErrorFOTA, NULL)!=0) {
    AZURE_PRINTF("Err: xio_open\r\n");
    OTAStatus = OTA_STATUS_ERROR;
    goto FOTA_exit;
  }
  AZURE_PRINTF("Ok xio_open\r\n");
  
  /* XIO_SEND HEAD Request  */
  OTAStatus=OTA_STATUS_START;
  TypeOfRequest= HTTP_HEAD_REQUEST;
  sprintf(buf,"HEAD %s HTTP/1.1\r\nHost: %s\r\nRange: bytes=0-1\r\n\r\n",path,hostname);
  if(xio_send(XIO_Instance, buf, strlen(buf),  onSendCompleteFOTA, &TypeOfRequest)!=0) {
    AZURE_PRINTF("Err: xio_send\r\n");
    OTAStatus = OTA_STATUS_ERROR;
    goto FOTA_exit;
  } else {
    AZURE_PRINTF("Ok xio_send HEAD Request\r\n");
  }

  /* Wait the end of HEAD */
  while((OTAStatus != OTA_STATUS_END) & (OTAStatus != OTA_STATUS_ERROR)) {
    xio_dowork(XIO_Instance);
    HAL_Delay(100);
  }

  if(OTAStatus == OTA_STATUS_ERROR) {
    goto FOTA_exit;
  }

  /* Clean the Flash before the Real HTTP GET */
  StartUpdateFWBlueMS(FullOTASize, 0);
  for(ChunkNum=0;(ChunkNum*FOTA_CHUNK_SIZE)<FullOTASize;ChunkNum++) {
    /* XIO_SEND GET Request  */
    OTAStatus = OTA_STATUS_START;
    TypeOfRequest= HTTP_GET_REQUEST;


    /* Prepare Request */
    if(((ChunkNum+1)*FOTA_CHUNK_SIZE-1)<FullOTASize) {
      sprintf(buf,"GET %s HTTP/1.1\r\nHost: %s\r\nRange: bytes=%ld-%ld\r\n\r\n",path,hostname,ChunkNum*FOTA_CHUNK_SIZE,(ChunkNum+1)*FOTA_CHUNK_SIZE-1);
    } else {
      sprintf(buf,"GET %s HTTP/1.1\r\nHost: %s\r\nRange: bytes=%ld-%ld\r\n\r\n",path,hostname,ChunkNum*FOTA_CHUNK_SIZE,FullOTASize);
    }

    if(xio_send(XIO_Instance, buf, strlen(buf),  onSendCompleteFOTA, &TypeOfRequest)!=0) {
      AZURE_PRINTF("Err: xio_send\r\n");
      OTAStatus = OTA_STATUS_ERROR;
      goto FOTA_exit;
    } else {
      AZURE_PRINTF("Ok xio_send GET (%03ld/%03ld) Request\r\n",ChunkNum,(FullOTASize+FOTA_CHUNK_SIZE-1)/FOTA_CHUNK_SIZE-1);
    }

    /* Wait the end of Chuck or Error */
    while((OTAStatus != OTA_STATUS_END) & (OTAStatus != OTA_STATUS_ERROR)) {
      xio_dowork(XIO_Instance);
      HAL_Delay(100);
    }

    if(OTAStatus == OTA_STATUS_ERROR) {
      goto FOTA_exit;
    } else {
      OTAStatus = OTA_STATUS_NULL;
    }
  }

  if(xio_close(XIO_Instance, onCloseCompleteOTA, NULL)!=0) {
    AZURE_PRINTF("Err: xio_close\r\n");
    OTAStatus = OTA_STATUS_ERROR;
    goto FOTA_exit;
  } else {
    AZURE_PRINTF("Ok xio_close\r\n");
  }

FOTA_exit:
  if(OTAStatus == OTA_STATUS_NULL) {
    /* Everything was ok */
    AZURE_PRINTF("OTA Downloaded\r\n");
    ReportState(DownloadComplete);
    WaitAllTheMessages();
    return OTAStatus;
  } else {
    /* The socket was closed before the end of FOTA trasmission... */
    AZURE_PRINTF("\r\nErr During FOTA... Sorry retry...\r\n");
    return OTA_STATUS_ERROR;
  }
}


/**
  * @brief This function provides accurate delay (in milliseconds) based
  *        on variable incremented.
  * @note This is a user implementation using WFI state
  * @param Delay specifies the delay time length, in milliseconds.
  * @retval None
  */
void HAL_Delay(__IO uint32_t Delay)
{
  uint32_t tickstart = 0;
  tickstart = HAL_GetTick();
  while((HAL_GetTick() - tickstart) < Delay){
    __WFI();
  }
}

/**
  * @brief This function initializes the GPIO for NFC expansion board
  * @param None
  * @retval None
  */

void M24SR_GPOInit(void)
{
  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __GPIOA_CLK_ENABLE();

  /* Configure GPIO pins for GPO (PA6)*/
#ifndef I2C_GPO_INTERRUPT_ALLOWED
  GPIO_InitStruct.Pin = M24SR_GPO_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  HAL_GPIO_Init(M24SR_GPO_PIN_PORT, &GPIO_InitStruct);
#else
  GPIO_InitStruct.Pin = M24SR_GPO_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  HAL_GPIO_Init(M24SR_GPO_PIN_PORT, &GPIO_InitStruct);
  /* Enable and set EXTI9_5 Interrupt to the lowest priority */
  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 3, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
#endif

  /* Configure GPIO pins for DISABLE (PA7)*/
  GPIO_InitStruct.Pin = M24SR_RFDIS_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(M24SR_RFDIS_PIN_PORT, &GPIO_InitStruct);
}

/**
  * @brief  This function wait the time given in param (in milisecond)
	* @param	time_ms: time value in milisecond
  */
void M24SR_WaitMs(uint32_t time_ms)
{
  HAL_Delay(time_ms);
}

/**
  * @brief  This function retrieve current tick
  * @param	ptickstart: pointer on a variable to store current tick value
  */
void M24SR_GetTick( uint32_t *ptickstart )
{
  *ptickstart = HAL_GetTick();
}
/**
  * @brief  This function read the state of the M24SR GPO
	* @param	none
  * @retval GPIO_PinState : state of the M24SR GPO
  */
void M24SR_GPO_ReadPin( GPIO_PinState * pPinState)
{
  *pPinState = HAL_GPIO_ReadPin(M24SR_GPO_PIN_PORT,M24SR_GPO_PIN);
}

/**
  * @brief  This function set the state of the M24SR RF disable pin
	* @param	PinState: put RF disable pin of M24SR in PinState (1 or 0)
  */
void M24SR_RFDIS_WritePin( GPIO_PinState PinState)
{
  HAL_GPIO_WritePin(M24SR_RFDIS_PIN_PORT,M24SR_RFDIS_PIN,PinState);
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
     ex: AZURE_PRINTF("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1){
  }
}
#endif

#ifdef USE_STM32L4XX_NUCLEO
/**
  * @brief  Gets the page of a given address
  * @param  Addr: Address of the FLASH Memory
  * @retval The page of a given address
  */
static uint32_t GetPage(uint32_t Addr)
{
  uint32_t page = 0;

  if (Addr < (FLASH_BASE + FLASH_BANK_SIZE)) {
    /* Bank 1 */
    page = (Addr - FLASH_BASE) / FLASH_PAGE_SIZE;
  } else {
    /* Bank 2 */
    page = (Addr - (FLASH_BASE + FLASH_BANK_SIZE)) / FLASH_PAGE_SIZE;
  }
  return page;
}

/**
  * @brief  Gets the bank of a given address
  * @param  Addr: Address of the FLASH Memory
  * @retval The bank of a given address
  */
static uint32_t GetBank(uint32_t Addr)
{
  uint32_t bank = 0;

  if (READ_BIT(SYSCFG->MEMRMP, SYSCFG_MEMRMP_FB_MODE) == 0){
    /* No Bank swap */
    if (Addr < (FLASH_BASE + FLASH_BANK_SIZE)) {
      bank = FLASH_BANK_1;
    } else {
      bank = FLASH_BANK_2;
    }
  } else {
    /* Bank swap */
    if (Addr < (FLASH_BASE + FLASH_BANK_SIZE)) {
      bank = FLASH_BANK_2;
    } else {
      bank = FLASH_BANK_1;
    }
  }
  return bank;
}

/**
 * @brief User function for Erasing the MDM on Flash
 * @param None
 * @retval uint32_t Success/NotSuccess [1/0]
 */
uint32_t UserFunctionForErasingFlash(void) {
  FLASH_EraseInitTypeDef EraseInitStruct;
  uint32_t SectorError = 0;
  uint32_t Success=1;

  EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
  EraseInitStruct.Banks       = GetBank(MDM_FLASH_ADD);
  EraseInitStruct.Page        = GetPage(MDM_FLASH_ADD);
  EraseInitStruct.NbPages     = 2;

  /* Unlock the Flash to enable the flash control register access *************/
  HAL_FLASH_Unlock();

  if(HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError) != HAL_OK){
    /* Error occurred while sector erase. 
      User can add here some code to deal with this error. 
      SectorError will contain the faulty sector and then to know the code error on this sector,
      user can call function 'HAL_FLASH_GetError()'
      FLASH_ErrorTypeDef errorcode = HAL_FLASH_GetError(); */
    Success=0;
    STM32_Error_Handler();
  }

  /* Lock the Flash to disable the flash control register access (recommended
  to protect the FLASH memory against possible unwanted operation) *********/
  HAL_FLASH_Lock();

  return Success;
}

/**
 * @brief User function for Saving the MDM  on the Flash
 * @param void *InitMetaDataVector Pointer to the MDM beginning
 * @param void *EndMetaDataVector Pointer to the MDM end
 * @retval uint32_t Success/NotSuccess [1/0]
 */
uint32_t UserFunctionForSavingFlash(void *InitMetaDataVector,void *EndMetaDataVector)
{
  uint32_t Success=1;

  /* Store in Flash Memory */
  uint32_t Address = MDM_FLASH_ADD;
  uint64_t *WriteIndex;

  /* Unlock the Flash to enable the flash control register access *************/
  HAL_FLASH_Unlock();
  for(WriteIndex =((uint64_t *) InitMetaDataVector); WriteIndex<((uint64_t *) EndMetaDataVector); WriteIndex++) {
    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, Address,*WriteIndex) == HAL_OK){
      Address = Address + 8;
    } else {
      /* Error occurred while writing data in Flash memory.
         User can add here some code to deal with this error
         FLASH_ErrorTypeDef errorcode = HAL_FLASH_GetError(); */
      STM32_Error_Handler();
      Success =0;
    }
  }

  /* Lock the Flash to disable the flash control register access (recommended
   to protect the FLASH memory against possible unwanted operation) *********/
  HAL_FLASH_Lock();
 
  return Success;
}
#endif /* USE_STM32L4XX_NUCLEO */
/******************* (C) COPYRIGHT 2017 STMicroelectronics *****END OF FILE****/
