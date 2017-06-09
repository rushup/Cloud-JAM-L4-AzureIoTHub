/**
  ******************************************************************************
  * @file    lib_NDEF_Wifi.c
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

/* Includes ------------------------------------------------------------------*/
#include "lib_NDEF_Wifi.h"
#define ATTRIBUTE_ID_SSID_LSB                                               0X10
#define ATTRIBUTE_ID_SSID_MSB                                               0X45


#define ATTRIBUTE_ID_NETWORK_LSB                                            0X10
#define ATTRIBUTE_ID_NETWORK_MSB                                            0X27


/** @addtogroup NFC_libraries
 * 	@{
 */


/** @addtogroup lib_NDEF
  * @{
  */

/**
 * @brief  This buffer contains the data send/received by TAG
 */
extern uint8_t NDEF_Buffer [NDEF_MAX_SIZE];

/** @defgroup libWifiToken_Private_Functions
  * @{
  */

static void NDEF_FillWifiTokenStruct( uint8_t* pPayload, uint32_t PayloadSize, sWifiTokenInfo *pWifiTokenStruct);
static void NDEF_Read_WifiToken ( struct sRecordInfo *pRecordStruct, sWifiTokenInfo *pWifiTokenStruct );

/**
  * @brief  This function fill WifiToken structure with information of NDEF message
	* @param	pPayload : pointer on the payload data of the NDEF message
	* @param	PayloadSize : number of data in the payload
	* @param	pWifiTokenStruct : pointer on the structure to fill
  * @retval NONE 
  */
static void NDEF_FillWifiTokenStruct( uint8_t* pPayload, uint32_t PayloadSize, sWifiTokenInfo *pWifiTokenStruct)
{
	uint8_t* pLastByteAdd,data1,data2,*temp,*temp_br ;
        uint16_t SSIDLen,NetWorkKeyLen;
        uint8_t *dbg,dbg1;
      
	
	pLastByteAdd = (uint8_t*)(pPayload + PayloadSize);
        pPayload--;     
        
        while(pPayload++ != pLastByteAdd)
        {
          uint8_t attribute = *pPayload;
          temp_br = pPayload;
          switch(attribute)
          {
            
          case ATTRIBUTE_ID_SSID_LSB:
            temp = pPayload;
            dbg = temp;
            dbg1 = *++dbg;
            if(dbg1 == ATTRIBUTE_ID_SSID_MSB )
            {
              data1 = *++dbg;
              data2 = *++dbg;
              SSIDLen = data1;
              SSIDLen = SSIDLen << 8;
              SSIDLen |= data2;
              pPayload += 4;
              memcpy( pWifiTokenStruct->NetworkSSID, pPayload, SSIDLen);
              /* add end of string charactere */
              pWifiTokenStruct->NetworkSSID[SSIDLen] = '\0';	
              pPayload += SSIDLen - 1;
            }
            else if(dbg1 == ATTRIBUTE_ID_NETWORK_MSB )
            {
              data1 = *++dbg;
              data2 = *++dbg;
              NetWorkKeyLen = data1;
              NetWorkKeyLen = NetWorkKeyLen << 8;
              NetWorkKeyLen |= data2;
              pPayload += 4;
              memcpy( pWifiTokenStruct->NetworkKey, pPayload, NetWorkKeyLen);
              /* add end of string charactere */
              pWifiTokenStruct->NetworkKey[NetWorkKeyLen] = '\0';
              pPayload += NetWorkKeyLen -1;
            }
            else
            {
              pPayload = temp_br;
            }
            
          break; 
           
          default :
            ;
          }
        }
        
}

/**
  * @brief  This fonction read the WifiToken and store data in a structure
	* @param	pRecordStruct : Pointer on the record structure
	* @param	pWifiTokenStruct : pointer on the structure to fill
  * @retval NONE 
  */
static void NDEF_Read_WifiToken ( struct sRecordInfo *pRecordStruct, sWifiTokenInfo *pWifiTokenStruct )
{
	uint8_t* pPayload;
	uint32_t PayloadSize;
	
	PayloadSize = ((uint32_t)(pRecordStruct->PayloadLength3)<<24) | ((uint32_t)(pRecordStruct->PayloadLength2)<<16) |
    ((uint32_t)(pRecordStruct->PayloadLength1)<<8)  | pRecordStruct->PayloadLength0;
	
	/* Read record header */
	pPayload = (uint8_t*)(pRecordStruct->PayloadBufferAdd);
	
	if( pRecordStruct->NDEF_Type == URI_WIFITOKEN_TYPE)
		NDEF_FillWifiTokenStruct(pPayload , PayloadSize, pWifiTokenStruct);
  
}

/**
  * @}
  */

/** @defgroup libWifiToken_Public_Functions
  * @{
  *	@brief  This file is used to manage WifiToken (stored or loaded in tag)
  */ 

/**
  * @brief  This fonction read NDEF and retrieve WifiToken information if any
	* @param	pRecordStruct : Pointer on the record structure
	* @param	pWifiTokenStruct : pointer on the structure to fill 
  * @retval SUCCESS : WifiToken information from NDEF have been retrieve
	* @retval ERROR : Not able to retrieve WifiToken information
  */
uint16_t NDEF_ReadWifiToken(struct sRecordInfo *pRecordStruct, sWifiTokenInfo *pWifiTokenStruct)
{
	uint16_t status = ERROR;
	uint16_t FileId=0;
	  
	if( pRecordStruct->NDEF_Type == URI_WIFITOKEN_TYPE )
	{	
		NDEF_Read_WifiToken(pRecordStruct, pWifiTokenStruct );
		status = SUCCESS;
	}
	else{
          
          
	}
	
	CloseNDEFSession(FileId);
	
	return status;
}


/**
  * @brief  This fonction write the NDEF file with the WifiToken data given in the structure
	* @param	pWifiTokenStruct : pointer on structure that contain the WifiToken information
  * @retval SUCCESS : the function is succesful
	* @retval ERROR : Not able to store NDEF file inside tag.
  */
uint16_t NDEF_WriteWifiToken ( sWifiTokenInfo *pWifiTokenStruct )
{
	uint16_t status = ERROR;
        uint8_t* pPayload,initStage = 0;
        uint16_t DataSize;
        uint32_t PayloadSize,SSIDSize,SSIDKeySize;
        uint8_t configToken1[CONFIG_TOKEN_1] = {0x10,0x4A, /* Attribute ID : Version*/
                                    0x00,0x01, /* Attribute ID Length*/
                                    0x10,    /* Version 1.0*/
                                    0x10,0x0E,  /* Attribute ID Credential*/  
                                    0x00,0x43, /* Attribute ID Length*/
                                    0x10,0x26, /* Attribute ID : Network Index*/
                                    0x00,0x01, /* Attribute Length*/
                                    0x01,  /* Length*/
                                    0x10,0x45, /* Attribute ID :SSID*/
                            
        };
        
      /*Fille SSID length + SSID between configToken1 and configToken3*/   
        uint8_t configToken3[CONFIG_TOKEN_3] = {0x10,0x03, /* Attribute ID :Authentication Type*/
                                    0x00,0x02, /* Attribute Length*/
                                    0x00,0x01,  /* Attribute Type : WPA2-Personal*/
                                    0x10,0x0F,  /* Attribute ID  : Encryption Type*/
                                    0x00,0x02,  /* Attribute Length*/
                                    0x00,0x02, /* Encryption Type : AES*/
                                    0x10,0x27};  /* Attribute ID  : Network Key */
        
              
     /*Fill SSID KEY Length and SSID Key between configToken3 and configToken5*/
        
        uint8_t configToken5[CONFIG_TOKEN_5] = {0x10,0x20, /* Attribute ID  : MAC Address */
                                    0x00,0x06, /* Attribute Length*/
                                    0, /*MAC-ADDRESS*/
                                    0, /*MAC-ADDRESS*/
                                    0, /*MAC-ADDRESS*/
                                    0, /*MAC-ADDRESS*/
                                    0, /*MAC-ADDRESS*/
                                    0, /*MAC-ADDRESS*/
                                    0x10,0x49, /* Attribute ID  : Vendor Extension */
                                    0x00,0x06, /* Attribute Length*/
                                    0x00,0x37,0x2A, /* Vendor ID:WFA*/
                                    0x02, /* Subelement ID:Network Key Shareable*/
                                    0x01, /* Subelement Length*/
                                    0x01, /*Network Key Shareable : TRUE*/
                                    0x10,0x49, /* Attribute ID  : Vendor Extension */
                                    0x00,0x06,/* Attribute Length*/
                                    0x00,0x37,0x2A,/* Vendor ID:WFA*/
                                    0x00, /* Subelement ID:Version2*/
                                    0x01, /* Subelement Length:1*/
                                    0x20 /* Version2*/
        };
               

	NDEF_Buffer[0] = 0x00;
	NDEF_Buffer[1] = 0x00;
	
	/* fill Wifi record header */
	NDEF_Buffer[FIRST_RECORD_OFFSET] = 0xD2;   /* Record Flag */
	NDEF_Buffer[FIRST_RECORD_OFFSET+1] = WIFITOKEN_TYPE_STRING_LENGTH;
	NDEF_Buffer[FIRST_RECORD_OFFSET+2] = 76; /* needs to be autocalculated - done at the end */
  
	memcpy(&NDEF_Buffer[FIRST_RECORD_OFFSET+3], WIFITOKEN_TYPE_STRING, WIFITOKEN_TYPE_STRING_LENGTH);
	
	pPayload = &NDEF_Buffer[FIRST_RECORD_OFFSET+3+WIFITOKEN_TYPE_STRING_LENGTH];
	PayloadSize = 0;
	
	
        for(initStage=0;initStage<CONFIG_TOKEN_1;initStage++)
        {
          *pPayload =configToken1[initStage];
          pPayload++;
        }
        
        /*Fill SSID length and SSID value*/
        SSIDSize = strlen(pWifiTokenStruct->NetworkSSID);
        *pPayload = 0x00; pPayload++;
        *pPayload = SSIDSize & 0x000000FF; pPayload++;
        
        strcpy((char*)pPayload,pWifiTokenStruct->NetworkSSID);
        pPayload = pPayload + strlen(pWifiTokenStruct->NetworkSSID);
 
        for(initStage=0;initStage<CONFIG_TOKEN_3;initStage++)
        {
          *pPayload =configToken3[initStage];
          pPayload++;
        }
        
      /*Fill SSIDKey length and SSIDKey value*/
        SSIDKeySize = strlen(pWifiTokenStruct->NetworkKey);
        *pPayload = 0x00; pPayload++;
        *pPayload = SSIDKeySize & 0x000000FF; pPayload++;
        
        strcpy((char*)pPayload,pWifiTokenStruct->NetworkKey);
        pPayload = pPayload + strlen(pWifiTokenStruct->NetworkKey);
                
        for(initStage=0;initStage<CONFIG_TOKEN_5;initStage++)
        {
          *pPayload =configToken5[initStage];
          pPayload++;
        }

	PayloadSize += CONFIG_TOKEN_1 + CONFIG_TOKEN_3 + CONFIG_TOKEN_5 + SSIDSize + SSIDKeySize;
		
	NDEF_Buffer[FIRST_RECORD_OFFSET+2] = (PayloadSize & 0x000000FF);
	
	DataSize = PayloadSize + 5 + WIFITOKEN_TYPE_STRING_LENGTH;
	
	/* Write NDEF */
	status = WriteData ( 0x00 , DataSize , NDEF_Buffer);
	
	/* Write NDEF size to complete*/
	if( status == NDEF_ACTION_COMPLETED)
	{
		DataSize -= 2; /* Must not count the 2 byte that represent the NDEF size */
		NDEF_Buffer[0] = (DataSize & 0xFF00)>>8;
		NDEF_Buffer[1] = (DataSize & 0x00FF);
    
		status = WriteData ( 0x00 , 2 , NDEF_Buffer);
	}
	
	if( status == NDEF_ACTION_COMPLETED)
		return SUCCESS;
	else
		return ERROR;
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


