/**
 ******************************************************************************
 * @file    STM32CubeRTCInterface.c
 * @author  Central LAB
 * @version V3.0.0
 * @date    21-April-2017
 * @brief   Implementation of RTC interface
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
 
/* Includes ------------------------------------------------------------------*/
#include "TargetFeatures.h"
#include "STM32CubeInterface.h"

#define CONVERSION_EPOCHFACTOR  2208988800ul

static volatile time_t          timeSyncSystem = 1446741778;

const int yytab[2][12] = {
  { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 },
  { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }
};

/**
 * @brief  Set RTC time
* @param  time_t epochTimeNow : Epoch Time
 * @retval int value for success(1)/failure(0)
 */
int TimingSystemSetSystemTime(time_t epochTimeNow)
{
  RTC_DateTypeDef sdatestructure;
  RTC_TimeTypeDef stimestructure;

  time_t now = epochTimeNow;
  struct tm *calendar = gmtime(&now);
  if (calendar == NULL) {
    return 0;
  }
  
  /* Configure the Date */
  sdatestructure.Year = calendar->tm_year - 100;
  sdatestructure.Month = calendar->tm_mon + 1;
  sdatestructure.Date = calendar->tm_mday;
  sdatestructure.WeekDay = calendar->tm_wday + 1;
  if(HAL_RTC_SetDate(&TargetBoardFeatures.RtcHandle,&sdatestructure,FORMAT_BIN) != HAL_OK) {
    /* Initialization Error */
    return 0;
  }

  /* Configure the Time */
  stimestructure.Hours                = calendar->tm_hour;
  stimestructure.Minutes              = calendar->tm_min;
  stimestructure.Seconds              = calendar->tm_sec;
  stimestructure.TimeFormat           = RTC_HOURFORMAT12_AM;
  stimestructure.DayLightSaving       = RTC_DAYLIGHTSAVING_NONE;
  stimestructure.StoreOperation       = RTC_STOREOPERATION_RESET;

  if(HAL_RTC_SetTime(&TargetBoardFeatures.RtcHandle,&stimestructure,FORMAT_BIN) != HAL_OK) {
    return 0;
  }

  timeSyncSystem      = epochTimeNow;

  gmtime(&epochTimeNow);

  /* Writes a data in a RTC Backup data Register0 */
  HAL_RTCEx_BKUPWrite(&TargetBoardFeatures.RtcHandle,RTC_BKP_DR0,0x32F2);

  return 1;
}

/**
* @brief  Display the Time on format hh:mm:ss mm-dd-yy
* @param  uint8_t* showtime pointer to the string where save the time
* @param  uint8_t* showdate pointer to the string where save the date
* @retval None
 */
void RTC_CalendarShow(uint8_t *showtime, uint8_t *showdate)
{
  RTC_DateTypeDef sdatestructureget;
  RTC_TimeTypeDef stimestructureget;

  /* Get the RTC current Time */
  HAL_RTC_GetTime(&TargetBoardFeatures.RtcHandle, &stimestructureget, RTC_FORMAT_BIN);
  /* Get the RTC current Date */
  HAL_RTC_GetDate(&TargetBoardFeatures.RtcHandle, &sdatestructureget, RTC_FORMAT_BIN);

#if 1
  /* Display time Format: hh:mm:ss */
  sprintf((char*)showtime,"%02d:%02d:%02d",stimestructureget.Hours, stimestructureget.Minutes, stimestructureget.Seconds);
  /* Display date Format: mm-dd-yy */
  sprintf((char*)showdate,"%02d-%02d-%02d",sdatestructureget.Month, sdatestructureget.Date, 2000 + sdatestructureget.Year);
#endif
}

/**
 * @brief  Get RTC time
* @param  void
* @retval time_t : time retrieved from RTC
 */
time_t 	TimingSystemGetSystemTime(void)
{
  time_t        returnTime;
  struct tm*    pCalendar;
  
  RTC_DateTypeDef sdatestructure;
  RTC_TimeTypeDef stimestructure;
  returnTime = 0;  
  pCalendar             = gmtime((const time_t*)&timeSyncSystem);

  if((HAL_RTC_GetTime(&TargetBoardFeatures.RtcHandle,&stimestructure,FORMAT_BIN)==HAL_OK) &&
     (HAL_RTC_GetDate(&TargetBoardFeatures.RtcHandle,&sdatestructure,FORMAT_BIN)==HAL_OK)) {
    pCalendar->tm_year           = sdatestructure.Year +100;
    pCalendar->tm_mon            = sdatestructure.Month-1;
    pCalendar->tm_mday           = sdatestructure.Date;
    pCalendar->tm_wday           = sdatestructure.WeekDay - 1;
    pCalendar->tm_hour           = stimestructure.Hours;
    pCalendar->tm_min            = stimestructure.Minutes;
    pCalendar->tm_sec            = stimestructure.Seconds;
    pCalendar->tm_isdst          = 0;
#if  _DLIB_SUPPORT_FOR_AEABI
    pCalendar->__BSD_bug_filler1 = 0;
    pCalendar->__BSD_bug_filler2 = 0;
#endif
    returnTime        = mktime(pCalendar);
    timeSyncSystem    = returnTime;
  }
  return returnTime;
}

/**
 * @brief  Convert NTP time to epoch time
* @param  uint8_t* pBufferTimingAnswer : pointer to buffere containing the NTP date
* @param  size_t sizeBuffer : size of buffer
* @retval time_t : epoch time after conversion
 */
time_t SynchronizationAgentConvertNTPTime2EpochTime(uint8_t* pBufferTimingAnswer,size_t sizeBuffer)
{
  uint32_t      valueNumericExtracted;
  time_t        epochTime;

  epochTime = (time_t)-1;
  
  if(sizeBuffer>=4) {
    valueNumericExtracted   = ((pBufferTimingAnswer[0]<<24 )|(pBufferTimingAnswer[1]<<16)|(pBufferTimingAnswer[2]<<8)| pBufferTimingAnswer[3]);
    epochTime               = (time_t)(valueNumericExtracted-CONVERSION_EPOCHFACTOR);
  }
  
  return epochTime;
}
/******************* (C) COPYRIGHT 2017 STMicroelectronics *****END OF FILE****/
