/**
  ******************************************************************************
  * @file    lib_NDEF_Text.c
  * @author  MMY Application Team
  * @version $Revision: 1331 $
  * @date    $Date: 2016-11-23 10:39:11 +0100 (Thu, 23 Nov 2016) $
  * @brief   This file help to manage Text NDEF file.
   ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2015 STMicroelectronics</center></h2>
  *
  * Licensed under ST MYLIBERTY SOFTWARE LICENSE AGREEMENT (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/myliberty  
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied,
  * AND SPECIFICALLY DISCLAIMING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
  * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "lib_NDEF_Text.h"

/** @addtogroup NFC_libraries
  * @{
  * @brief  <b>This is the library used to manage the content of the TAG (data)
  *          But also the specific feature of the tag, for instance
  *          password, gpo... </b>
  */


/** @addtogroup libNFC_FORUM
  * @{
  * @brief  This part of the library manage data which follow NFC forum organisation.
  */

/**
  * @brief  This buffer contains the data send/received by TAG
  */
extern uint8_t NDEF_Buffer [];

/**
  * @}
  */

/** @defgroup libEmail_Public_Functions
  * @{
  * @brief  This file is used to manage Email (stored or loaded in tag)
  */ 

/**
  * @brief  This function write the text in the TAG.
  * @param  text : text to write.
  * @retval SUCCESS : NDEF file data written in the tag.
  * @retval ERROR : not able to store NDEF in tag.
  * @retval NDEF_ERROR_MEMORY_INTERNAL : Cannot write to tag.
  * @retval NDEF_ERROR_NOT_FORMATED : CCFile data not supported or not present.
  * @retval NDEF_ERROR_MEMORY_TAG : Size not compatible with memory.
  * @retval NDEF_ERROR_LOCKED : Tag locked, cannot be write.
  */
uint16_t NDEF_WriteText( char *text )
{
  uint16_t status = ERROR;
  uint32_t textSize, Offset = 0;

  NDEF_Buffer[0] = 0;
  NDEF_Buffer[1] = 0;
  Offset = FIRST_RECORD_OFFSET;

  /* TEXT : 1+en+message */
  textSize = 3 + strlen(text);

  /* TEXT header */
  NDEF_Buffer[Offset] = 0xD1;
  if( textSize < 256 ) NDEF_Buffer[Offset] |= 0x10;                   // Set the SR bit
  Offset++;

  NDEF_Buffer[Offset++] = TEXT_TYPE_STRING_LENGTH;
  if( textSize > 255 )
  {
    NDEF_Buffer[Offset++] = (textSize & 0xFF000000) >> 24;
    NDEF_Buffer[Offset++] = (textSize & 0x00FF0000) >> 16;
    NDEF_Buffer[Offset++] = (textSize & 0x0000FF00) >> 8;
    NDEF_Buffer[Offset++] = textSize & 0x000000FF;
  }
  else
  {
    NDEF_Buffer[Offset++] = (uint8_t)textSize;
  }
  memcpy( &NDEF_Buffer[Offset], TEXT_TYPE_STRING, TEXT_TYPE_STRING_LENGTH );
  Offset += TEXT_TYPE_STRING_LENGTH;

  /* TEXT payload */
  NDEF_Buffer[Offset++] = ISO_ENGLISH_CODE_STRING_LENGTH;
  memcpy( &NDEF_Buffer[Offset], ISO_ENGLISH_CODE_STRING, ISO_ENGLISH_CODE_STRING_LENGTH );
  Offset += ISO_ENGLISH_CODE_STRING_LENGTH;

  memcpy( &NDEF_Buffer[Offset], text, strlen(text) );
  Offset += strlen(text);

  Offset -= 2; /* Must not count the 2 byte that represent the NDEF size */
  NDEF_Buffer[0] = (Offset & 0xFF00) >> 8;
  NDEF_Buffer[1] = Offset & 0x00FF;

  status = WriteData( 0x00, Offset + 2, NDEF_Buffer );

  return status;
}


/**
  * @brief  This function the Text information in a record if any.
  * @param  pRecordStruct Pointer on a NDEF record structure.
  * @param  pText A pointer on a text information structure, used to return the text & its metadata.
  * @retval SUCCESS                    Text information has been retrieved.
  * @retval ERROR                 The record is not a Text record.
  * @retval NDEF_ERROR_MEMORY_INTERNAL Language code or Text length is too big for the buffers.
  */
uint16_t NDEF_ReadText( struct sRecordInfo *pRecordStruct, NDEF_Text_info_t *pText )
{
  uint16_t status = ERROR;

  if( pRecordStruct->NDEF_Type == TEXT_TYPE )
  {
      uint32_t PayloadSize = ((uint32_t)(pRecordStruct->PayloadLength3) << 24) | ((uint32_t)(pRecordStruct->PayloadLength2) << 16) |
                ((uint32_t)(pRecordStruct->PayloadLength1) << 8)  | pRecordStruct->PayloadLength0;
    /* Get the text metadata (status byte (encoding & language code length) + language code) */
    NDEF_Text_metadata_t *text_record_info = (NDEF_Text_metadata_t *)pRecordStruct->PayloadBufferAdd;
        
    uint32_t text_length = PayloadSize                            /* record length */
                           - text_record_info->language_length    /* minus language code length */
                           - sizeof(uint8_t);                     /* minus the status byte length */
    
    if((text_record_info->language_length >= NDEF_TEXT_LANGUAGE_CODE_MAX_LENGTH) || 
        (text_length >= NDEF_TEXT_MAX_LENGTH))
    {
      /* One of the text info structure buffer is too small */
      return ERROR;
    }
    
    /* Retrieve the encoding */
    pText->encoding =  (NDEF_Text_encoding_t)text_record_info->encoding;

    /* Save the language code string (adding null char at the end) */
    memcpy(&pText->language_code,text_record_info->language,text_record_info->language_length);
    pText->language_code[text_record_info->language_length] = '\0';
    
    /* Copy the text string itself (adding null char at the end) */
    memcpy(&pText->text, text_record_info->language + text_record_info->language_length,text_length);
    pText->text[text_length] = '\0';
    
    status = SUCCESS;
  }
  else 
  {
    /* Not a text record, exit in error */
    status = ERROR;
  }

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

/******************* (C) COPYRIGHT 2015 STMicroelectronics *****END OF FILE****/
