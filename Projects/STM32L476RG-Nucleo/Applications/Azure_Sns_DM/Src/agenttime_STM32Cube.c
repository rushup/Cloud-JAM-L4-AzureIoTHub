/**
  ******************************************************************************
  * @file    agenttime_STM32Cube.c
  * @author  Central LAB
  * @version V3.0.0
  * @date    21-April-2017
  * @brief   Adapter call the standard C functions
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
#include "azure_c_shared_utility/gballoc.h"
#include <time.h>
#include "azure_c_shared_utility/agenttime.h"

/**
 * @brief implementation of get_time
 * @param time_t* p pointer to the time_t structure
 * @retval time_t return time_t structure
 */
time_t get_time(time_t* p)
{
  return time(p);
}

/**
 * @brief implementation of get_gmtime
 * @param time_t* currentTime pointer to the time_t structure
 * @retval tm* return tm structure
 */
struct tm* get_gmtime(time_t* currentTime)
{
  return gmtime(currentTime);
}

/**
 * @brief implementation of get_mktime
 * @param tm* cal_time pointer to the tm structure
 * @retval time_t return time_t structure
 */
time_t get_mktime(struct tm* cal_time)
{
  return mktime(cal_time);
}

/**
 * @brief implementation of get_ctime
 * @param time_t* timeToGet pointer to the time_t structure
 * @retval char* return value
 */
char* get_ctime(time_t* timeToGet)
{
  return ctime(timeToGet);
}

/**
 * @brief implementation of get_difftime
 * @param time_t stopTime end time
 * @param time_t startTime starting time
 * @retval double difference
 */
double get_difftime(time_t stopTime, time_t startTime)
{
  return difftime(stopTime, startTime);
}
/******************* (C) COPYRIGHT 2017 STMicroelectronics *****END OF FILE****/
