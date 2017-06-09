 /**
  ******************************************************************************
  * @file    timingSystem.h
  * @author  Central LAB
  * @version V1.1.0
  * @date    08-August-2016
  * @brief   Header file for timingSystem.c
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


#ifndef __TIMING_AGENT_H
#define __TIMING_AGENT_H

#include <time.h>

extern void    TimingSystemInitialize(void);
extern int 	TimingSystemSetSystemTime(time_t epochTimeNow);
extern time_t 	TimingSystemGetSystemTime(void);

#define YYEAR0           1900                    /* the first year */
#define EEPOCH_YR        1970            /* EPOCH = Jan 1 1970 00:00:00 */
#define SSECS_DAY        (24L * 60L * 60L)
#define LLEAPYEAR(year)  (!((year) % 4) && (((year) % 100) || !((year) % 400)))
#define YYEARSIZE(year)  (LLEAPYEAR(year) ? 366 : 365)

// MDK
extern struct tm *gmtimeMDK(register const time_t *timer);

#endif

/******************* (C) COPYRIGHT 2013 STMicroelectronics *****END OF FILE****/
