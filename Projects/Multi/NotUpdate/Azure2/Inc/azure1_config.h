/**
  ******************************************************************************
  * @file    azure1_config.h
  * @author  Central LAB
  * @version V2.0.0
  * @date    24-November-2016
  * @brief   azure1 configuration
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2016 STMicroelectronics</center></h2>
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
#ifndef __azure1_CONFIG_H
#define __azure1_CONFIG_H

/* Exported define ------------------------------------------------------------*/

/*************** Enable Print Informations ******************/
#define AZURE_ENABLE_PRINTF

/*************** Default WIFI Credential ******************/
#define AZURE_DEFAULT_SSID "STM"
#define AZURE_DEFAULT_SECKEY "STMdemoPWD1"
#define AZURE_DEFAULT_PRIV_MODE WPA_Personal


/* For enabling the FOTA with one http get request pressing the UserButton */
#define AZURE_ENABLE_OTA


#ifdef AZURE_ENABLE_OTA
  /* For implementing the http get on one Socket */
  #define AZURE_OTA_HTTP_SOCKET
#endif /* AZURE_ENABLE_OTA */






/*************** Don't Change the following defines *************/

/* Package Version only numbers 0->9 */
#define AZURE_VERSION_MAJOR '1'
#define AZURE_VERSION_MINOR '0'
#define AZURE_VERSION_PATCH '0'

/* Package Name */
#define AZURE_PACKAGENAME "Azure2"

#ifdef AZURE_ENABLE_PRINTF
  #define AZURE_PRINTF(...) printf(__VA_ARGS__)
#else /* AZURE_ENABLE_PRINTF */
  #define AZURE_PRINTF(...)
#endif /* AZURE_ENABLE_PRINTF */

#endif /* __azure1_CONFIG_H */

/******************* (C) COPYRIGHT 2016 STMicroelectronics *****END OF FILE****/
