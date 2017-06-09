/**
  ******************************************************************************
  * @file    lib_NDEF_WifiToken.h
  * @author  Central LAB
  * @version V1.0.0
  * @date    7-October-2015
  * @brief   This file help to manage NDEF file that represent Wifi Token
 ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2014 STMicroelectronics</center></h2>
  *
  * Licensed under MMY-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __LIB_NDEF_WIFIT_H
#define __LIB_NDEF_WIFIT_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "lib_NDEF.h"
	 
/** @addtogroup NFC_libraries
  * @{
  */


/** @addtogroup lib_NDEF
  * @{
  */

/* Exported types ------------------------------------------------------------*/

/**
  * @brief  WifiToken structure, to store Network SSID,Authentication Type,
   Encryption Type and Network Key
  */	 	 
typedef struct 
{
	char NetworkSSID[32];
	char AuthenticationType[6];
	char EncryptionType[6];
        char NetworkKey[32];
}sWifiTokenInfo;

	 	 
uint16_t NDEF_ReadWifiToken(struct sRecordInfo *pRecordStruct, sWifiTokenInfo *pWifiTokenStruct);
uint16_t NDEF_WriteWifiToken( sWifiTokenInfo *pWifiTokenStruct );

/**
  * @}
  */

/**
  * @}
  */

#ifdef __cplusplus
}
#endif
	 
#endif /* __LIB_NDEF_WIFIT_H */

/******************* (C) COPYRIGHT 2013 STMicroelectronics *****END OF FILE****/
