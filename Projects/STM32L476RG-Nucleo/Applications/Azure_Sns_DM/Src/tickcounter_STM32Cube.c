/**
  ******************************************************************************
  * @file    tickcounter_STM32Cube.c
  * @author  Central LAB
  * @version V3.0.0
  * @date    21-April-2017
  * @brief   Tick Counter adapter 
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
#include <stdint.h>
#include "HWAdvanceFeatures.h"
#include "azure_c_shared_utility/gballoc.h"
#include "azure_c_shared_utility/tickcounter.h"
#include "azure_c_shared_utility/xlogging.h"

typedef struct TICK_COUNTER_INSTANCE_TAG
{
	int dummy : 1;
} TICK_COUNTER_INSTANCE;

/**
 * @brief Function for allocating the Tick Counter Structure
 * @param None
 * @retval TICK_COUNTER_HANDLE pointer to the allocated Tick Counter structure
 */
TICK_COUNTER_HANDLE tickcounter_create(void)
{
  TICK_COUNTER_INSTANCE* result = (TICK_COUNTER_INSTANCE*)malloc(sizeof(TICK_COUNTER_INSTANCE));
  if (result == NULL) {
    /* add any per instance initialization (starting a timer for example) here if needed (most platforms will not need this) */
    LogError("Cannot create tick counter");
  }
  return result;
}

/**
 * @brief Function for deallocating the Tick Counter Structure
 * @param TICK_COUNTER_HANDLE pointer to the allocated Tick Counter structure
 * @retval None
 */
void tickcounter_destroy(TICK_COUNTER_HANDLE tick_counter)
{
  if (tick_counter != NULL) {
    /* add any per instance de-initialization here (stopping the time) if needed (most platforms will not need this) */
    free(tick_counter);
  }
}

/**
 * @brief Function for geting the Tick counter values in milliseconds
 * @param TICK_COUNTER_HANDLE pointer to the allocated Tick Counter structure
 * @param tickcounter_ms_t* current_ms Tick counter value in milliseconds
 * @retval int OK/Error (0/!=0)
 */
int tickcounter_get_current_ms(TICK_COUNTER_HANDLE tick_counter, tickcounter_ms_t* current_ms)
{
  int result =0;
  if (tick_counter == NULL) {
    LogError("tickcounter failed: Invalid Arguments.");
    result = __LINE__;
  } else {
    /* call here any platform/OS specific function to get the millisecond tick and perform any conversions */
    /* Important: The HAL_GetTick return one uint32_t Values .. so after 49 days it will restart */
    *current_ms = HAL_GetTick();
  }
  return result;
}
/******************* (C) COPYRIGHT 2017 STMicroelectronics *****END OF FILE****/

