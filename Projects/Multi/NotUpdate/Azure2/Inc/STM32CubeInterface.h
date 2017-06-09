/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32CUBEINTERFACE_H
#define __STM32CUBEINTERFACE_H

/* Includes ------------------------------------------------------------------*/
#include "TargetFeatures.h"
#include "HWAdvanceFeatures.h"
#include "wifi_globals.h"
#include "wifi_interface.h"
#include <time.h>

/* Exported Types ------------------------------------------------------- */
typedef enum {
  wifi_state_reset = 0,
  wifi_state_ready,
  wifi_state_idle,
  wifi_state_connected,
  wifi_state_connecting,
  wifi_state_disconnected,
  wifi_state_activity,
  wifi_state_inter,
  wifi_state_print_data,
  wifi_state_error,
  wifi_undefine_state       = 0xFF,
} wifi_state_t;

typedef struct {
  char ssid[40];
  char seckey[40];
  WiFi_Priv_Mode mode;
} WIFI_CredAcc_t;

/* Exported variables ------------------------------------------------------- */
extern __IO wifi_state_t wifi_state;

/* Exported functions ------------------------------------------------------- */
extern void RTC_CalendarShow(uint8_t *showtime, uint8_t *showdate);
extern time_t 	TimingSystemGetSystemTime(void);
extern int TimingSystemSetSystemTime(time_t epochTimeNow);
extern time_t SynchronizationAgentConvertNTPTime2EpochTime(uint8_t* pBufferTimingAnswer,size_t sizeBuffer);

extern void StartTimer1(void);
extern void StopTimer1(void);
extern void StartTimer2(void);
extern void StopTimer2(void);
extern void ReadSingle53L0X(void);
extern void FOTACallback(char * hostname,uint32_t  port_num,char * path);
extern void ButtonCallback(void);
extern void MEMSCallback(void);

#endif /* __STM32CUBEINTERFACE_H */
