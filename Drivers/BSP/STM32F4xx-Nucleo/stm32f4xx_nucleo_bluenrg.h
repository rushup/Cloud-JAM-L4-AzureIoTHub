/**
  ******************************************************************************
  * @file    stm32f4xx_nucleo_bluenrg.h
  * @author  CL
  * @version V1.0.0
  * @date    04-July-2014
  * @brief   This file contains definitions for SPI communication on
  *          STM32F4XX-Nucleo Kit from STMicroelectronics for BLE BlueNRG
  *          Expansion Board (reference X-NUCLEO-IDB04A1).
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
  
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32F4XX_NUCLEO_BLUENRG_H
#define __STM32F4XX_NUCLEO_BLUENRG_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "stm32f4xx_nucleo.h"
#include "stm32f4xx_periph_conf.h"

/** @addtogroup BSP
  * @{
  */

/** @addtogroup STM32F4XX_NUCLEO
  * @{
  */

/** @defgroup STM32F4XX_NUCLEO_BLUENRG
  * @{
  */ 

/** @defgroup STM32F4XX_NUCLEO_BLUENRG_Exported_Defines
  * @{
  */
  
/**
* @brief SPI communication details between Nucleo F4 and BlueNRG
*        Expansion Board.
*/
#define BNRG_SPI_RESET_PIN     SPI1_CMN_DEFAULT_RESET_PIN
#define BNRG_SPI_RESET_PORT    SPI1_CMN_DEFAULT_RESET_PORT
#define BNRG_SPI_EXTI_PIN      SPI1_CMN_DEFAULT_EXTI_PIN
#define BNRG_SPI_EXTI_PORT     SPI1_CMN_DEFAULT_EXTI_PORT
#define BNRG_SPI_CS_PIN        SPI1_CMN_DEFAULT_CS_PIN
#define BNRG_SPI_CS_PORT       SPI1_CMN_DEFAULT_CS_PORT
#define BNRG_SPI_IRQ_PIN       SPI1_CMN_DEFAULT_IRQ_PIN
#define BNRG_SPI_IRQ_SPEED     SPI1_CMN_DEFAULT_IRQ_SPEED
#define BNRG_SPI_IRQ_PORT      SPI1_CMN_DEFAULT_IRQ_PORT
#define BNRG_SPI_IRQ_MODE      SPI1_CMN_DEFAULT_IRQ_MODE
#define BNRG_SPI_IRQ_ALTERNATE SPI1_CMN_DEFAULT_IRQ_ALTERNATE
#define BNRG_SPI_IRQ_PULL      SPI1_CMN_DEFAULT_IRQ_PULL
#define BNRG_SPI_EXTI_IRQn     SPI1_CMN_DEFAULT_EXTI_IRQn


#define RTC_WAKEUP_IRQHandler       RTC_WKUP_IRQHandler

//EXTI External Interrupt for user button
//#define PUSH_BUTTON_EXTI_IRQHandler EXTI15_10_IRQHandler
/**
  * @}
  */

/** @defgroup STM32F4XX_NUCLEO_BLUENRG_Exported_Functions
  * @{
  */
  
void Enable_SPI_IRQ(void);
void Disable_SPI_IRQ(void);
void Clear_SPI_IRQ(void);
void Clear_SPI_EXTI_Flag(void);

/**
  * @}
  */ 

/**
  * @}
  */ 

/**
  * @}
  */

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* __STM32F4XX_NUCLEO_BLUENRG_H */

    
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

