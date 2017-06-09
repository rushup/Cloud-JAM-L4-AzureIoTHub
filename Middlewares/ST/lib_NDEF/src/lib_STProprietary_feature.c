/**
  ******************************************************************************
  * @file    lib_STProprietary_feature.c
  * @author  MMY Application Team
  * @version V1.0.0
  * @date    20-November-2013
  * @brief   This file help to manage some proprietary feature.
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

/* Includes ------------------------------------------------------------------*/
#include "lib_STProprietary_feature.h"

/** @addtogroup NFC_libraries
 * 	@{
 */


/** @addtogroup libSTProprietary
  * @{
	*	@brief  This part of the library help to manage TAG with proprietary feature.
  */


/** @defgroup libSpecialFeature_Private_Functions
  * @{
  */


/**
  * @}
  */


/** @defgroup libSpecialFeature_Public_Functions
  * @{
	*	@brief  This file is used to manage private feature of the tag
  */ 

/**
  * @brief  This fonction enable read only mode
	* @param	pCurrentWritePassword : Write password is needed to have right to enable read only mode
  * @retval SUCCESS : M24SR access is now forbidden in write mode
	* @retval ERROR : operation does not complete   
  */
uint16_t STProprietary_EnableReadOnly(uint8_t* pCurrentWritePassword)
{
	uint16_t status;
	uint16_t FileId;
	
	/* Before using Verify command NDEF file must be selected */
	GetNDEFFileId(&FileId);
	OpenNDEFSession(FileId, ASK_FOR_SESSION);	
	
	status = EnableReadOnly( pCurrentWritePassword);
	
	CloseNDEFSession(FileId);
  
	return status;
}

/**
  * @brief  This fonction disable read only mode
	* @param	pCurrentWritePassword : Write password is needed to have right to disable read only mode
  * @retval SUCCESS : M24SR write access is now allowed 
	* @retval ERROR : operation does not complete   
  */
uint16_t STProprietary_DisableReadOnly(uint8_t* pCurrentWritePassword )
{
	uint16_t status;
	uint16_t FileId;
	
	/* Before using Verify command NDEF file must be selected */
	GetNDEFFileId(&FileId);
	OpenNDEFSession(FileId, ASK_FOR_SESSION);	
	
	status = DisableReadOnly( pCurrentWritePassword );
	
	CloseNDEFSession(FileId);
  
	return status;
}

/**
  * @brief  This fonction enable write only mode
	* @param	pCurrentWritePassword : Write password is needed to have right to enable write only mode
  * @retval SUCCESS : M24SR access is now forbidden in read mode
	* @retval ERROR : operation does not complete   
  */
uint16_t STProprietary_EnableWriteOnly(uint8_t* pCurrentWritePassword)
{
	uint16_t status;
	uint16_t FileId;
	
	/* Before using Verify command NDEF file must be selected */
	GetNDEFFileId(&FileId);
	OpenNDEFSession(FileId, ASK_FOR_SESSION);	
	
	status = EnableWriteOnly( pCurrentWritePassword);
	
	CloseNDEFSession(FileId);
  
	return status;
}

/**
  * @brief  This fonction disable write only mode
	* @param	pCurrentWritePassword : Write password is needed to have right to disable write only mode
  * @retval SUCCESS : M24SR read access is now allowed 
	* @retval ERROR : operation does not complete   
  */
uint16_t STProprietary_DisableWriteOnly(uint8_t* pCurrentWritePassword)
{
	uint16_t status;
	uint16_t FileId;
	
	/* Before using Verify command NDEF file must be selected */
	GetNDEFFileId(&FileId);
	OpenNDEFSession(FileId, ASK_FOR_SESSION);	
	
	status = DisableWriteOnly( pCurrentWritePassword);
	
	CloseNDEFSession(FileId);
  
	return status;
}	

/**
  * @brief  This function configure GPO purpose for RF or I2C session
	* @param	GPO_config: GPO configuration to set
	* @param	mode: select RF or I2C, GPO config to update
  * @retval Status : Status of the operation.
  */
uint16_t STProprietary_GPOConfig(uc8 GPO_config, uc8 mode)
{
	uint16_t status;
	uint16_t FileId;
	
	/* Before using Verify command NDEF file must be selected */
	GetNDEFFileId(&FileId);
	OpenNDEFSession(FileId, ASK_FOR_SESSION);	
	status = GPO_Config(GPO_config, mode);
	CloseNDEFSession(FileId);
  
	return status;
}


/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/******************* (C) COPYRIGHT 2013 STMicroelectronics *****END OF FILE****/

