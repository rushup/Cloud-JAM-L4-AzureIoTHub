/**
  ******************************************************************************
  * @file    stm32l4xx_hal_msp.c
  * @author  Central LAB
  * @version V2.0.0
  * @date    24-November-2016
  * @brief   HAL MSP module.
  *         
  @verbatim
 ===============================================================================
                     ##### How to use this driver #####
 ===============================================================================
    [..]
    This file is generated automatically by STM32CubeMX and eventually modified 
    by the user

  @endverbatim
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

/* Includes ------------------------------------------------------------------*/
#include "TargetFeatures.h"
#include "wifi_globals.h"

/**
  * @brief UART MSP Initialization
  *        This function configures the hardware resources used in this example:
  *           - Peripheral's clock enable
  *           - Peripheral's GPIO Configuration  
  * @param huart: UART handle pointer
  * @retval None
  */
void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
  if(huart==&UartHandle) {
    /* UART2 for Console Messagges */
    GPIO_InitTypeDef  GPIO_InitStruct;

    /* Enable GPIO TX/RX clock */
    USART2_CMN_DEFAULT_TX_GPIO_CLK_ENABLE();
    USART2_CMN_DEFAULT_RX_GPIO_CLK_ENABLE();

    /* Enable USARTx clock */
    USART2_CMN_DEFAULT_CLK_ENABLE();

    /* UART TX GPIO pin configuration  */
    GPIO_InitStruct.Pin       = USART2_CMN_DEFAULT_TX_PIN;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_PULLUP;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FAST;
    GPIO_InitStruct.Alternate = USART2_CMN_DEFAULT_TX_AF;

    HAL_GPIO_Init(USART2_CMN_DEFAULT_TX_GPIO_PORT, &GPIO_InitStruct);

    /* UART RX GPIO pin configuration  */
    GPIO_InitStruct.Pin = USART2_CMN_DEFAULT_RX_PIN;
    GPIO_InitStruct.Alternate = USART2_CMN_DEFAULT_RX_AF;

    HAL_GPIO_Init(USART2_CMN_DEFAULT_RX_GPIO_PORT, &GPIO_InitStruct);
  } else if (huart==&UartWiFiHandle) {
    /* UART1 for WIFI */
    GPIO_InitTypeDef  GPIO_InitStruct;

    /* Enable GPIO TX/RX clock */
    USARTx_TX_GPIO_CLK_ENABLE();
    USARTx_RX_GPIO_CLK_ENABLE();

    /* Enable USARTx clock */
    USARTx_CLK_ENABLE();
    __SYSCFG_CLK_ENABLE();

    /* UART TX GPIO pin configuration  */
    GPIO_InitStruct.Pin       = WiFi_USART_TX_PIN;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Alternate = WiFi_USARTx_TX_AF;
    GPIO_InitStruct.Speed     = GPIO_SPEED_HIGH;

    HAL_GPIO_Init(WiFi_USART_TX_GPIO_PORT, &GPIO_InitStruct);

    /* UART RX GPIO pin configuration  */
    GPIO_InitStruct.Pin = WiFi_USART_RX_PIN;
    GPIO_InitStruct.Alternate = WiFi_USARTx_RX_AF;

    HAL_GPIO_Init(WiFi_USART_RX_GPIO_PORT, &GPIO_InitStruct);

    /* UART RTS GPIO pin configuration  */
    GPIO_InitStruct.Pin = WiFi_USART_RTS_PIN;
    GPIO_InitStruct.Pull     = GPIO_PULLUP;
    GPIO_InitStruct.Alternate = WiFi_USARTx_RX_AF;
    HAL_GPIO_Init(WiFi_USART_RTS_GPIO_PORT, &GPIO_InitStruct);
    HAL_NVIC_SetPriority(USARTx_IRQn, 3, 0);
    HAL_NVIC_EnableIRQ(USARTx_IRQn);
  }
}

/**
  * @brief UART MSP De-Initialization 
  *        This function frees the hardware resources used in this example:
  *          - Disable the Peripheral's clock
  *          - Revert GPIO and NVIC configuration to their default state
  * @param huart: UART handle pointer
  * @retval None
  */
void HAL_UART_MspDeInit(UART_HandleTypeDef *huart)
{
  if(huart==&UartHandle) {
    /* UART2 for Console Messagges */
    USART2_CMN_DEFAULT_FORCE_RESET();
    USART2_CMN_DEFAULT_RELEASE_RESET();

    /* Configure UART Tx as alternate function  */
    HAL_GPIO_DeInit(USART2_CMN_DEFAULT_TX_GPIO_PORT, USART2_CMN_DEFAULT_TX_PIN);
    /* Configure UART Rx as alternate function  */
    HAL_GPIO_DeInit(USART2_CMN_DEFAULT_RX_GPIO_PORT, USART2_CMN_DEFAULT_RX_PIN);
  } else if (huart==&UartWiFiHandle) {
    /* UART1 for WIFI */
    USARTx_FORCE_RESET();
    USARTx_RELEASE_RESET();

    /* Configure UART Tx as alternate function  */
    HAL_GPIO_DeInit(WiFi_USART_TX_GPIO_PORT, WiFi_USART_TX_PIN);
    /* Configure UART Rx as alternate function  */
    HAL_GPIO_DeInit(WiFi_USART_RX_GPIO_PORT, WiFi_USART_RX_PIN);
    HAL_NVIC_DisableIRQ(USARTx_IRQn);
  }
}


/**
 * @brief  I2C MSP Initialization
 * @param hi2c: I2C handle pointer
 * @retval None
 */

void HAL_I2C_MspInit(I2C_HandleTypeDef *hi2c)
{
  GPIO_InitTypeDef  GPIO_InitStruct;
  
  /* Enable I2C GPIO clocks */
  I2C1_CMN_DEFAULT_SCL_SDA_GPIO_CLK_ENABLE();
  
  /* I2C_EXPBD SCL and SDA pins configuration -------------------------------------*/
  GPIO_InitStruct.Pin = I2C1_CMN_DEFAULT_SCL_PIN |I2C1_CMN_DEFAULT_SDA_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
  GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
  GPIO_InitStruct.Pull  = GPIO_NOPULL;
  GPIO_InitStruct.Alternate  =I2C1_CMN_DEFAULT_SCL_SDA_AF;
  HAL_GPIO_Init(I2C1_CMN_DEFAULT_SCL_SDA_GPIO_PORT, &GPIO_InitStruct);
  
  /* Enable the I2C_EXPBD peripheral clock */
 I2C1_CMN_DEFAULT_CLK_ENABLE();
  
  /* Force the I2C peripheral clock reset */
 I2C1_CMN_DEFAULT_FORCE_RESET();
  
  /* Release the I2C peripheral clock reset */
 I2C1_CMN_DEFAULT_RELEASE_RESET();
  
  /* Enable and set I2C_EXPBD Interrupt to the highest priority */
  HAL_NVIC_SetPriority(I2C1_CMN_DEFAULT_EV_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(I2C1_CMN_DEFAULT_EV_IRQn);
  
  /* Enable and set I2C_EXPBD Interrupt to the highest priority */
  HAL_NVIC_SetPriority(I2C1_CMN_DEFAULT_ER_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(I2C1_CMN_DEFAULT_ER_IRQn);
}

/**
  * @brief TIM OC MSP Initialization
  * This function configures the hardware resources used in this example:
  *  - Peripheral's clock enable
  *  - Peripheral's Interrupt Configuration
  * @param htim: TIM handle pointer
  * @retval None
  */
void HAL_TIM_OC_MspInit(TIM_HandleTypeDef *htim)
{ 
  /* TIM1 Peripheral clock enable */
  __HAL_RCC_TIM1_CLK_ENABLE();

  /*  Enable the TIM1 global Interrupt & set priority */
  HAL_NVIC_SetPriority(TIM1_CC_IRQn, 0x8, 0);
  HAL_NVIC_EnableIRQ(TIM1_CC_IRQn);
}

/**
  * @brief CRC MSP Initialization
  *        This function configures the hardware resources used in this example:
  *           - Peripheral's clock enable
  * @param hcrc: CRC handle pointer
  * @retval None
  */
void HAL_CRC_MspInit(CRC_HandleTypeDef *hcrc)
{
  /* CRC Peripheral clock enable */
  __HAL_RCC_CRC_CLK_ENABLE();
}

/**
  * @brief CRC MSP De-Initialization
  *        This function freeze the hardware resources used in this example:
  *          - Disable the Peripheral's clock
  * @param hcrc: CRC handle pointer
  * @retval None
  */
void HAL_CRC_MspDeInit(CRC_HandleTypeDef *hcrc)
{
  /* Enable CRC reset state */
  __HAL_RCC_CRC_FORCE_RESET();

  /* Release CRC from reset state */
  __HAL_RCC_CRC_RELEASE_RESET();
}

/**
  * @brief RTC MSP Initialization 
  *        This function configures the hardware resources used in this example
  * @param hrtc: RTC handle pointer
  * 
  * @note  Care must be taken when HAL_RCCEx_PeriphCLKConfig() is used to select 
  *        the RTC clock source; in this case the Backup domain will be reset in  
  *        order to modify the RTC Clock source, as consequence RTC registers (including 
  *        the backup registers) and RCC_BDCR register are set to their reset values.
  *             
  * @retval None
  */
void HAL_RTC_MspInit(RTC_HandleTypeDef *hrtc)
{
  RCC_OscInitTypeDef        RCC_OscInitStruct;
  RCC_PeriphCLKInitTypeDef  PeriphClkInitStruct;
  
  /* To change the source clock of the RTC feature (LSE, LSI), You have to:
     - Enable the power clock using __HAL_RCC_PWR_CLK_ENABLE()
     - Enable write access using HAL_PWR_EnableBkUpAccess() function before to 
       configure the RTC clock source (to be done once after reset).
     - Reset the Back up Domain using __HAL_RCC_BACKUPRESET_FORCE() and 
       __HAL_RCC_BACKUPRESET_RELEASE().
     - Configure the needed RTc clock source */
  
  /* Configue LSE as RTC clock source */ 
  RCC_OscInitStruct.OscillatorType =  RCC_OSCILLATORTYPE_LSI | RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_OFF;
  if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK){ 
    while(1);
  }
  
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  if(HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK){
    while(1);
  }

  /* Enable RTC Clock */ 
  __HAL_RCC_RTC_ENABLE(); 
}

/**
  * @brief RTC MSP De-Initialization 
  *        This function frees the hardware resources used in this example:
  *          - Disable the Peripheral's clock
  * @param hrtc: RTC handle pointer
  * @retval None
  */
void HAL_RTC_MspDeInit(RTC_HandleTypeDef *hrtc)
{
  /*##-1- Reset peripherals ##################################################*/
   __HAL_RCC_RTC_DISABLE();
}

/**
  * @brief TIM MSP Initialization
  *        This function configures the hardware resources used in this example:
  *           - Peripheral's clock enable
  * @param htim: TIM handle pointer
  * @retval None
  */
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim)
{
  if(htim==&PushTimHandle) {
    /* TIMx Peripheral clock enable */
    __HAL_RCC_TIM2_CLK_ENABLE();

    /* Set the TIMx priority */
    HAL_NVIC_SetPriority(TIM2_IRQn, 3, 0);

    /* Enable the TIMx global Interrupt */
    HAL_NVIC_EnableIRQ(TIM2_IRQn);
  } else {
    /* TIMx Peripheral clock enable */
    __HAL_RCC_TIM3_CLK_ENABLE();

    /* Set the TIMx priority */
    HAL_NVIC_SetPriority(TIM3_IRQn, 3, 0);

    /* Enable the TIMx global Interrupt */
    HAL_NVIC_EnableIRQ(TIM3_IRQn);
  }
}


/************************ (C) COPYRIGHT 2016 STMicroelectronics *****END OF FILE****/
