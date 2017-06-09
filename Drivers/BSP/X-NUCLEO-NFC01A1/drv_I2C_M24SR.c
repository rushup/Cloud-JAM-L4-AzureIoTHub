/**
  ******************************************************************************
  * @file    drv_I2C_M24SR.c
  * @author  MMY Application Team
  * @version V1.2.0
  * @date    20-october-2014
  * @brief   This file provides a set of functions needed to manage the I2C of
	*				   the M24SR device.
 ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2014 STMicroelectronics</center></h2>
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

/* Includes ------------------------------------------------------------------*/
#include "drv_I2C_M24SR.h"

#ifdef USE_STM32F4XX_NUCLEO
#include "stm32f4xx_I2C.h"
#endif /* USE_STM32F4XX_NUCLEO */

#ifdef USE_STM32L4XX_NUCLEO
#include "stm32l4xx_I2C.h"
#endif /* USE_STM32L4XX_NUCLEO */

/** @addtogroup M24SR_Driver
  * @{
  */

/** @defgroup M24SR_I2C
 * 	@{
 *  @brief  This file includes the I2C driver used by M24SR family to communicate with the MCU.  
 */


/** @defgroup M24SR_I2C_Private_Functions
 *  @{
 */
#define hi2c1 I2CHandle
//extern I2C_HandleTypeDef hi2c1;

uint8_t uSynchroMode = M24SR_WAITINGTIME_POLLING;
volatile uint8_t	GPO_Low = 0;

void M24SR_GetTick( uint32_t *ptickstart );
/**
  * @}
  */


/** @defgroup M24SR_I2C_Public_Functions
  * @{
  */
#if 0
/**
  * @brief  This function initializes the M24SR_I2C interface
	* @retval None  
  */
__weak void M24SR_I2CInit ( void )
{

}
#endif

/**
  * @brief  This function initializes the M24SR_I2C interface
	* @retval None  
  */
__weak void	M24SR_GPOInit(void)
{

}

/**
  * @brief  this functions configure I2C synchronization mode
	* @param  mode : interruption or polling
  * @retval None
  */
void M24SR_SetI2CSynchroMode( uc8 mode )
{
#if defined (I2C_GPO_SYNCHRO_ALLOWED) || defined (I2C_GPO_INTERRUPT_ALLOWED)
	uSynchroMode = mode;
#else
	if(mode == M24SR_WAITINGTIME_GPO || mode == M24SR_INTERRUPT_GPO)
		uSynchroMode = M24SR_WAITINGTIME_POLLING;
	else
		uSynchroMode = mode;
#endif /*  I2C_GPO_SYNCHRO_ALLOWED */
}

/**
  * @brief  This functions polls the I2C interface
  * @retval M24SR_STATUS_SUCCESS : the function is succesful
	* @retval M24SR_ERROR_I2CTIMEOUT : The I2C timeout occured. 
  */
int8_t M24SR_PollI2C ( void )
{
	HAL_StatusTypeDef status;
	uint32_t tickstart = 0;
	uint32_t currenttick = 0;
	
  /* Get tick */
	M24SR_GetTick(&tickstart);
	
	/* Wait until M24SR is ready or timeout occurs */
  do
  {
		status = HAL_I2C_IsDeviceReady(&hi2c1, M24SR_ADDR, M24SR_I2C_POLLING, 100);
		M24SR_GetTick(&currenttick);
	} while( ( (currenttick - tickstart) < M24SR_I2C_TIMEOUT) && (status != HAL_OK) );
	
	if (status == HAL_OK)
		return M24SR_STATUS_SUCCESS;
	else
		return M24SR_ERROR_I2CTIMEOUT;

}

/**
  * @brief  This functions sends the command buffer 
	* @param  NbByte : Number of byte to send
  * @param  pBuffer : pointer to the buffer to send to the M24SR
  * @retval M24SR_STATUS_SUCCESS : the function is succesful
	* @retval M24SR_ERROR_I2CTIMEOUT : The I2C timeout occured. 
  */
int8_t M24SR_SendI2Ccommand ( uint8_t NbByte , uint8_t *pBuffer )
{
	uint16_t status ;
	
	// Add a check to detect an issue
	errchk(M24SR_PollI2C ( ));
	
	if( HAL_I2C_Master_Transmit(&hi2c1, M24SR_ADDR, (uint8_t*)pBuffer,NbByte, 100) == HAL_OK)	
		return M24SR_STATUS_SUCCESS;
	else
		return M24SR_ERROR_I2CTIMEOUT;
	
Error :
  return M24SR_ERROR_I2CTIMEOUT;
}

/**
  * @brief  This functions reads a response of the M24SR device
	* @param  NbByte : Number of byte to read (shall be >= 5)
  * @param  pBuffer : Pointer on the buffer to retrieve M24SR response
  * @retval M24SR_STATUS_SUCCESS : The function is succesful
	* @retval M24SR_ERROR_I2CTIMEOUT : The I2C timeout occured. 
  */
int8_t M24SR_ReceiveI2Cresponse ( uint8_t NbByte , uint8_t *pBuffer )
{
	uint16_t status ;
	
	// Before calling this function M24SR must be ready, here is a check to detect an issue
	errchk(M24SR_PollI2C ( ));
	
	if( HAL_I2C_Master_Receive(&hi2c1, M24SR_ADDR, (uint8_t*)pBuffer,NbByte, 100) == HAL_OK)
		return M24SR_STATUS_SUCCESS;
	else
		return M24SR_ERROR_I2CTIMEOUT;
	
Error :
  return M24SR_ERROR_I2CTIMEOUT;	
}



/**
  * @brief  This functions returns M24SR_STATUS_SUCCESS when a response is ready
  * @retval M24SR_STATUS_SUCCESS : a response of the M24LR is ready
	* @retval M24SR_ERROR_DEFAULT : the response of the M24LR is not ready
  */
int8_t M24SR_IsAnswerReady ( void )
{
	uint16_t status ;
  uint32_t retry = 0xFFFFF;
	uint8_t stable = 0;
	GPIO_PinState PinState;

  switch (uSynchroMode)
  {
  case M24SR_WAITINGTIME_POLLING :
    errchk(M24SR_PollI2C ( ));
    return M24SR_STATUS_SUCCESS;
    
  case M24SR_WAITINGTIME_TIMEOUT :
    // M24SR FWI=5 => (256*16/fc)*2^5=9.6ms but M24SR ask for extended time to program up to 246Bytes.
    // can be improved by 
    M24SR_WaitMs(80);	
    return M24SR_STATUS_SUCCESS;
    
  case M24SR_WAITINGTIME_GPO :
    /* mbd does not support interrupt for the moment with nucleo board */
    do
    {
			M24SR_GPO_ReadPin(&PinState);
      if( PinState == GPIO_PIN_RESET)
      {
        stable ++;						
      }
      retry --;						
    }
    while(stable <5 && retry>0);
    if(!retry)
      goto Error;				
    return M24SR_STATUS_SUCCESS;
    
  case M24SR_INTERRUPT_GPO :
    /* Check if the GPIO is not already low before calling this function */
		M24SR_GPO_ReadPin(&PinState);
    if(PinState == GPIO_PIN_SET)
    {
      while (GPO_Low == 0);
    }
    GPO_Low = 0;
    return M24SR_STATUS_SUCCESS;
    
  default : 
    return M24SR_ERROR_DEFAULT;
  }
  
Error :
  return M24SR_ERROR_DEFAULT;
}

/**
  * @brief  This function enable or disable RF communication
	* @param	OnOffChoice: GPO configuration to set
  */
void M24SR_RFConfig_Hard( uc8 OnOffChoice)
{
	/* Disable RF */
	if ( OnOffChoice != 0 )
	{	
		M24SR_RFDIS_WritePin(GPIO_PIN_RESET);
	}
	else
	{	
		M24SR_RFDIS_WritePin(GPIO_PIN_SET);
	}
}

__weak void M24SR_GetTick( uint32_t *ptickstart )
{

}

/**
  * @brief  This function wait the time given in param (in milisecond)
	* @param	time_ms: time value in milisecond
  */
__weak void M24SR_WaitMs(uint32_t time_ms)
{
	
}

/**
  * @brief  This function read the state of the M24SR GPO
	* @param	none
  * @retval GPIO_PinState : state of the M24SR GPO
  */
__weak void M24SR_GPO_ReadPin( GPIO_PinState * pPinState)
{

}

/**
  * @brief  This function set the state of the M24SR RF disable pin
	* @param	PinState: put RF disable pin of M24SR in PinState (1 or 0)
  */
__weak void M24SR_RFDIS_WritePin( GPIO_PinState PinState)
{
	
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

