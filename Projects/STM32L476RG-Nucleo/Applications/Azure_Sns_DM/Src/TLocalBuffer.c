/**
  ******************************************************************************
  * @file    TLocalBuffer.c
  * @author  Central LAB
  * @version V3.0.0
  * @date    21-April-2017
  * @brief   Buffer to interface SPWF WiFi callbacks with mbedTLS read functions
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
#include <string.h>
#include "TLocalBuffer.h"
#include "STM32CubeInterface.h"

//#define DEBUG_TLOCAL_BUFFER

//#define TLOCAL_BUFFER_USE_FOR_CYCLES
    
TLocalBuffer localBufferReading[2];

/**
 * @brief Return the size of the buffer
 * @param TLocalBuffer* localBuffer pointer to the local buffer structure
 * @retval int32_t Size of the buffer
 */
int32_t LocalBufferGetSizeBuffer(TLocalBuffer* localBuffer)
{
#ifdef DEBUG_TLOCAL_BUFFER
  AZURE_PRINTF("====LocalBufferGetSizeBuffer size=%d\r\n",localBuffer->SizeBuffer);
#endif /* DEBUG_TLOCAL_BUFFER */
  return localBuffer->SizeBuffer;
}

/**
 * @brief Local Buffer initialization
 * @param TLocalBuffer* localBuffer pointer to the local buffer structure
 * @retval None
 */
void LocalBufferInit(TLocalBuffer* localBuffer)
{
  localBuffer->PopIndex = localBuffer->PushIndex = localBuffer->SizeBuffer = 0;
#ifdef DEBUG_TLOCAL_BUFFER
  AZURE_PRINTF(">><<LocalBufferInit\r\n");
#endif /* DEBUG_TLOCAL_BUFFER */
}

/**
 * @brief Extract bytes from Buffer 
 * @param TLocalBuffer* localBuffer pointer to the local buffer structure
 * @param void* Buffer destination buffer
 * @param int Size Number of byte to extract
 * @retval int8_t Ok/Error (1/0)
 */
int8_t LocalBufferPopBuffer(TLocalBuffer* localBuffer,void* Buffer,int Size)
{
#ifdef DEBUG_TLOCAL_BUFFER
  AZURE_PRINTF("--<<LocalBufferPopBuffer size=%d [pop=%d push=%d]\r\n",Size,localBuffer->PopIndex,localBuffer->PushIndex);
#endif /* DEBUG_TLOCAL_BUFFER */

  /* Protect this region from here.... */
  __disable_irq();

  if((localBuffer->SizeBuffer<Size) | (Size==0)) {
    __enable_irq();
    return 0;
  }
  
  /* Pop the Data */
  if(Size) {
    uint8_t *BuffPointer = (uint8_t *) Buffer;
#ifdef TLOCAL_BUFFER_USE_FOR_CYCLES
    int32_t Count;
    for(Count=0; Count<Size; Count++) {
      BuffPointer[Count] = localBuffer->Buffer[localBuffer->PopIndex];
      localBuffer->PopIndex++;
      localBuffer->PopIndex&=TLOCALBUFFER_MASK_SIZE;
    }
    localBuffer->SizeBuffer -= Size;
#else /* TLOCAL_BUFFER_USE_FOR_CYCLES */
    /* Copy first Part */
    int32_t FromPointerToEndSize;
    int32_t MinSize;
    FromPointerToEndSize = TLOCALBUFFER_SIZE - localBuffer->PopIndex;
    MinSize = FromPointerToEndSize < Size ? FromPointerToEndSize : Size;
    memcpy(BuffPointer, localBuffer->Buffer + localBuffer->PopIndex, MinSize);
    localBuffer->SizeBuffer -= MinSize;
    localBuffer->PopIndex   +=MinSize;
    localBuffer->PopIndex   &=TLOCALBUFFER_MASK_SIZE;
    /* Copy second Part if it's necessary */
    if(MinSize<Size) {
      memcpy(BuffPointer+MinSize, localBuffer->Buffer + localBuffer->PopIndex, Size-MinSize);
      localBuffer->SizeBuffer -= Size-MinSize;
      localBuffer->PopIndex   += Size-MinSize;
    }
#endif /* TLOCAL_BUFFER_USE_FOR_CYCLES */
  }

  /* .... to here */
  __enable_irq();
  return 1;
}

/**
 * @brief Push bytes to Buffer 
 * @param TLocalBuffer* localBuffer pointer to the local buffer structure
 * @param void* Buffer source buffer
 * @param int Size Number of byte to push
 * @retval int8_t Ok/Error (1/0)
 */
int8_t LocalBufferPushBuffer(TLocalBuffer* localBuffer, void* Data,int Size)
{
  /* Check if there is enough Space */
  if((TLOCALBUFFER_SIZE-localBuffer->SizeBuffer) < Size){
    return 0;
  }
  
  /* Push the Data */
  if(Size) {
    uint8_t *BuffPointer = (uint8_t *) Data;
#ifdef TLOCAL_BUFFER_USE_FOR_CYCLES
    int32_t Count;
    for(Count=0; Count<Size; Count++) {
      localBuffer->Buffer[localBuffer->PushIndex] = BuffPointer[Count];
      localBuffer->PushIndex++;
      localBuffer->PushIndex&=TLOCALBUFFER_MASK_SIZE;
    }
    localBuffer->SizeBuffer += Size;
#else /* TLOCAL_BUFFER_USE_FOR_CYCLES */
    /* Copy first Part */
    int32_t FromPointerToEndSize;
    int32_t MinSize;
    FromPointerToEndSize = TLOCALBUFFER_SIZE - localBuffer->PushIndex;
    MinSize = FromPointerToEndSize < Size ? FromPointerToEndSize : Size;
    memcpy(localBuffer->Buffer + localBuffer->PushIndex, BuffPointer, MinSize);
    localBuffer->SizeBuffer  += MinSize;
    localBuffer->PushIndex   += MinSize;
    localBuffer->PushIndex   &= TLOCALBUFFER_MASK_SIZE;
    /* Copy second Part if it's necessary */
    if(MinSize<Size) {
      memcpy(localBuffer->Buffer + localBuffer->PushIndex, BuffPointer+MinSize, Size-MinSize);
      localBuffer->SizeBuffer += Size-MinSize;
      localBuffer->PushIndex  += Size-MinSize;
    }
#endif /* TLOCAL_BUFFER_USE_FOR_CYCLES */
  }
  return 1;
}
/******************* (C) COPYRIGHT 2017 STMicroelectronics *****END OF FILE****/
