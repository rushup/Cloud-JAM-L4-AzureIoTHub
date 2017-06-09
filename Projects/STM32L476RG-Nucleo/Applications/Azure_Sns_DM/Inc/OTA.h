/**
  ******************************************************************************
  * @file    OTA.h 
  * @author  Central LAB
  * @version V3.0.0
  * @date    21-April-2017
  * @brief   Over-the-Air Update API
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
  
/* Define to prevent recursive inclusion -------------------------------------*/  
#ifndef _OTA_H_
#define _OTA_H_

#ifdef __cplusplus
 extern "C" {
#endif

/* Exported defines ----------------------------------------------------------*/

#ifdef USE_STM32F4XX_NUCLEO
  /* 240Kbytes Max Program Size */
  #define OTA_MAX_PROG_SIZE (0x40000-0x4000-8)
#endif /* USE_STM32F4XX_NUCLEO */
   
#ifdef USE_STM32L4XX_NUCLEO
  /* 496Kbytes Max Program Size */
  #define OTA_MAX_PROG_SIZE (0x80000-0x4000-8)
#endif /* USE_STM32L4XX_NUCLEO */

/* Exported functions --------------------------------------------------------*/
/* API for preparing the Flash for receiving the Update. It defines also the Size of the Update and the CRC value aspected */
extern void StartUpdateFWBlueMS(uint32_t SizeOfUpdate,uint32_t uwCRCValue);
/* API for storing chuck of data to Flash.
 * When it has recived the total number of byte defined by StartUpdateFWBlueMS,
 * it computes the CRC value and if it matches the aspected CRC value,
 * it writes the Magic Number in Flash for BootLoader */
extern int8_t UpdateFWBlueMS(uint32_t *SizeOfUpdateBlueFW,uint8_t * att_data, int32_t data_length,uint8_t WriteMagicNum);

/* API for checking the BootLoader compliance */
extern int8_t CheckBootLoaderCompliance(void);

/* API for overriding the Size of firmware image */
extern void ReWriteSizeOfUpdate(uint32_t SizeOfUpdate);

/* API for reading the Remaining Size of firmware image */
extern uint32_t ReadRemSizeOfUpdate(void);

/* API for restarting the FOTA procedure */
extern void CleanBeforeRestart(void);
#ifdef __cplusplus
}
#endif

#endif /* _OTA_H_ */

/******************* (C) COPYRIGHT 2017 STMicroelectronics *****END OF FILE****/
