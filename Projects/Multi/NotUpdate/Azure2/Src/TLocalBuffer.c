   /**
  ******************************************************************************
  * @file    TLocalBuffer.c
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

#include <string.h>
#include "TLocalBuffer.h"
#include "STM32CubeInterface.h"

//#define DEBUG_TLOCAL_BUFFER

    
TLocalBuffer localBufferReading;

uint32_t LocalBufferGetSizeBuffer(TLocalBuffer* localBuffer)
{
#ifdef DEBUG_TLOCAL_BUFFER
  AZURE_PRINTF("--<<LocalBufferGetSizeBuffer size=%d\r\n",localBuffer->SizeBuffer);
#endif /* DEBUG_TLOCAL_BUFFER */
  return localBuffer->SizeBuffer;
}

uint32_t        LocalBufferGetResidueSize(TLocalBuffer* localBuffer)
{
#ifdef DEBUG_TLOCAL_BUFFER
  AZURE_PRINTF("<<LocalBufferGetResidueSize size=%d\r\n",localBuffer->SizeBuffer);
#endif /* DEBUG_TLOCAL_BUFFER */
  return TLOCALBUFFER_SIZE-localBuffer->SizeBuffer;
}

/**
*/
void LocalBufferInit(TLocalBuffer* localBuffer)
{
#ifdef DEBUG_TLOCAL_BUFFER
  AZURE_PRINTF("--<<LocalBufferInit\r\n");
#endif /* DEBUG_TLOCAL_BUFFER */
  memset(localBuffer,0,sizeof(TLocalBuffer));
}

/**
*/
uint8_t LocalBufferPopBuffer(TLocalBuffer* localBuffer,void* Buffer,int Size)
{
#ifdef DEBUG_TLOCAL_BUFFER
  AZURE_PRINTF("--<<LocalBufferPopBuffer size=%d\r\n",Size);
#endif /* DEBUG_TLOCAL_BUFFER */

  while(localBuffer->Semaphore)
    HAL_Delay(1);

  if((localBuffer->PushIndex-localBuffer->PopIndex)<(uint32_t)Size || Size==0)
    return 0;

  localBuffer->Semaphore = 1;
  
  memcpy(Buffer,localBuffer->Buffer+localBuffer->PopIndex,Size);
  
  localBuffer->PopIndex = (localBuffer->PopIndex+Size)%TLOCALBUFFER_SIZE;

  #ifdef TLOCALBUFFER_SAFEMODE
  if(localBuffer->PopIndex==localBuffer->PushIndex)
  {
    memset(localBuffer->Buffer,0,localBuffer->PopIndex);
    localBuffer->PopIndex	        = 0;
    localBuffer->PushIndex	= 0;
  }
  #endif
  localBuffer->SizeBuffer = localBuffer->PushIndex>=localBuffer->PopIndex? localBuffer->PushIndex-localBuffer->PopIndex:TLOCALBUFFER_SIZE-localBuffer->PopIndex+localBuffer->PushIndex;

  localBuffer->Semaphore = 0;

  return 1;
}

/**
*/
uint8_t LocalBufferPopAllBuffer(TLocalBuffer* localBuffer,void* OutPut)
{
#ifdef DEBUG_TLOCAL_BUFFER
  AZURE_PRINTF("--<<LocalBufferPopAllBuffer\r\n");
#endif /* DEBUG_TLOCAL_BUFFER */
  return LocalBufferPopBuffer(localBuffer,OutPut,localBuffer->PushIndex);
}

/**
*/
uint8_t LocalBufferPushBuffer(TLocalBuffer* localBuffer, void* Data,int Size)
{
#ifdef DEBUG_TLOCAL_BUFFER
  AZURE_PRINTF("--<<LocalBufferPushBuffer size=%d\r\n",Size);
#endif /* DEBUG_TLOCAL_BUFFER */
  if(TLOCALBUFFER_SIZE-(localBuffer->PushIndex-localBuffer->PopIndex)<(uint32_t)Size)
      return 0;

  while(localBuffer->Semaphore)
      HAL_Delay(1);

  localBuffer->Semaphore = 1;
  
  memcpy(localBuffer->Buffer+localBuffer->PushIndex,Data,Size);
  localBuffer->PushIndex = (localBuffer->PushIndex+Size)%TLOCALBUFFER_SIZE;

  localBuffer->SizeBuffer = localBuffer->PushIndex>=localBuffer->PopIndex? localBuffer->PushIndex-localBuffer->PopIndex:TLOCALBUFFER_SIZE-localBuffer->PopIndex+localBuffer->PushIndex;

  localBuffer->Semaphore = 0;

  return 1;
}
