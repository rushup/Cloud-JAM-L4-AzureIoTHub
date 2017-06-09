   /**
  ******************************************************************************
  * @file    TLocalBuffer.h
  * @author  Central LAB
  * @version V1.0.0
  * @date    17-Oct-2015
  * @brief   Queue implementation to interface SPWF WiFi callbacks with MQTT read functions
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

#ifndef __TLOCALBUFFER_H
#define __TLOCALBUFFER_H

#include <stdint.h>

#define TLOCALBUFFER_SIZE   1024

#define TLOCALBUFFER_SAFEMODE
#define TLOCALBUFFER_USEONESEMAPHORE

typedef struct __TLocalBuffer_t
{
    uint8_t 		Buffer[TLOCALBUFFER_SIZE];
    uint8_t		Semaphore;
    uint32_t 		PopIndex;
    uint32_t 		PushIndex;
    uint32_t		SizeBuffer;
}TLocalBuffer;

void LocalBufferInit(TLocalBuffer* localBuffer);
void LocalBufferFlush(TLocalBuffer* localBuffer);
uint8_t LocalBufferPopBuffer(TLocalBuffer* localBuffer,void *OutPut,int Size);
uint8_t LocalBufferPushBuffer(TLocalBuffer* localBuffer,void* Data,int Size);
uint8_t LocalBufferPopAllBuffer(TLocalBuffer* localBuffer,void* OutPut);
uint32_t	LocalBufferGetSizeBuffer(TLocalBuffer* localBuffer);
uint32_t        LocalBufferGetResidueSize(TLocalBuffer* localBuffer);

extern TLocalBuffer localBufferReading;

#endif
 
