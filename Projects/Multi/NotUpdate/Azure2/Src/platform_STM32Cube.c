// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

/* This is a template file used for porting */

/* Please go through all the TODO sections below and implement the needed code */

#include <stdlib.h>
#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include "STM32CubeInterface.h"
#include "MetaDataManager.h"
#include "azure_c_shared_utility/crt_abstractions.h"
#include "azure_c_shared_utility/platform.h"
#include "tlsio_STM32Cube.h"
#include "TLocalBuffer.h"
#include "OTA.h"
#include "AzureClient_mqtt_DM_TM.h"

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

TIM_HandleTypeDef    TimCCHandle;

volatile int ButtonPressed  =0;
volatile int MEMSInterrupt  =0;
volatile uint32_t SendData  =0;
volatile uint32_t ReadProx  =0;

__IO wifi_state_t wifi_state;

WIFI_CredAcc_t WIFI_CredAcc;

/* Local variables ---------------------------------------------------------*/

static volatile int32_t ConnectedToNTP=0;
#ifdef AZURE_OTA_HTTP_SOCKET
static volatile uint8_t ConnectedToOTA=0xFF;
#endif /*AZURE_OTA_HTTP_SOCKET */

#define OTA_STATUS_NULL   0
#define OTA_STATUS_START  1
#define OTA_STATUS_RUNNIG 2
#define OTA_STATUS_END    3

static volatile int32_t OTAStatus=OTA_STATUS_NULL;
 
/* Local defines -------------------------------------------------------------*/
//10kHz/1 For Sensors Data data@1Hz
#define DEFAULT_uhCCR1_Val  20000
//#define DEFAULT_uhCCR1_Val  200
//10kHz/10 For 53L0X Data data@10Hz
#define DEFAULT_uhCCR2_Val  2000

/* Defines related to Clock configuration */    
#define RTC_ASYNCH_PREDIV  0x7F   /* LSE as RTC clock */
#define RTC_SYNCH_PREDIV   0x00FF /* LSE as RTC clock */


typedef enum {
	LONG_RANGE 		= 0, /*!< Long range mode */
	HIGH_SPEED 		= 1, /*!< High speed mode */
	HIGH_ACCURACY	= 2, /*!< High accuracy mode */
} RangingConfig_e;

VL53L0X_Dev_t VL53L0XDevs[]={
        {.Id=XNUCLEO53L0A1_DEV_LEFT, .DevLetter='l', .I2cHandle=&I2CHandle, .I2cDevAddr=0x52},
        {.Id=XNUCLEO53L0A1_DEV_CENTER, .DevLetter='c', .I2cHandle=&I2CHandle, .I2cDevAddr=0x52},
        {.Id=XNUCLEO53L0A1_DEV_RIGHT, .DevLetter='r', .I2cHandle=&I2CHandle, .I2cDevAddr=0x52},
};

int nDevPresent=0;
/** bit is index in VL53L0XDevs that is not necessary the dev id of the BSP */
static int nDevMask;
/** leaky factor for filtered range
 *
 * r(n) = averaged_r(n-1)*leaky +r(n)(1-leaky)
 *
 * */
static int LeakyFactorFix8 = (int)( 0.6 *256);

static int DetectSensors(void);
static void SetupSingleShot(RangingConfig_e rangingConfig);
static void Sensor_SetNewRange(VL53L0X_Dev_t *pDev, VL53L0X_RangingMeasurementData_t *pRange);
void StartTimer3(void);
void StopTimer3(void);

/* For Enabling the 53L01 print */
#if 1
  #define debug_printf(...) AZURE_PRINTF(__VA_ARGS__)
#else
  #define debug_printf(...)
#endif

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

#define MCR_AZURE_F2I_1D(in, out_int, out_dec) {out_int = (int32_t)in; out_dec= (int32_t)((in-out_int)*10);};
#define MCR_AZURE_F2I_2D(in, out_int, out_dec) {out_int = (int32_t)in; out_dec= (int32_t)((in-out_int)*100);};

int platform_init(void)
{
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
#ifdef USE_STM32F4XX_NUCLEO
        "\tSTM32F401RE-Nucleo board"
#elif USE_STM32L4XX_NUCLEO
        "\tSTM32L476RG-Nucleo board"
#endif /* USE_STM32F4XX_NUCLEO */
          "\r\n",
          AZURE_PACKAGENAME,
          AZURE_VERSION_MAJOR,AZURE_VERSION_MINOR,AZURE_VERSION_PATCH);

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

#ifdef AZURE_ENABLE_OTA
  AZURE_PRINTF("OTA enabled by Device Method\r\n");
  #ifdef AZURE_OTA_HTTP_SOCKET
    AZURE_PRINTF("\tOTA with one HTTP get on one opened Socket\r\n");
  #else /* AZURE_OTA_HTTP_SOCKET */
    AZURE_PRINTF("\tOTA with one HTTP get\r\n");
  #endif /* AZURE_OTA_HTTP_SOCKET */
#endif /* AZURE_ENABLE_OTA */

  if(TargetBoardFeatures.HWAdvanceFeatures) {
    InitHWFeatures();
  }

  /* Set Full Scale to +/-2g */
  (TargetBoardFeatures.SnsAltFunc ? BSP_ACCELERO_Set_FS_Value_IKS01A2 : BSP_ACCELERO_Set_FS_Value)(TargetBoardFeatures.HandleAccSensor,2.0f);


  /* initialize timers */
  InitTimers();
  AZURE_PRINTF("Init Application's Timers\r\n");

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
              AZURE_PRINTF("ERROR reading the WIFI's credentials from NFC\r\n");
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
              AZURE_PRINTF("\r\nWrong Entry. Priv Mode is not compatible The board will restart in 2 Seconds\n");
              HAL_Delay(2000);
              HAL_NVIC_SystemReset();
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
        AZURE_PRINTF("ERROR reading the WIFI's credentials from NFC\r\n");
        AZURE_PRINTF("Restart the Board\r\n");
        /* Infinite triple led blinking */
        BSP_LED_Off(LED2);
        while(1) {
          BSP_LED_On(LED2);
          HAL_Delay(100);
          BSP_LED_Off(LED2);
          HAL_Delay(100);
          BSP_LED_On(LED2);
          HAL_Delay(100);
          BSP_LED_Off(LED2);
          HAL_Delay(100);
          BSP_LED_On(LED2);
          HAL_Delay(100);
          BSP_LED_Off(LED2);
          HAL_Delay(500);
        }
      }
    }
  }

  /* Init the X-NUCLEO-53L0A1 */
  XNUCLEO53L0A1_Init();
  DetectSensors();
  if(nDevPresent!=0) {
    XNUCLEO53L0A1_SetDisplayString("AZUR");
    /* Set VL53L0X API trace level */
    VL53L0X_trace_config(NULL, TRACE_MODULE_NONE, TRACE_LEVEL_NONE, TRACE_FUNCTION_NONE); // No Trace
    SetupSingleShot(LONG_RANGE);
  }

  /* Initialize the WIFI module */
  wifi_state = wifi_state_idle;
  memset(&config,0,sizeof(wifi_config));
  config.power=wifi_active;
  config.power_level=high;
  config.dhcp=on;//use DHCP IP address

  status = wifi_init(&config);
  if(status!=WiFi_MODULE_SUCCESS) {
    AZURE_PRINTF("Error in Config\r\n");
    return 1;
  } else {
    uint8_t macaddstart[32];
    AZURE_PRINTF("WIFI module Configured\r\n");          
    GET_Configuration_Value("nv_wifi_macaddr",(uint32_t *)macaddstart);
    macaddstart[17]='\0';
    AZURE_PRINTF("WiFi MAC Address is: %s\r\n",macaddstart);
  }
  
  status = wifi_connect(WIFI_CredAcc.ssid, WIFI_CredAcc.seckey, WIFI_CredAcc.mode);
  if(status!=WiFi_MODULE_SUCCESS) {
    AZURE_PRINTF("Error Connecting to WIFI\r\n");
    return 1;
  }
  /* Wait WIFI Connection */
  while(wifi_state != wifi_state_connected) {
    HAL_Delay(100);
  }

  if(nDevPresent) {
    XNUCLEO53L0A1_SetDisplayString("WIFI");
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
        AZURE_PRINTF("Error opening socket with NTP server\r\n");
      } else {
        AZURE_PRINTF("WiFi opened socket with NTP server [%s]\r\n",ipAddress);
        ConnectedToNTP=1;

        while(ConnectedToNTP) {
          HAL_Delay(100);
        }

        status = wifi_socket_client_close(socketHandle);
        if(status!=WiFi_MODULE_SUCCESS) {
          AZURE_PRINTF("Error closing socket with NTP server\r\n");
        } else {
          AZURE_PRINTF("WiFi closed socket with NTP server\r\n");
          NTP_OK = 1;
        }
      }

      if(!NTP_OK) {
        AZURE_PRINTF("I'll try again to connect to NTP server in 2 seconds\r\n");
        HAL_Delay(2000);
      }
    }

   if(nDevPresent) {
      XNUCLEO53L0A1_SetDisplayString("NTP ");
    }
  } else {
    uint8_t aShowTime[50] = {0};
    uint8_t aShowDate[50] = {0};
    RTC_CalendarShow(aShowTime,aShowDate);
    AZURE_PRINTF("Init Real Time Clock %s %s\r\n",aShowDate,aShowTime);
  }
  return 0;
}

void StartTimer1(void)
{
  /* Starting timer */
  if(HAL_TIM_OC_Start_IT(&TimCCHandle, TIM_CHANNEL_1) != HAL_OK){
    /* Starting Error */
    STM32_Error_Handler();
  }

  /* Set the new Capture compare value */
  {
    uint32_t uhCapture = __HAL_TIM_GET_COUNTER(&TimCCHandle);
    /* Set the Capture Compare Register value */
    __HAL_TIM_SET_COMPARE(&TimCCHandle, TIM_CHANNEL_1, (uhCapture + DEFAULT_uhCCR1_Val));
  }

  AZURE_PRINTF("Channel 1 for Timer 1 started\r\n");
}

void StopTimer1(void)
{
  /* Stop timer */
  if(HAL_TIM_OC_Stop_IT(&TimCCHandle, TIM_CHANNEL_1) != HAL_OK){
    /* Starting Error */
    STM32_Error_Handler();
  }
  AZURE_PRINTF("Channel 1 for Timer 1 stopped\r\n");
}

void StartTimer2(void)
{
  if(nDevPresent!=0) {
    /* Starting timer */
    if(HAL_TIM_OC_Start_IT(&TimCCHandle, TIM_CHANNEL_2) != HAL_OK){
      /* Starting Error */
      Error_Handler();
    }

    /* Set the new Capture compare value */
    {
      uint32_t uhCapture = __HAL_TIM_GET_COUNTER(&TimCCHandle);
      /* Set the Capture Compare Register value */
      __HAL_TIM_SET_COMPARE(&TimCCHandle, TIM_CHANNEL_2, (uhCapture + DEFAULT_uhCCR2_Val));
    }

    AZURE_PRINTF("Channel 2 for Timer 1 started\r\n");
  }
}

void StopTimer2(void)
{
  if(nDevPresent!=0) {
    /* Stop timer */
    if(HAL_TIM_OC_Stop_IT(&TimCCHandle, TIM_CHANNEL_2) != HAL_OK){
      /* Starting Error */
      Error_Handler();
    }
    AZURE_PRINTF("Channel 2 for Timer 1 stopped\r\n");
  }
}

/**
  * @brief  Callback for user button
  * @param  None
  * @retval None
  */
void ButtonCallback(void)
{
  AZURE_PRINTF("Button Pressed\r\n");
  exit(0);
}

/**
 *  Setup all detected sensors for single shot mode and setup ranging configuration
 */
static void SetupSingleShot(RangingConfig_e rangingConfig){
  int i;
  int status;
  uint8_t VhvSettings;
  uint8_t PhaseCal;
  uint32_t refSpadCount;
  uint8_t isApertureSpads;
  FixPoint1616_t signalLimit = (FixPoint1616_t)(0.25*65536);
  FixPoint1616_t sigmaLimit = (FixPoint1616_t)(18*65536);
  uint32_t timingBudget = 33000;
  uint8_t preRangeVcselPeriod = 14;
  uint8_t finalRangeVcselPeriod = 10;

  for( i=0; i<3; i++) {
    if( VL53L0XDevs[i].Present){
      status=VL53L0X_StaticInit(&VL53L0XDevs[i]);
      if( status ){
        debug_printf("VL53L0X_StaticInit %d failed\n",i);
      }

      status = VL53L0X_PerformRefCalibration(&VL53L0XDevs[i], &VhvSettings, &PhaseCal);
      if( status ){
        debug_printf("VL53L0X_PerformRefCalibration failed\n");
      }

      status = VL53L0X_PerformRefSpadManagement(&VL53L0XDevs[i], &refSpadCount, &isApertureSpads);
      if( status ){
        debug_printf("VL53L0X_PerformRefSpadManagement failed\n");
      }

      status = VL53L0X_SetDeviceMode(&VL53L0XDevs[i], VL53L0X_DEVICEMODE_SINGLE_RANGING); // Setup in single ranging mode
      //status = VL53L0X_SetDeviceMode(&VL53L0XDevs[i], VL53L0X_DEVICEMODE_GPIO_OSC);
      if( status ){
        debug_printf("VL53L0X_SetDeviceMode failed\n");
      }

      status = VL53L0X_SetLimitCheckEnable(&VL53L0XDevs[i], VL53L0X_CHECKENABLE_SIGMA_FINAL_RANGE, 1); // Enable Sigma limit
      if( status ){
         debug_printf("VL53L0X_SetLimitCheckEnable failed\n");
      }

      status = VL53L0X_SetLimitCheckEnable(&VL53L0XDevs[i], VL53L0X_CHECKENABLE_SIGNAL_RATE_FINAL_RANGE, 1); // Enable Signa limit
      if( status ){
         debug_printf("VL53L0X_SetLimitCheckEnable failed\n");
      }
      /* Ranging configuration */
      switch(rangingConfig) {
      case LONG_RANGE:
        signalLimit = (FixPoint1616_t)(0.1*65536);
        sigmaLimit = (FixPoint1616_t)(60*65536);
        timingBudget = 33000;
        preRangeVcselPeriod = 18;
        finalRangeVcselPeriod = 14;
      break;
      case HIGH_ACCURACY:
        signalLimit = (FixPoint1616_t)(0.25*65536);
        sigmaLimit = (FixPoint1616_t)(18*65536);
        timingBudget = 200000;
        preRangeVcselPeriod = 14;
        finalRangeVcselPeriod = 10;
      break;
      case HIGH_SPEED:
        signalLimit = (FixPoint1616_t)(0.25*65536);
        sigmaLimit = (FixPoint1616_t)(32*65536);
        timingBudget = 20000;
        preRangeVcselPeriod = 14;
        finalRangeVcselPeriod = 10;
      break;
      default:
        debug_printf("Not Supported");
      }

      status = VL53L0X_SetLimitCheckValue(&VL53L0XDevs[i],  VL53L0X_CHECKENABLE_SIGNAL_RATE_FINAL_RANGE, signalLimit);
      if( status ){
        debug_printf("VL53L0X_SetLimitCheckValue failed\n");
      }

      status = VL53L0X_SetLimitCheckValue(&VL53L0XDevs[i],  VL53L0X_CHECKENABLE_SIGMA_FINAL_RANGE, sigmaLimit);
      if( status ){
        debug_printf("VL53L0X_SetLimitCheckValue failed\n");
      }

      status = VL53L0X_SetMeasurementTimingBudgetMicroSeconds(&VL53L0XDevs[i],  timingBudget);
      if( status ){
        debug_printf("VL53L0X_SetMeasurementTimingBudgetMicroSeconds failed\n");
      }

      status = VL53L0X_SetVcselPulsePeriod(&VL53L0XDevs[i],  VL53L0X_VCSEL_PERIOD_PRE_RANGE, preRangeVcselPeriod);
      if( status ){
        debug_printf("VL53L0X_SetVcselPulsePeriod failed\n");
      }

      status = VL53L0X_SetVcselPulsePeriod(&VL53L0XDevs[i],  VL53L0X_VCSEL_PERIOD_FINAL_RANGE, finalRangeVcselPeriod);
      if( status ){
        debug_printf("VL53L0X_SetVcselPulsePeriod failed\n");
      }

      status = VL53L0X_PerformRefCalibration(&VL53L0XDevs[i], &VhvSettings, &PhaseCal);
      if( status ){
        debug_printf("VL53L0X_PerformRefCalibration failed\n");
      }

      status = XNUCLEO53L0A1_SetIntrStateId(0,i);
      if( status ){
        debug_printf("XNUCLEO53L0A1_SetIntrStateId failed\n");
      }

      VL53L0XDevs[i].LeakyFirst=1;
    }
  }
}

/**
 * Reset all sensor then do presence detection
 *
 * All present devices are data initiated and assigned to their final I2C address
 * @return
 */
static int DetectSensors(void) {
  int i;
  uint16_t Id;
  int status;
  int FinalAddress;

  /* Reset all */
  nDevPresent = 0;
  for (i = 0; i < 3; i++) {
    status = XNUCLEO53L0A1_ResetId(i, 0);
  }

  /* detect all sensors (even on-board)*/
  for (i = 0; i < 3; i++) {
    VL53L0X_Dev_t *pDev;
    pDev = &VL53L0XDevs[i];
    pDev->I2cDevAddr = 0x52;
    pDev->Present = 0;
    status = XNUCLEO53L0A1_ResetId( pDev->Id, 1);
    HAL_Delay(2);
    FinalAddress=0x52+(i+1)*2;

    do {
      /* Set I2C standard mode (400 KHz) before doing the first register access */
      if (status == VL53L0X_ERROR_NONE) {
        status = VL53L0X_WrByte(pDev, 0x88, 0x00);
      }

      /* Try to read one register using default 0x52 address */
      status = VL53L0X_RdWord(pDev, VL53L0X_REG_IDENTIFICATION_MODEL_ID, &Id);
      if (status) {
        debug_printf("#%d Read id fail\r\n", i);
        break;
      }
      if (Id == 0xEEAA) {
        /* Sensor is found => Change its I2C address to final one */
        status = VL53L0X_SetDeviceAddress(pDev,FinalAddress);
        if (status != 0) {
          debug_printf("#%d VL53L0X_SetDeviceAddress fail\r\n", i);
          break;
        }

        pDev->I2cDevAddr = FinalAddress;
        /* Check all is OK with the new I2C address and initialize the sensor */
        status = VL53L0X_RdWord(pDev, VL53L0X_REG_IDENTIFICATION_MODEL_ID, &Id);
        if (status != 0) {
          debug_printf("#%d VL53L0X_RdWord fail\r\n", i);
          break;
        }

        status = VL53L0X_DataInit(pDev);
        if( status == 0 ){
          pDev->Present = 1;
        } else {
          debug_printf("VL53L0X_DataInit %d fail\r\n", i);
          break;
        }
        debug_printf("VL53L0X %d Present and initiated to final 0x%x\r\n", pDev->Id, pDev->I2cDevAddr);
        nDevPresent++;
        nDevMask |= 1 << i;
        pDev->Present = 1;
      } else {
        debug_printf("#%d unknown ID %x\r\n", i, Id);
        status = 1;
      }
    } while (0);
    /* if fail r can't use for any reason then put the  device back to reset */
    if (status) {
      XNUCLEO53L0A1_ResetId(i, 0);
    }
  }
  return nDevPresent;
}


void ReadSingle53L0X(void)
{
  int status;
  VL53L0X_RangingMeasurementData_t RangingMeasurementData;  
  status = VL53L0X_PerformSingleRangingMeasurement(&VL53L0XDevs[1],&RangingMeasurementData);
  if( status ){
    AZURE_PRINTF("---->Error Reading VL53L0 Sensor 1\r\n");
  } else {
    char Number[4];
    Sensor_SetNewRange(&VL53L0XDevs[1],&RangingMeasurementData);
    if(RangingMeasurementData.RangeStatus == 0) {
      sprintf(Number,"%04d",RangingMeasurementData.RangeMilliMeter);
    } else {
      sprintf(Number,"----");
    }
    XNUCLEO53L0A1_SetDisplayString(Number);
  }
}

/* Store new ranging data into the device structure, apply leaky integrator if needed */
static void Sensor_SetNewRange(VL53L0X_Dev_t *pDev, VL53L0X_RangingMeasurementData_t *pRange){
  if( pRange->RangeStatus == 0 ){
    if( pDev->LeakyFirst ){
      pDev->LeakyFirst = 0;
      pDev->LeakyRange = pRange->RangeMilliMeter;
    } else {
      pDev->LeakyRange = (pDev->LeakyRange*LeakyFactorFix8 + (256-LeakyFactorFix8)*pRange->RangeMilliMeter)>>8;
    }
  } else {
    pDev->LeakyFirst = 1;
  }
}


#ifdef AZURE_ENABLE_OTA
#ifndef AZURE_OTA_HTTP_SOCKET
void ind_wifi_http_data_available(uint8_t * data_ptr, uint32_t message_size)
{
//#define DEBUG_OTA_RECEIVE
  static uint32_t SizeOFUpdate=0;
#ifdef USE_STM32L4XX_NUCLEO
  static uint64_t ValueToWrite=0;
  static uint8_t *Pointer= (uint8_t *) &ValueToWrite;
  static int32_t PointerCounter= 0;
  int32_t counter;
#endif /* USE_STM32L4XX_NUCLEO */

#ifdef USE_STM32F4XX_NUCLEO
  #define MCR_OTA_DOUBLE_WORLD(size) UpdateFWBlueMS(&SizeOFUpdate,StartAddress, size,1)
#elif USE_STM32L4XX_NUCLEO
  #define MCR_OTA_DOUBLE_WORLD(size)                  \
  {                                                   \
    for(counter=0;counter<(size);counter++) {         \
      Pointer[PointerCounter] = StartAddress[counter];\
      PointerCounter++;                               \
      /* Write the Double Word */                     \
      if(PointerCounter==8) {                         \
        UpdateFWBlueMS(&SizeOFUpdate,Pointer, 8,1);   \
        PointerCounter=0;                             \
        ValueToWrite=0;                               \
      }                                               \
    }                                                 \
  }
#endif /* USE_STM32F4XX_NUCLEO */

#ifdef DEBUG_OTA_RECEIVE
    AZURE_PRINTF("message_size=%ld\r\n",message_size);
#endif /* DEBUG_OTA_RECEIVE */

  if(OTAStatus==OTA_STATUS_START) {
    uint8_t *StartAddress=NULL;
    /* First Chuck */
    OTAStatus = OTA_STATUS_RUNNIG;
    SizeOFUpdate =0;

#ifdef DEBUG_OTA_RECEIVE
    AZURE_PRINTF("first message=[%s]\r\n",(char*)data_ptr);
#endif /* DEBUG_OTA_RECEIVE */

    /* Make the parsing of the header */
    StartAddress = (uint8_t *) strstr( (char *) data_ptr, "Content-Length:");
    StartAddress+=16; // for moving to the end of "Content-Length: "

    if(StartAddress!=NULL) {
      SizeOFUpdate = atoi((char *) StartAddress);
      AZURE_PRINTF("OTA size=%ld\r\n",SizeOFUpdate);
    } else {
      AZURE_PRINTF("Error... retriving the OTA Size...\r\n");
      exit(-1);
    }

    /* Check the OTA size */
    if(SizeOFUpdate>OTA_MAX_PROG_SIZE) {
      AZURE_PRINTF("OTA SIZE=%ld > %d Max Allowed\r\n",SizeOFUpdate, OTA_MAX_PROG_SIZE);
      exit(-1);
    }

    /* Move to the beginning of the real OTA */
    StartAddress = (uint8_t *) strstr( (char *) StartAddress, "close");
    if(StartAddress!=NULL) {
      StartAddress += 9;
      uint32_t RealMessageSize= message_size - (StartAddress - data_ptr);
      StartUpdateFWBlueMS(SizeOFUpdate, 0);

#ifdef USE_STM32F4XX_NUCLEO
        /* Save the Meta Data Manager if we need to use also the last 128K flash sector*/
      if(SizeOFUpdate>(0x20000-8)) {
        SaveMetaDataManager();
      }
#endif /* USE_STM32F4XX_NUCLEO */

      MCR_OTA_DOUBLE_WORLD(RealMessageSize);
    } else {
      AZURE_PRINTF("Error... retriving the OTA Starting point...\r\n");
      exit(-1);
    }
  } else if(OTAStatus==OTA_STATUS_RUNNIG) {
    uint8_t *StartAddress= (uint8_t *) data_ptr;
    /* all the remaing chunks */
    if(message_size==505) {
      /* This is a full chunk of data */
      MCR_OTA_DOUBLE_WORLD(message_size);
    } else {
      /* Check if we had received the whole OTA */
      if(!strncmp("\r\n\r\nOK\r\n",(char *)(data_ptr+message_size-8),8)) {
        /* OTA full received */
        /* TMP Workaround ... */
        {
          int32_t MaxSize= ((message_size-8)>SizeOFUpdate) ? SizeOFUpdate : (message_size-8);
          MCR_OTA_DOUBLE_WORLD(MaxSize);
        }
  
        AZURE_PRINTF("\r\n!!!OTA FULL received!!!\r\n\r\n");
        OTAStatus = OTA_STATUS_END;
      } else {
        AZURE_PRINTF("Error receiving the OTA...\r\n");
        AZURE_PRINTF("Last 8 chars are [%s] [%d %d %d %d %c %c %d %d]\r\n",(char *)(data_ptr+message_size-8),                     
                     data_ptr[message_size-8],
                     data_ptr[message_size-7],
                     data_ptr[message_size-6],
                     data_ptr[message_size-5],
                     data_ptr[message_size-4],
                     data_ptr[message_size-3],
                     data_ptr[message_size-2],
                     data_ptr[message_size-1]);
        exit(-1);
      }
    }
  }
}

#else /* AZURE_OTA_HTTP_SOCKET */
void ind_wifi_socket_client_remote_server_closed(uint8_t * socketID)
{
  printf("WIFI closed =%x\r\n",* socketID);
  if(ConnectedToOTA==(*socketID)) {
    ConnectedToOTA=0xFF;
  } else {
    printf("Called Weak Function ind_wifi_socket_client_remote_server_closed\r\n");
  }
}
#endif /* AZURE_OTA_HTTP_SOCKET */
#endif /* AZURE_ENABLE_OTA */

/**
  * @brief  WiFi callback for data received
* @param  SOCKETHANDLE* pSocketHandle : handler for TCP/TLS connection
* @param  uint8_t * data_ptr : pointer to buffer containing data received 
* @param  uint32_t message_size : number of bytes in message received
* @param  uint32_t chunck_size : numeber of bytes in chunk
  * @retval void
  */
void ind_wifi_socket_data_received(uint8_t pSocketHandle,uint8_t * data_ptr, uint32_t message_size, uint32_t chunck_size)
{
#if 0
  AZURE_PRINTF("****** Received buffer for a socket...***** \r\n");
  AZURE_PRINTF("socket received =\r\n");
  AZURE_PRINTF("\tSock=%x\r\n",pSocketHandle);
  AZURE_PRINTF("\tdata=%s\r\n",data_ptr);
  AZURE_PRINTF("\tmessage_size=%d\r\n",message_size);
  AZURE_PRINTF("\tchunck_size=%d\r\n",chunck_size);
#endif

  if(ConnectedToNTP==1) {
    if(message_size==4){
      time_t epochTimeToSetForSystem = SynchronizationAgentConvertNTPTime2EpochTime(data_ptr,message_size);
      if (TimingSystemSetSystemTime(epochTimeToSetForSystem)== 0){
        AZURE_PRINTF("Error Failed to set system time. \r\n");
      } else {
        AZURE_PRINTF("Set UTC Time: %s\r\n",(get_ctime(&epochTimeToSetForSystem)));
        ConnectedToNTP =0;
      }
    }
#ifdef AZURE_OTA_HTTP_SOCKET
  } else if(ConnectedToOTA==pSocketHandle) {
//#define DEBUG_OTA_RECEIVE
  static uint32_t SizeOFUpdate=0;
#ifdef USE_STM32L4XX_NUCLEO
  static uint64_t ValueToWrite=0;
  static uint8_t *Pointer= (uint8_t *) &ValueToWrite;
  static int32_t PointerCounter= 0;
  int32_t counter;
#endif /* USE_STM32L4XX_NUCLEO */

#ifdef USE_STM32F4XX_NUCLEO
  #define MCR_OTA_DOUBLE_WORLD(size) UpdateFWBlueMS(&SizeOFUpdate,StartAddress, size,1)
#elif USE_STM32L4XX_NUCLEO
  #define MCR_OTA_DOUBLE_WORLD(size)                  \
  {                                                   \
    for(counter=0;counter<(size);counter++) {         \
      Pointer[PointerCounter] = StartAddress[counter];\
      PointerCounter++;                               \
      /* Write the Double Word */                     \
      if(PointerCounter==8) {                         \
        UpdateFWBlueMS(&SizeOFUpdate,Pointer, 8,1);   \
        PointerCounter=0;                             \
        ValueToWrite=0;                               \
      }                                               \
    }                                                 \
  }
#endif /* USE_STM32F4XX_NUCLEO */

#ifdef DEBUG_OTA_RECEIVE
    AZURE_PRINTF("chunck_size=%ld SizeOFUpdate=%d\r\n",chunck_size,SizeOFUpdate);
#endif /* DEBUG_OTA_RECEIVE */
  if(OTAStatus==OTA_STATUS_START) {
    uint8_t *StartAddress=NULL;
    /* First Chuck */
    OTAStatus = OTA_STATUS_RUNNIG;
    SizeOFUpdate =0;

#ifdef DEBUG_OTA_RECEIVE
    AZURE_PRINTF("first message=[%s]\r\n",(char*)data_ptr);
#endif /* DEBUG_OTA_RECEIVE */

    /* Make the parsing of the header */
    StartAddress = (uint8_t *) strstr( (char *) data_ptr, "Content-Length:");
    StartAddress+=16; // for moving to the end of "Content-Length: "

    if(StartAddress!=NULL) {
      SizeOFUpdate = atoi((char *) StartAddress);
      AZURE_PRINTF("OTA size=%ld\r\n",SizeOFUpdate);
    } else {
      AZURE_PRINTF("Error... retriving the OTA Size...\r\n");
      UpdateStatus = downloadFailed;
      ReportState(UpdateStatus);
      exit(-1);
    }

    /* Check the OTA size */
    if(SizeOFUpdate>OTA_MAX_PROG_SIZE) {
      AZURE_PRINTF("OTA SIZE=%ld > %d Max Allowed\r\n",SizeOFUpdate, OTA_MAX_PROG_SIZE);
      UpdateStatus = downloadFailed;
      ReportState(UpdateStatus);
      exit(-1);
    }

    /* Move to the beginning of the real OTA */
    StartAddress = (uint8_t *) strstr( (char *) StartAddress, "Type:");
    if(StartAddress!=NULL) {
      StartAddress = (uint8_t *) strstr( (char *) StartAddress, "\r\n");
      if(StartAddress!=NULL) {
        uint32_t RealMessageSize=0;
        StartAddress += 4;
        RealMessageSize= chunck_size - (StartAddress - data_ptr);
#ifdef DEBUG_OTA_RECEIVE
        AZURE_PRINTF("RealMessage:%s\r\n",StartAddress);
#endif /* DEBUG_OTA_RECEIVE */
        StartUpdateFWBlueMS(SizeOFUpdate, 0);

#ifdef USE_STM32F4XX_NUCLEO
          /* Save the Meta Data Manager if we need to use also the last 128K flash sector*/
        if(SizeOFUpdate>(0x20000-8)) {
          SaveMetaDataManager();
        }
#endif /* USE_STM32F4XX_NUCLEO */

        MCR_OTA_DOUBLE_WORLD(RealMessageSize);
      } else {
        AZURE_PRINTF("Error... retriving the OTA Starting point (1)...\r\n");
        UpdateStatus = downloadFailed;
        ReportState(UpdateStatus);
        exit(-1);
      }
    } else {
      AZURE_PRINTF("Error... retriving the OTA Starting point (2)...\r\n");
      UpdateStatus = downloadFailed;
      ReportState(UpdateStatus);
      exit(-1);
    }
  } else if(OTAStatus==OTA_STATUS_RUNNIG) {
    uint8_t *StartAddress= (uint8_t *) data_ptr;
    MCR_OTA_DOUBLE_WORLD(chunck_size);
    /* Check if we had received the whole OTA */
    if(SizeOFUpdate==0) {
      /* OTA full received */
      AZURE_PRINTF("\r\n!!!OTA FULL received!!!\r\n\r\n");
      OTAStatus = OTA_STATUS_END;
    }
  }
#endif /* AZURE_OTA_HTTP_SOCKET */
  } else {
    LocalBufferPushBuffer(&localBufferReading, (void*) data_ptr,chunck_size);
  }
}

void ind_wifi_connected()
{
  AZURE_PRINTF("WiFi connected to AccessPoint\r\n");
  wifi_state = wifi_state_connected;
}

void ind_wifi_on()
{
  AZURE_PRINTF("\r\n\nwifi started and ready...\r\n");
  wifi_state = wifi_state_ready;
}

const IO_INTERFACE_DESCRIPTION* platform_get_default_tlsio(void)
{
  /* TODO: Insert here a call to the tlsio adapter that is available on your platform. */
  return tlsio_STM32Cube_get_interface_description();
}

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
  /* Test if the board is IK01A1 */
  if (BSP_ACCELERO_Init_IKS01A1( ACCELERO_SENSORS_AUTO, &TargetBoardFeatures.HandleAccSensor ) == COMPONENT_OK) {
    AZURE_PRINTF("IKS01A1 board\n\r");
    AZURE_PRINTF("OK Accelero Sensor\n\r");
    TargetBoardFeatures.SnsAltFunc = 0;
  } else {
    TargetBoardFeatures.SnsAltFunc = 1;
    if (BSP_ACCELERO_Init_IKS01A2( ACCELERO_SENSORS_AUTO, &TargetBoardFeatures.HandleAccSensor ) == COMPONENT_OK){
      AZURE_PRINTF("IKS01A2 board\n\r");
      AZURE_PRINTF("OK Accelero Sensor\n\r");
    } else {
      AZURE_PRINTF("IKS01A2 or IKS01A1 board not present, Emulation enabled\n\r");
      TargetBoardFeatures.EmulateSns=1;
    }
  }

  if(!TargetBoardFeatures.EmulateSns) {
    /* DS3/DSM or DS0 */
    /* This section works with IKS01A1 or with IKS01A1/A2 Autodiscovery */
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
      TargetBoardFeatures.HWAdvanceFeatures = 1;
    }

    /* Gyro */
    if((TargetBoardFeatures.SnsAltFunc ? BSP_GYRO_Init_IKS01A2 : BSP_GYRO_Init)( GYRO_SENSORS_AUTO, &TargetBoardFeatures.HandleGyroSensor )==COMPONENT_OK){
      AZURE_PRINTF("OK Gyroscope Sensor\n\r");
    } else {
      AZURE_PRINTF("Error Gyroscope Sensor\n\r");
      while(1);
    }

    if((TargetBoardFeatures.SnsAltFunc ? BSP_MAGNETO_Init_IKS01A2 : BSP_MAGNETO_Init)( MAGNETO_SENSORS_AUTO, &TargetBoardFeatures.HandleMagSensor )==COMPONENT_OK){
      AZURE_PRINTF("OK Magneto Sensor\n\r");
    } else {
      AZURE_PRINTF("Error Magneto Sensor\n\r");
      while(1);
    }

    /* Humidity */
    if((TargetBoardFeatures.SnsAltFunc ? BSP_HUMIDITY_Init_IKS01A2 : BSP_HUMIDITY_Init)( HUMIDITY_SENSORS_AUTO, &TargetBoardFeatures.HandleHumSensor )==COMPONENT_OK){
      AZURE_PRINTF("OK Humidity Sensor\n\r");
    } else {
      AZURE_PRINTF("Error Humidity Sensor\n\r");
    }

    /* Temperature1 */
    if((TargetBoardFeatures.SnsAltFunc ? BSP_TEMPERATURE_Init_IKS01A2 : BSP_TEMPERATURE_Init)( TEMPERATURE_SENSORS_AUTO, &TargetBoardFeatures.HandleTempSensors[TargetBoardFeatures.NumTempSensors] )==COMPONENT_OK){
       AZURE_PRINTF("OK Temperature Sensor1\n\r");
       TargetBoardFeatures.NumTempSensors++;
    } else {
      AZURE_PRINTF("Error Temperature Sensor1\n\r");
    }

    /* Temperature2 */
    if((TargetBoardFeatures.SnsAltFunc ? BSP_TEMPERATURE_Init_IKS01A2 : BSP_TEMPERATURE_Init) (TargetBoardFeatures.SnsAltFunc ? LPS22HB_T_0: LPS25HB_T_0,&TargetBoardFeatures.HandleTempSensors[TargetBoardFeatures.NumTempSensors] )==COMPONENT_OK){
      AZURE_PRINTF("OK Temperature Sensor2\n\r");
      TargetBoardFeatures.NumTempSensors++;
    } else {
      AZURE_PRINTF("Error Temperature Sensor2\n\r");
    }

    /* Pressure */
    if((TargetBoardFeatures.SnsAltFunc ? BSP_PRESSURE_Init_IKS01A2 : BSP_PRESSURE_Init)( PRESSURE_SENSORS_AUTO, &TargetBoardFeatures.HandlePressSensor )==COMPONENT_OK){
      AZURE_PRINTF("OK Pressure Sensor\n\r");
    } else {
      AZURE_PRINTF("Error Pressure Sensor\n\r");
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
  * @brief  Compose message to be transmitted to IoT Hub (date, sensors data, MAC address of the WiFi board) 
  * @param  char* textMsg : pointer to the buffer containing the message to be transmitted to IoT Hub
  * @retval int value for success (1) / failure (0)
  */
int ComposeMessageSensors(char* textMsg)
{
  char   timestr[32];

  float SensorValue;
  int32_t decPartT=0, intPartT=0;
  int32_t decPartH=0, intPartH=0;
  uint8_t Status;
  SensorAxes_t ACC_Value;
  SensorAxes_t GYR_Value;

  RTC_DateTypeDef sdatestructureget;
  RTC_TimeTypeDef stimestructure;

  if(!TargetBoardFeatures.EmulateSns) {
    if((TargetBoardFeatures.SnsAltFunc ? BSP_TEMPERATURE_IsInitialized_IKS01A2 : BSP_TEMPERATURE_IsInitialized)(TargetBoardFeatures.HandleTempSensors[0],&Status)==COMPONENT_OK){
      (TargetBoardFeatures.SnsAltFunc ? BSP_TEMPERATURE_Get_Temp_IKS01A2 : BSP_TEMPERATURE_Get_Temp)(TargetBoardFeatures.HandleTempSensors[0],(float *)&SensorValue);
      MCR_AZURE_F2I_2D(SensorValue, intPartT, decPartT);
    }

    if(TargetBoardFeatures.HandleHumSensor) {
      if((TargetBoardFeatures.SnsAltFunc ? BSP_HUMIDITY_IsInitialized_IKS01A2 : BSP_HUMIDITY_IsInitialized)(TargetBoardFeatures.HandleHumSensor,&Status)==COMPONENT_OK){
        (TargetBoardFeatures.SnsAltFunc ? BSP_HUMIDITY_Get_Hum_IKS01A2 : BSP_HUMIDITY_Get_Hum)(TargetBoardFeatures.HandleHumSensor,(float *)&SensorValue);
        MCR_AZURE_F2I_2D(SensorValue, intPartH, decPartH);
      }
    }

    /* Read the Acc values */
    (TargetBoardFeatures.SnsAltFunc ? BSP_ACCELERO_Get_Axes_IKS01A2 : BSP_ACCELERO_Get_Axes)(TargetBoardFeatures.HandleAccSensor,&ACC_Value);

    /* Read the Gyro values */
    (TargetBoardFeatures.SnsAltFunc ? BSP_GYRO_Get_Axes_IKS01A2 : BSP_GYRO_Get_Axes)(TargetBoardFeatures.HandleGyroSensor,&GYR_Value);
  } else {
    /* Emulatation IkS01A1/IKS01A2 */
    ACC_Value.AXIS_X = 100 + (((int32_t)rand())&0x5FF);
    ACC_Value.AXIS_Y = 300 - (((int32_t)rand())&0x5FF);
    ACC_Value.AXIS_Z = -200 + (((int32_t)rand())&0x5FF);

    GYR_Value.AXIS_X =  30000 + (((int32_t)rand())&0x4FFFF);
    GYR_Value.AXIS_Y =  30000 - (((int32_t)rand())&0x4FFFF);
    GYR_Value.AXIS_Z = -30000 + (((int32_t)rand())&0x4FFFF);

    SensorValue  = 26.0    + (((int16_t)rand())&0x14);
    MCR_AZURE_F2I_2D(SensorValue, intPartT, decPartT);
    SensorValue   = 50.0    + (((int16_t)rand())&0x1F);
    MCR_AZURE_F2I_2D(SensorValue, intPartH, decPartH);
  }

  HAL_RTC_GetTime(&TargetBoardFeatures.RtcHandle, &stimestructure, FORMAT_BIN);
  HAL_RTC_GetDate(&TargetBoardFeatures.RtcHandle, &sdatestructureget, FORMAT_BIN);

  if ((sprintf_s(timestr, sizeof(timestr),
                 "20%02d-%02d-%02dT%02d:%02d:%02d.000000Z",
                 sdatestructureget.Year, sdatestructureget.Month,
                 sdatestructureget.Date,stimestructure.Hours,
                 stimestructure.Minutes, stimestructure.Seconds)) < 0) {
    AZURE_PRINTF("[IoTHub][E]. Failed to compose message \r\n");
    return 0;
  }

  if( (sprintf_s((char *)textMsg, 1024,
                  "{\"id\":\"%s\",\"name\":\"Nucleo-%s\","
                  "\"ts\":\"%s\",\"mtype\":\"ins\","
                  "\"temp\":%ld.%ld,\"hum\":%ld.%ld,"
                  "\"accX\":%ld,\"accY\":%ld,\"accZ\":%ld,"
                  "\"gyrX\":%ld,\"gyrY\":%ld,\"gyrZ\":%ld}",
                  "0080E1B8A9E2",
                  "0080E1B8A9E2",
                  timestr,
                  intPartT, decPartT,
                  intPartH, decPartH,
                  ACC_Value.AXIS_X, ACC_Value.AXIS_Y, ACC_Value.AXIS_Z,
                  GYR_Value.AXIS_X, GYR_Value.AXIS_Y, GYR_Value.AXIS_Z ) ) < 0){
    AZURE_PRINTF("[IoTHub][E]. Failed to compose message. \r\n");
    return 0;
  }
  return 1; 
}


/**
  * @brief  Send Notification where there is a interrupt from MEMS
  * @param  None
  * @retval None
  */
void MEMSCallback(void)
{
  uint8_t stat = 0;  

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

#ifdef USE_STM32F4XX_NUCLEO
/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow:
  *            System Clock source            = PLL (HSI)
  *            SYSCLK(Hz)                     = 84000000
  *            HCLK(Hz)                       = 84000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 2
  *            APB2 Prescaler                 = 1
  *            HSI Frequency(Hz)              = 16000000
  *            PLL_M                          = 16
  *            PLL_N                          = 336
  *            PLL_P                          = 4
  *            PLL_Q                          = 7
  *            VDD(V)                         = 3.3
  *            Main regulator output voltage  = Scale2 mode
  *            Flash Latency(WS)              = 2
  * @param  None
  * @retval None
  */
static void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;

  /* Enable Power Control clock */
  __PWR_CLK_ENABLE();

  /* The voltage scaling allows optimizing the power consumption when the device is
     clocked below the maximum system frequency, to update the voltage scaling value
     regarding system frequency refer to product datasheet.  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /* Enable HSI Oscillator and activate PLL with HSI as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = 0x10;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK){
    STM32_Error_Handler();
  }

  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK){
    STM32_Error_Handler();
  }
}
#endif /* USE_STM32F4XX_NUCLEO */

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
    __HAL_TIM_SET_COMPARE(&TimCCHandle, TIM_CHANNEL_1, (uhCapture + DEFAULT_uhCCR1_Val));
    SendData=1;
  } else if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2) {
    uhCapture = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2);
    /* Set the Capture Compare Register value */
    __HAL_TIM_SET_COMPARE(&TimCCHandle, TIM_CHANNEL_2, (uhCapture + DEFAULT_uhCCR2_Val));
    ReadProx=1;
  }
}


/**
* @brief  Function for initializing timers for sending the information to BLE:
 *  - MotionFX/AR/CP and Acc/Gyro/Mag
 *  - Environmental info
 * @param  None
 * @retval None
 */
static void InitTimers(void)
{
  uint32_t uwPrescalerValue;

  /* Timer Output Compare Configuration Structure declaration */
  TIM_OC_InitTypeDef sConfig;

  /* Compute the prescaler value to have TIM3 counter clock equal to 10 KHz */
  uwPrescalerValue = (uint32_t) ((SystemCoreClock / 10000) - 1); 

  /* Set TIM1 instance (Motion)*/
  /* Set TIM1 instance */
#if ((defined (USE_STM32F4XX_NUCLEO)) || (defined (USE_STM32L4XX_NUCLEO)))
  TimCCHandle.Instance = TIM1;
#endif
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
  sConfig.Pulse = DEFAULT_uhCCR1_Val;
  if(HAL_TIM_OC_ConfigChannel(&TimCCHandle, &sConfig, TIM_CHANNEL_1) != HAL_OK) {
    /* Configuration Error */
    STM32_Error_Handler();
  }

  /* Output Compare Toggle Mode configuration: Channel2 */
  sConfig.Pulse = DEFAULT_uhCCR2_Val;
  if(HAL_TIM_OC_ConfigChannel(&TimCCHandle, &sConfig, TIM_CHANNEL_2) != HAL_OK) {
    /* Configuration Error */
    Error_Handler();
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
   case M_INT1_PIN:
   case LSM6DSL_INT1_O_PIN:
    MEMSInterrupt=1;
    break;
  }
}


/**
* @brief  Period elapsed callback in non blocking mode
*         This timer is used for calling back User registered functions with information
* @param  htim : TIM handle
* @retval None
*/
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  Wifi_TIM_Handler(htim);  
}

/**
  * @brief  Execute the Firmware update after the Command FOTA
  * example: FOTA,192.168.43.1,12345,/HelloL4_WithBL.bin
  * @param  char *hostname Server address
  * @param  uint32_t port_num Server port number
  * @param  char *path FOTA path
  * @retval None
  */
void FOTACallback(char * hostname,uint32_t  port_num,char * path)
{
  WiFi_Status_t status = WiFi_MODULE_SUCCESS;
#ifdef AZURE_ENABLE_OTA
  //StopTimer1();
  //StopTimer2();
  UpdateStatus = downloading;
  ReportState(UpdateStatus);
#ifdef AZURE_OTA_HTTP_SOCKET
  {
    char    buf[256];
    uint8_t socketHandle;

     AZURE_PRINTF("TestOTA HostName=[%s] port=[%ld] File=[%s]\r\n",hostname,port_num,path);

    /* Flag for understanding that we are starting one OTA */
    OTAStatus=OTA_STATUS_START;
    status = wifi_socket_client_open((uint8_t*)hostname, port_num, "t", &socketHandle);
    if(status!=WiFi_MODULE_SUCCESS) {
      AZURE_PRINTF("Error opening socket with OTA server\r\n");
      UpdateStatus = downloadFailed;
      ReportState(UpdateStatus);
      return;
    } else {
      AZURE_PRINTF("WiFi opened socket with OTA server [%s]\r\n",hostname);
      ConnectedToOTA=socketHandle;
    }

    /* Prepare the HTTP Get request */
    sprintf(buf,"GET %s HTTP/1.1\r\n",path);

    /* Send the request */
    status = wifi_socket_client_write(socketHandle, strlen(buf),buf);
    if(status!=WiFi_MODULE_SUCCESS) {
      AZURE_PRINTF("Error sending the HTTP GET to OTA server (1/3)\r\n");
      UpdateStatus = downloadFailed;
      ReportState(UpdateStatus);
      return;
    } else {
      AZURE_PRINTF("HTTP GET request sent to OTA server (1/3)\r\n");
    }

    sprintf(buf,"Host: %s\r\n",hostname);
    /* Send the request */
    status = wifi_socket_client_write(socketHandle, strlen(buf),buf);
    if(status!=WiFi_MODULE_SUCCESS) {
      AZURE_PRINTF("Error sending the HTTP GET to OTA server (2/3)\r\n");
      UpdateStatus = downloadFailed;
      ReportState(UpdateStatus);
      return;
    } else {
      AZURE_PRINTF("HTTP GET request sent to OTA server (2/3)\r\n");
    }

    sprintf(buf,"\r\n");
    /* Send the request */
    status = wifi_socket_client_write(socketHandle, strlen(buf),buf);
    if(status!=WiFi_MODULE_SUCCESS) {
      AZURE_PRINTF("Error sending the HTTP GET to OTA server (3/3)\r\n");
      UpdateStatus = downloadFailed;
      ReportState(UpdateStatus);
      return;
    } else {
      AZURE_PRINTF("HTTP GET request sent to OTA server (3/3)\r\n");
    }

    /* Wait the end of OTA */
    while(OTAStatus != OTA_STATUS_END) {
      HAL_Delay(100);
    }
    OTAStatus = OTA_STATUS_NULL;

    status = wifi_socket_client_close(socketHandle);
    if(status!=WiFi_MODULE_SUCCESS) {
      AZURE_PRINTF("Error closing socket with OTA server\r\n");
      return;
    } else {
      AZURE_PRINTF("WiFi closed socket with OTA server\r\n");
    }
  }
#else /* AZURE_OTA_HTTP_SOCKET */
  {
     AZURE_PRINTF("TestOTA HostName=[%s] port=[%d] File=[%s]\r\n",hostname,port_num,path);

    /* Flag for understanding that we are starting one OTA */
    OTAStatus=OTA_STATUS_START;

    status = wifi_http_get((uint8_t *)hostname, (uint8_t *)path, port_num);
    if(status == WiFi_MODULE_SUCCESS){
      AZURE_PRINTF("\r\nHTTP GET OK\r\n");
    } else {
      AZURE_PRINTF("\r\nHTTP GET Error\r\n");
      UpdateStatus = downloadFailed;
      ReportState(UpdateStatus);
    }
  }

  /* Wait the end of OTA */
  while(OTAStatus != OTA_STATUS_END) {
    HAL_Delay(100);
  }
  OTAStatus = OTA_STATUS_NULL;
#endif /* AZURE_OTA_HTTP_SOCKET */
  
  AZURE_PRINTF("The Board will restart in 5 seconds for Appling the OTA\r\n");
  UpdateStatus = downloadComplete;
  ReportState(UpdateStatus);

  HAL_Delay(2000);

  UpdateStatus = applying;
  ReportState(UpdateStatus);

  HAL_Delay(2000);
  HAL_NVIC_SystemReset();
#endif /* AZURE_ENABLE_OTA */
}


/**
  * @brief This function provides accurate delay (in milliseconds) based
  *        on variable incremented.
  * @note This is a user implementation using WFI state
  * @param Delay: specifies the delay time length, in milliseconds.
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
#if (defined USE_STM32F4XX_NUCLEO) || (defined USE_STM32L4XX_NUCLEO) || (defined USE_STM32F3XX_NUCLEO) || \
     (defined USE_STM32L1XX_NUCLEO) || (defined USE_STM32F1XX_NUCLEO)
  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 3, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
#elif (defined USE_STM32L0XX_NUCLEO) || (defined USE_STM32F0XX_NUCLEO)
  HAL_NVIC_SetPriority(EXTI4_15_IRQn, 3, 0);
  HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);
#endif

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

#ifdef USE_STM32F4XX_NUCLEO
/**
 * @brief User function for Erasing the Flash data for MDM
 * @param None
 * @retval uint32_t Success/NotSuccess [1/0]
 */
uint32_t UserFunctionForErasingFlash(void) {
  FLASH_EraseInitTypeDef EraseInitStruct;
  uint32_t SectorError = 0;
  uint32_t Success=1;

  EraseInitStruct.TypeErase = TYPEERASE_SECTORS;
  EraseInitStruct.VoltageRange = VOLTAGE_RANGE_3;
  EraseInitStruct.Sector = FLASH_SECTOR_7;
  EraseInitStruct.NbSectors = 1;

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
  uint32_t *WriteIndex;

  /* Unlock the Flash to enable the flash control register access *************/
  HAL_FLASH_Unlock();

  for(WriteIndex =((uint32_t *) InitMetaDataVector); WriteIndex<((uint32_t *) EndMetaDataVector); WriteIndex++) {
    if (HAL_FLASH_Program(TYPEPROGRAM_WORD, Address,*WriteIndex) == HAL_OK){
      Address = Address + 4;
    } else {
      /* Error occurred while writing data in Flash memory.
         User can add here some code to deal with this error
         FLASH_ErrorTypeDef errorcode = HAL_FLASH_GetError(); */
      STM32_Error_Handler();
      Success=0;
    }
  }

  /* Lock the Flash to disable the flash control register access (recommended
   to protect the FLASH memory against possible unwanted operation) *********/
  HAL_FLASH_Lock();
 
  return Success;
}
#endif /* USE_STM32F4XX_NUCLEO */
