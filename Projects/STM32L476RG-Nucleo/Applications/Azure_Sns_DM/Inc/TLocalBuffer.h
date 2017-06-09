   /**
  ******************************************************************************
  * @file    TLocalBuffer.h
  * @author  Central LAB
  * @version V3.0.0
  * @date    21-April-2017
  * @brief   Header file for TLocalBuffer.c
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

/* 0x3FF = 1023 */
#define TLOCALBUFFER_MASK_SIZE 0x3FF

#define TLOCALBUFFER_SIZE   (TLOCALBUFFER_MASK_SIZE+1)

typedef struct __TLocalBuffer_t
{
    uint8_t 		Buffer[TLOCALBUFFER_SIZE];
    int32_t 		PopIndex;
    int32_t 		PushIndex;
    int32_t		SizeBuffer;
}TLocalBuffer;

void LocalBufferInit(TLocalBuffer* localBuffer);
int8_t LocalBufferPopBuffer(TLocalBuffer* localBuffer,void *OutPut,int Size);
int8_t LocalBufferPushBuffer(TLocalBuffer* localBuffer,void* Data,int Size);
int32_t LocalBufferGetSizeBuffer(TLocalBuffer* localBuffer);

extern TLocalBuffer localBufferReading[2];

#endif /* __TLOCALBUFFER_H */
/******************* (C) COPYRIGHT 2017 STMicroelectronics *****END OF FILE****/
 
