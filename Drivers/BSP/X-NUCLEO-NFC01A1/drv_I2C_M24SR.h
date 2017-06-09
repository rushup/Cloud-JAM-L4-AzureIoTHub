/**
  ******************************************************************************
  * @file    drv_I2C_M24SR.h
  * @author  MMY Application Team
  * @version V1.2.0
  * @date    20-October-2014
  * @brief   This file provides a set of functions needed to manage the I2C of
						 the M24SR device.
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
#ifndef __DRV_I2CM24SR_H
#define __DRV_I2CM24SR_H


#ifdef __MBED__ /* UNDEFINED types with mbed */
	#ifdef TARGET_NUCLEO_F401RE
		#include "stm32f4xx.h"
	#elif defined TARGET_NUCLEO_L053R8
		#include "stm32l0xx.h"
	#elif defined TARGET_NUCLEO_F030R8
		#include "stm32F0xx.h"
	#elif defined TARGET_NUCLEO_F302R8
		#include "stm32F3xx.h"
	#elif defined TARGET_NUCLEO_L152RE
		#include "stm32l1xx.h"
	#else
		#error "You need to update your code to this new microcontroller"
	#endif
#else
	#ifdef USE_STM32F4XX_NUCLEO
		#include "stm32f4xx_hal.h"
	#elif defined USE_STM32F0XX_NUCLEO
		#include "stm32f0xx_hal.h"
  #elif defined USE_STM32F1XX_NUCLEO
		#include "stm32f1xx_hal.h"
	#elif defined USE_STM32F3XX_NUCLEO
		#include "stm32f3xx_hal.h"
	#elif defined USE_STM32L0XX_NUCLEO
		#include "stm32l0xx_hal.h"
	#elif defined USE_STM32L1XX_NUCLEO
		#include "stm32l1xx_hal.h"
        #elif defined USE_STM32L4XX_NUCLEO
		#include "stm32l4xx_hal.h"
	#else
		#error "You need to update your code to this new microcontroller"
	#endif
#endif


#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "string.h"
#include "stdio.h"
#include "stdbool.h"
	 
/* Flags ---------------------------------------------------------------------*/
/* If both of this two flags are disabled, then the I²C polling will be used */
//#define I2C_GPO_SYNCHRO_ALLOWED /* allow tu use GPO polling as I2C synchronization */ 
//#define I2C_GPO_INTERRUPT_ALLOWED /* allow tu use GPO interrupt as I2C synchronization ! NOT SUPPORTED BY MBED ! */
	 
#define EXTERNAL_PULLUP /* If SCL is already pulled up using an external resistor */
	 
/* macro function ------------------------------------------------------------*/	 
 
#ifndef errchk
#define errchk(fCall) if (status = (fCall), status != M24SR_STATUS_SUCCESS) \
	{goto Error;} else
#endif

/*!< constant Unsigned integer types  */
typedef const unsigned char     uc8;
typedef const unsigned short    uc16;
//typedef const unsigned long     uc32;
	
	
/** @addtogroup M24SR_Driver
  * @{
  */

/** @addtogroup M24SR_I2C
  * @{
  */
	
/* Exported types ------------------------------------------------------------*/


/**
  * @brief  Synchronization Mechanism structure 
  */
typedef enum{
	M24SR_WAITINGTIME_UNKNOWN= 0,
	M24SR_WAITINGTIME_POLLING,
	M24SR_WAITINGTIME_TIMEOUT,
	M24SR_WAITINGTIME_GPO,
	M24SR_INTERRUPT_GPO
}M24SR_WAITINGTIME_MGMT; 	

/* Exported constants --------------------------------------------------------*/

/** @defgroup M24SR_I2C_Exported_Constants
  * @{
  */


/** @defgroup M24SR_I2C_Acces_Configuration 
  * @{
  */
#define M24SR_I2C_TIMEOUT   	200 /* I2C Time out (ms), this is the maximum time needed by M24SR to complete any command */
#define M24SR_I2C_POLLING  		1 /* In case M24SR will reply ACK failed allow to perform retry before returning error (HAL option not used) */
#define M24SR_ADDR           	0xAC   /*!< M24SR address */
/**
  * @}
  */

/** @defgroup M24SR_I2C_Error_Code_From_M24SR
  * @{
  */
/* error code ---------------------------------------------------------------------------------*/
#define M24SR_ERRORCODE_FILEOVERFLOW							0x6280
#define M24SR_ERRORCODE_ENDOFFILE									0x6282
#define M24SR_ERRORCODE_PASSWORDREQUIRED					0x63C0
#define M24SR_ERRORCODE_PASSWORDINCORRECT2RETRY		0x63C2
#define M24SR_ERRORCODE_PASSWORDINCORRECT1RETRY		0x63C1
#define M24SR_ERRORCODE_RFSESSIONKILLED						0x6500
#define M24SR_ERRORCODE_UNSUCCESSFULUPDATING			0x6581
#define M24SR_ERRORCODE_WRONGHLENGTH							0x6700
#define M24SR_ERRORCODE_COMMANDINCORRECT					0x6981
#define M24SR_ERRORCODE_SECURITYSTATUS						0x6982
#define M24SR_ERRORCODE_REFERENCEDATANOTUSABLE		0x6984
#define M24SR_ERRORCODE_INCORRECTPARAMETER				0x6A80
#define M24SR_ERRORCODE_FILENOTFOUND							0x6A82
#define M24SR_ERRORCODE_FILEOVERFLOWLC						0x6A84
#define M24SR_ERRORCODE_INCORRECTP1P2							0x6A86
#define M24SR_ERRORCODE_INSNOTSUPPORTED						0x6D00
#define M24SR_ERRORCODE_CLASSNOTSUPPORTED					0x6E00
#define M24SR_ERRORCODE_DAFAULT										0x6F00
/**
  * @}
  */

/** @defgroup M24SR_I2C_Error_Code_From_M24SR_SW
  * @{
  */
/* Status and error code -----------------------------------------------------*/	 
#define M24SR_STATUS_SUCCESS									0x0000
#define M24SR_ERROR_DEFAULT										0x0010
#define M24SR_ERROR_I2CTIMEOUT								0x0011
#define M24SR_ERROR_CRC												0x0012
#define M24SR_ERROR_NACK											0x0013
#define M24SR_ERROR_PARAMETER									0x0014 
#define M24SR_ERROR_NBATEMPT									0x0015 
#define M24SR_ERROR_NOACKNOWLEDGE							0x0016
/**
  * @}
  */

/**
  * @}
  */

/*  public function	--------------------------------------------------------------------------*/

//void          M24SR_I2CInit                					( void );
void          M24SR_GPOInit                					( void );
void          M24SR_WaitMs                          ( uint32_t time_ms );
void          M24SR_GetTick                         ( uint32_t *ptickstart );
void          M24SR_GPO_ReadPin                     ( GPIO_PinState *pPinState);
void          M24SR_RFDIS_WritePin                  ( GPIO_PinState PinState);
void	        M24SR_SetI2CSynchroMode    						( uint8_t mode );
int8_t        M24SR_SendI2Ccommand 									( uint8_t NbByte , uint8_t *pBuffer );
int8_t        M24SR_IsAnswerReady 				  				( void );
int8_t        M24SR_PollI2C 												( void );
int8_t        M24SR_ReceiveI2Cresponse 							( uint8_t NbByte , uint8_t *pBuffer );
void          M24SR_RFConfig_Hard										( uint8_t OnOffChoice);

#endif 

/**
  * @}
  */
	
/**
  * @}
  */


#ifdef __cplusplus
}
#endif

/******************* (C) COPYRIGHT 2013 STMicroelectronics *****END OF FILE****/
