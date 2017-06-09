/**
  ******************************************************************************
  * @file    lib_STProprietary_feature.h
  * @author  MMY Application Team
  * @version V1.0.0
  * @date    20-November-2013
  * @brief   This file help to manage some special feature embedded by target
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
#ifndef __LIB_STPROPRIETARY_FEATURE_H
#define __LIB_STPROPRIETARY_FEATURE_H

/* Includes ------------------------------------------------------------------*/ 
#include "lib_NDEF.h"	 
	 
uint16_t STProprietary_EnableReadOnly(uint8_t* pCurrentWritePassword);
uint16_t STProprietary_DisableReadOnly(uint8_t* pCurrentWritePassword);
uint16_t STProprietary_EnableWriteOnly(uint8_t* pCurrentWritePassword);
uint16_t STProprietary_DisableWriteOnly(uint8_t* pCurrentWritePassword);
uint16_t STProprietary_GPOConfig(uc8 GPO_RFconfig, uc8 mode);

#endif /* __LIB_STPROPRIETARY_FEATURE_H */

/******************* (C) COPYRIGHT 2013 STMicroelectronics *****END OF FILE****/
