/**
 ******************************************************************************
 * @file    stm32_spwf_wifi.c
 * @author  Central LAB
 * @version V2.1.0
 * @date    17-May-2016
 * @brief   HAL related functionality of X-CUBE-WIFI1
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT(c) 2015 STMicroelectronics</center></h2>
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
#include "wifi_module.h"
#include "stm32_spwf_wifi.h"
#include "wifi_globals.h"

/** @addtogroup BSP
* @{
*/ 


/** @defgroup  NUCLEO_WIFI_DRIVER
  * @brief Wi-Fi_driver modules
  * @{
  */


/** @defgroup NUCLEO_WIFI_DRIVER_Private_Defines
  * @{
  */
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Prescaler declaration */
uint32_t uwPrescalerValue = 0;  

/* TIM handle declaration */

/**
  * @}
  */

/** @addtogroup NUCLEO_WIFI_DRIVER_Private_Variables
  * @{
  */
/* Private variables ---------------------------------------------------------*/

/**
  * @}
  */

  
/** @defgroup NUCLEO_WIFI_DRIVER_Private_Functions
  * @{
  */

  /*##-1- Configure the TIM peripheral #######################################*/
  /* -----------------------------------------------------------------------
    In this example TIM3 input clock (TIM3CLK)  is set to APB1 clock (PCLK1) x2,
    since APB1 prescaler is set to 4 (0x100).
       TIM3CLK = PCLK1*2
       PCLK1   = HCLK/2
    => TIM3CLK = PCLK1*2 = (HCLK/2)*2 = HCLK = SystemCoreClock
    To get TIM3 counter clock at 10 KHz, the Prescaler is computed as following:
    Prescaler = (TIM3CLK / TIM3 counter clock) - 1
    Prescaler = (SystemCoreClock /10 KHz) - 1

    Note:
     SystemCoreClock variable holds HCLK frequency and is defined in system_stm32f1xx.c file.
     Each time the core clock (HCLK) changes, user had to update SystemCoreClock
     variable value. Otherwise, any configuration based on this variable will be incorrect.
     This variable is updated in three ways:
      1) by calling CMSIS function SystemCoreClockUpdate()
      2) by calling HAL API function HAL_RCC_GetSysClockFreq()
      3) each time HAL_RCC_ClockConfig() is called to configure the system clock frequency
  ----------------------------------------------------------------------- */
void Timer_Config()
{
  /* Compute the prescaler value to have TIMx counter clock equal to 10000 Hz */
  uwPrescalerValue = (uint32_t)(SystemCoreClock / 10000) - 1;
  
  /* Set TIMx instance */
  TimHandle.Instance = TIMx;

  /* Initialize TIMx peripheral as follows:
       + Period = 10000 - 1
       + Prescaler = (SystemCoreClock/10000) - 1
       + ClockDivision = 0
       + Counter direction = Up
  */
#if defined (USE_STM32L0XX_NUCLEO) || (USE_STM32F4XX_NUCLEO) || (USE_STM32L4XX_NUCLEO)
  TimHandle.Init.Period            = 100 - 1;
#endif
#if defined (USE_STM32F1xx_NUCLEO) 
  TimHandle.Init.Period            = 100 - 1;
#endif  
  TimHandle.Init.Prescaler         = uwPrescalerValue;
  TimHandle.Init.ClockDivision     = 0;
  TimHandle.Init.CounterMode       = TIM_COUNTERMODE_UP;
#ifdef USE_STM32F1xx_NUCLEO
  TimHandle.Init.RepetitionCounter = 0;
#endif 

  if (HAL_TIM_Base_Init(&TimHandle) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler(); 
  }

}

/**
  * @brief Push_Timer_Config
  *        This function configures the Push Timer
  * @param None
  * @retval None
  */
void Push_Timer_Config()
{
  /* Compute the prescaler value to have TIMx counter clock equal to 10000 Hz */
  uwPrescalerValue = (uint32_t)(SystemCoreClock / 10000) - 1;
  
  /* Set TIMx instance */
  PushTimHandle.Instance = TIMp;

  /* Initialize TIMx peripheral as follows:
       + Period = 10000 - 1
       + Prescaler = (SystemCoreClock/10000) - 1
       + ClockDivision = 0
       + Counter direction = Up
  */
  PushTimHandle.Init.Period            = 10 - 1;//10000
  PushTimHandle.Init.Prescaler         = uwPrescalerValue;
  PushTimHandle.Init.ClockDivision     = 0;
  PushTimHandle.Init.CounterMode       = TIM_COUNTERMODE_UP;
#ifdef USE_STM32F1xx_NUCLEO
  PushTimHandle.Init.RepetitionCounter = 0;
#endif 

  if (HAL_TIM_Base_Init(&PushTimHandle) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler(); 
  }

}

/**
* @brief  USART_Configuration
* WB_WIFI_UART configured as follow:
*      - BaudRate = 115200 baud  
*      - Word Length = 8 Bits
*      - One Stop Bit
*      - No parity
*      - Hardware flow control enabled (RTS and CTS signals)
*      - Receive and transmit enabled
*
* @param  None
* @retval None
*/
void UART_Configuration(uint32_t baud_rate)
{
  UartWiFiHandle.Instance             = WB_WIFI_UART;
  UartWiFiHandle.Init.BaudRate        = baud_rate;
  UartWiFiHandle.Init.WordLength      = UART_WORDLENGTH_8B;
  UartWiFiHandle.Init.StopBits        = UART_STOPBITS_1;
  UartWiFiHandle.Init.Parity          = UART_PARITY_NONE ;
  UartWiFiHandle.Init.HwFlowCtl       = UART_HWCONTROL_RTS;//UART_HWCONTROL_NONE;
  UartWiFiHandle.Init.Mode            = UART_MODE_TX_RX;
  UartWiFiHandle.Init.OverSampling    = UART_OVERSAMPLING_16;
  //UartWiFiHandle.Init.OneBitSampling  = UART_ONEBIT_SAMPLING_ENABLED;
  
  if(HAL_UART_DeInit(&UartWiFiHandle) != HAL_OK)
  {
    Error_Handler();
  }  
  if(HAL_UART_Init(&UartWiFiHandle) != HAL_OK)
  {
    Error_Handler();
  }  
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

