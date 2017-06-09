/**
  ******************************************************************************
  * @file    main.c
  * @author  Central LAB
  * @version V2.2.0
  * @date    29-September-2015
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2015 STMicroelectronics</center></h2>
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
#include <stdio.h>
#include "main.h"
/* Private typedef -----------------------------------------------------------*/

typedef struct
{
  uint32_t Version;
  uint32_t MagicNum;
  uint32_t OTAStartAdd;
  uint32_t OTAMaxSize;
  uint32_t ProgStartAdd;
} BootLoaderFeatures_t;


/* Private define ------------------------------------------------------------*/
#define BL_VERSION_MAJOR 1
#define BL_VERSION_MINOR 2
#define BL_VERSION_PATCH 0

#define BMS_OTA_CHECK_MAGIC_NUM ((uint32_t)0xDEADBEEF)

#ifdef USE_STM32F4XX_NUCLEO

  #if defined(STM32F401xE)

    #define BMS_MAX_PROG_SIZE (0x3FFFF-0x3FFF)
    /* OTA Position */
    #define BMS_OTA_ADDRESS_START  0x08040000

    /* Running program Position */
    #define BMS_PROG_ADDRESS_START 0x08004000

    /* OTA Position */
    #define BMS_OTA_SECTOR_START  FLASH_SECTOR_6
    /* 128 Kbytes we don't clean the latest Sector where there are the osxMotion Licenses */
    #define BMS_OTA_NUM_SECTORS  1

    /* Running program Position */
    #define BMS_PROG_SECTOR_START  FLASH_SECTOR_1
    /* 240 Kbytes */
    #define BMS_PROG_NUM_SECTORS 5

  #elif defined(STM32F429xx)
  
    #define BMS_MAX_PROG_SIZE (0x7FFFF-0x3FFF)
    /* OTA Position */
    #define BMS_OTA_ADDRESS_START  0x08080000

    /* Running program Position */
    #define BMS_PROG_ADDRESS_START 0x08004000

    /* OTA Position */
    #define BMS_OTA_SECTOR_START  FLASH_SECTOR_8
    /* 512 Kbytes */
    #define BMS_OTA_NUM_SECTORS  4

    /* Running program Position */
    #define BMS_PROG_SECTOR_START  FLASH_SECTOR_1
    /* 496 Kbytes */
    #define BMS_PROG_NUM_SECTORS 7
  
  #endif

#endif /* USE_STM32F4XX_NUCLEO */

#ifdef USE_STM32L4XX_NUCLEO
  #define BMS_MAX_PROG_SIZE (0x7FFFF-0x3FFF)
  /* OTA Position */
  #define BMS_OTA_ADDRESS_START  0x08080000

  /* Running program Position */
  #define BMS_PROG_ADDRESS_START 0x08004000

  /* OTA  */
  /* remove only the Magic Number (2bytes) */
  #define BMS_OTA_NUM_PAGES  1

  /* Running program */
  /* 496 Kbytes */
  #define BMS_PROG_NUM_PAGES 248
#endif /* USE_STM32L4XX_NUCLEO */

/* Private variables ---------------------------------------------------------*/
#pragma location=".version"
__root const BootLoaderFeatures_t BootLoaderFeatures={((BL_VERSION_MAJOR<<16) | (BL_VERSION_MINOR<<8) | BL_VERSION_PATCH),
                                                      BMS_OTA_CHECK_MAGIC_NUM,
                                                      BMS_OTA_ADDRESS_START,
                                                      BMS_MAX_PROG_SIZE,
                                                      BMS_PROG_ADDRESS_START};

#pragma location=".wifistring"
__root const char WifiString[]={"ERROR\0""\r\nOK\r\n\0""OK\r\n\0""OK\r\0""OK\0"};

/* Private function prototypes -----------------------------------------------*/
static void SystemClock_Config(void);

#ifdef USE_STM32L4XX_NUCLEO
static uint32_t GetPage(uint32_t Addr);
#endif /* USE_STM32L4XX_NUCLEO */

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
  uint32_t Address = BMS_OTA_ADDRESS_START;
  __IO uint32_t data32 = *(__IO uint32_t*) Address;
  
  /* Check if There is a New OTA */
  if(data32==BMS_OTA_CHECK_MAGIC_NUM) {    
    /* Make the OTA */
    HAL_Init();
    /* Configure the System clock */
    SystemClock_Config();

    /* Reset the First Flash Section */
    {
      FLASH_EraseInitTypeDef EraseInitStruct;
      uint32_t SectorError = 0;
#ifdef USE_STM32F4XX_NUCLEO
      EraseInitStruct.TypeErase    = TYPEERASE_SECTORS;
      EraseInitStruct.VoltageRange = VOLTAGE_RANGE_3;
      EraseInitStruct.Sector       = BMS_PROG_SECTOR_START;
      EraseInitStruct.NbSectors    = BMS_PROG_NUM_SECTORS;
#endif /* USE_STM32F4XX_NUCLEO */

#ifdef USE_STM32L4XX_NUCLEO
      EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
      EraseInitStruct.Banks       = FLASH_BANK_1;
      EraseInitStruct.Page        = GetPage(BMS_PROG_ADDRESS_START);
      EraseInitStruct.NbPages     = BMS_PROG_NUM_PAGES;
#endif /* USE_STM32L4XX_NUCLEO */

      /* Unlock the Flash to enable the flash control register access *************/
      HAL_FLASH_Unlock();
      
      if(HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError) != HAL_OK){
        /* Error occurred while sector erase. 
          User can add here some code to deal with this error. 
          SectorError will contain the faulty sector and then to know the code error on this sector,
          user can call function 'HAL_FLASH_GetError()'
          FLASH_ErrorTypeDef errorcode = HAL_FLASH_GetError(); */
        while(1);
      }

      /* Lock the Flash to disable the flash control register access (recommended
      to protect the FLASH memory against possible unwanted operation) *********/
      HAL_FLASH_Lock();
    }

    /* Make the OTA */
#ifdef USE_STM32F4XX_NUCLEO
    {
      int32_t WriteIndex;
      uint32_t *OTAAddress = (uint32_t *) (BMS_OTA_ADDRESS_START+8 /* For Skipping the Magic Number (Aligned to 8 for L4)*/);
      uint32_t ProgAddress = (uint32_t  ) BMS_PROG_ADDRESS_START;
      
      /* Unlock the Flash to enable the flash control register access *************/
      HAL_FLASH_Unlock();

      for(WriteIndex=0;WriteIndex<BMS_MAX_PROG_SIZE;WriteIndex+=4){
        if (HAL_FLASH_Program(TYPEPROGRAM_WORD, ProgAddress+WriteIndex,OTAAddress[WriteIndex>>2]) != HAL_OK){
          /* Error occurred while writing data in Flash memory.
             User can add here some code to deal with this error
             FLASH_ErrorTypeDef errorcode = HAL_FLASH_GetError(); */
          while(1);
        }
      }

      /* Lock the Flash to disable the flash control register access (recommended
       to protect the FLASH memory against possible unwanted operation) *********/
      HAL_FLASH_Lock();
    }
#endif /* USE_STM32F4XX_NUCLEO */
    
#ifdef USE_STM32L4XX_NUCLEO
    {
      int32_t WriteIndex;
      uint64_t *OTAAddress = (uint64_t *) (BMS_OTA_ADDRESS_START+8 /* For Skipping the Magic Number (Aligned to 8 for L4) */);
      uint32_t ProgAddress = (uint32_t  ) BMS_PROG_ADDRESS_START;
      
      /* Unlock the Flash to enable the flash control register access *************/
      HAL_FLASH_Unlock();

      for(WriteIndex=0;WriteIndex<BMS_MAX_PROG_SIZE;WriteIndex+=8){
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, ProgAddress+WriteIndex,OTAAddress[WriteIndex>>3]) != HAL_OK){
          /* Error occurred while writing data in Flash memory.
             User can add here some code to deal with this error
             FLASH_ErrorTypeDef errorcode = HAL_FLASH_GetError(); */
          while(1);
        }
      }

      /* Lock the Flash to disable the flash control register access (recommended
       to protect the FLASH memory against possible unwanted operation) *********/
      HAL_FLASH_Lock();
    }
#endif /* USE_STM32L4XX_NUCLEO */

    /* Reset the Second Half of the Flash except where is stored the License Manager*/
    {
      FLASH_EraseInitTypeDef EraseInitStruct;
      uint32_t SectorError = 0;
      
      /* Unlock the Flash to enable the flash control register access *************/
      HAL_FLASH_Unlock();

      /* Reset the Second half Flash */
#ifdef USE_STM32F4XX_NUCLEO
      EraseInitStruct.TypeErase    = TYPEERASE_SECTORS;
      EraseInitStruct.VoltageRange = VOLTAGE_RANGE_3;
      EraseInitStruct.Sector       = BMS_OTA_SECTOR_START;
      EraseInitStruct.NbSectors    = BMS_OTA_NUM_SECTORS;
#endif /* USE_STM32F4XX_NUCLEO */

#ifdef USE_STM32L4XX_NUCLEO
      EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
      EraseInitStruct.Banks       = FLASH_BANK_2;
      EraseInitStruct.Page        = GetPage(BMS_OTA_ADDRESS_START);
      EraseInitStruct.NbPages     = BMS_OTA_NUM_PAGES;
#endif /* USE_STM32L4XX_NUCLEO */
      
      if(HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError) != HAL_OK){
        /* Error occurred while sector erase. 
          User can add here some code to deal with this error. 
          SectorError will contain the faulty sector and then to know the code error on this sector,
          user can call function 'HAL_FLASH_GetError()'
          FLASH_ErrorTypeDef errorcode = HAL_FLASH_GetError(); */
        while(1);
      }
      
      /* Lock the Flash to disable the flash control register access (recommended
       to protect the FLASH memory against possible unwanted operation) *********/
      HAL_FLASH_Lock();
    }

    /* System Reboot */
    HAL_NVIC_SystemReset();
  } else {
    /* Jump To Normal boot */
    typedef  void (*pFunction)(void);

    pFunction JumpToApplication;
    uint32_t JumpAddress;

    /* reset all interrupts to default */
   // __disable_irq();

    /* Jump to system memory */
    JumpAddress = *(__IO uint32_t*) (BMS_PROG_ADDRESS_START + 4);
    JumpToApplication = (pFunction) JumpAddress;
    /* Initialize user application's Stack Pointer */
    __set_MSP(*(__IO uint32_t*) BMS_PROG_ADDRESS_START);
    JumpToApplication();
  }
}

#ifdef USE_STM32L4XX_NUCLEO
/**
  * @brief  Gets the page of a given address
  * @param  Addr: Address of the FLASH Memory
  * @retval The page of a given address
  */
static uint32_t GetPage(uint32_t Addr)
{
  uint32_t page = 0;
  
  if (Addr < (FLASH_BASE + FLASH_BANK_SIZE))
  {
    /* Bank 1 */
    page = (Addr - FLASH_BASE) / FLASH_PAGE_SIZE;
  }
  else
  {
    /* Bank 2 */
    page = (Addr - (FLASH_BASE + FLASH_BANK_SIZE)) / FLASH_PAGE_SIZE;
  }
  
  return page;
}
#endif /* USE_STM32L4XX_NUCLEO */

#ifdef USE_STM32F4XX_NUCLEO
#ifdef OSX_BMS_NUCLEO
/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow:
  *            System Clock source            = PLL (HSI)
  *            SYSCLK(Hz)                     = 84000000
  *            HCLK(Hz)                       = 84000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 2
  *            APB2 Prescaler                 = 1
  *            HSI Frequency(Hz)              = 16000000
  *            PLL_M                          = 16
  *            PLL_N                          = 336
  *            PLL_P                          = 4
  *            PLL_Q                          = 7
  *            VDD(V)                         = 3.3
  *            Main regulator output voltage  = Scale2 mode
  *            Flash Latency(WS)              = 2
  * @param  None
  * @retval None
  */
static void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;

  /* Enable Power Control clock */
  __PWR_CLK_ENABLE();

  /* The voltage scaling allows optimizing the power consumption when the device is
     clocked below the maximum system frequency, to update the voltage scaling value
     regarding system frequency refer to product datasheet.  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /* Enable HSI Oscillator and activate PLL with HSI as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = 0x10;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK){
    while(1);
  }

  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK){
    while(1);
  }
}
#elif OSX_BMS_BLUECOIN
/**
* @brief  System Clock Configuration
*         The system Clock is configured as follow : 
*            System Clock source            = PLL (HSE)
*            SYSCLK(Hz)                     = 168000000
*            HCLK(Hz)                       = 168000000
*            AHB Prescaler                  = 1
*            APB1 Prescaler                 = 4
*            APB2 Prescaler                 = 2
*            HSE Frequency(Hz)              = 8000000
*            PLL_M                          = 8
*            PLL_N                          = 336
*            PLL_P                          = 2
*            PLL_Q                          = 7
*            VDD(V)                         = 3.3
*            Main regulator output voltage  = Scale1 mode
*            Flash Latency(WS)              = 5
*         The USB clock configuration from PLLSAI:
*            PLLSAIM                        = 8
*            PLLSAIN                        = 384
*            PLLSAIP                        = 8
* @param  None
* @retval None
*/
static void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;
  
  /* Enable Power Control clock */
  __HAL_RCC_PWR_CLK_ENABLE();
  
  /* The voltage scaling allows optimizing the power consumption when the device is 
  clocked below the maximum system frequency, to update the voltage scaling value 
  regarding system frequency refer to product datasheet.  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  
  /* Enable HSE Oscillator and activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  #ifdef USE_BLUECOINPLUS   
  RCC_OscInitStruct.PLL.PLLM = 16;
#else
  RCC_OscInitStruct.PLL.PLLM = 8;
#endif
  RCC_OscInitStruct.PLL.PLLN = 336;//192;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;//4;
  RCC_OscInitStruct.PLL.PLLR = 2;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);
  
  /* Activate the OverDrive to reach the 180 MHz Frequency */  
  //HAL_PWREx_EnableOverDrive();
  
  /*Select Main PLL output as USB clock source */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_CK48;
  PeriphClkInitStruct.Clk48ClockSelection = RCC_CK48CLKSOURCE_PLLQ;
  HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);
  
  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 
  clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;  
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;  
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);
}
#endif /* OSX_BMS_NUCLEO */
#endif /* USE_STM32F4XX_NUCLEO */

#ifdef USE_STM32L4XX_NUCLEO
#ifdef OSX_BMS_NUCLEO
/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow : 
  *            System Clock source            = PLL (MSI)
  *            SYSCLK(Hz)                     = 80000000
  *            HCLK(Hz)                       = 80000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 1
  *            APB2 Prescaler                 = 1
  *            MSI Frequency(Hz)              = 4000000
  *            PLL_M                          = 1
  *            PLL_N                          = 40
  *            PLL_R                          = 2
  *            PLL_P                          = 7
  *            PLL_Q                          = 4
  *            Flash Latency(WS)              = 4
  * @param  None
  * @retval None
  */
static void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;

  /* MSI is enabled after System reset, activate PLL with MSI as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 40;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLP = 7;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK){
    /* Initialization Error */
    while(1);
  }
  
  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 
     clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;  
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;  
  if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK) {
    /* Initialization Error */
    while(1);
  }
}
#elif OSX_BMS_SENSORTILE
/**
* @brief  System Clock Configuration
* @param  None
* @retval None
*/
void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
  
  __HAL_RCC_PWR_CLK_ENABLE();
  HAL_PWR_EnableBkUpAccess();
  
  /* Enable the LSE Oscilator */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK){
    while(1);
  }
  
  /* Enable the CSS interrupt in case LSE signal is corrupted or not present */
  HAL_RCCEx_DisableLSECSS();
  
  /* Enable MSI Oscillator and activate PLL with MSI as source */
  RCC_OscInitStruct.OscillatorType      = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState            = RCC_MSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.MSIClockRange       = RCC_MSIRANGE_11;
  RCC_OscInitStruct.PLL.PLLState        = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource       = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM            = 6;
  RCC_OscInitStruct.PLL.PLLN            = 40;
  RCC_OscInitStruct.PLL.PLLP            = 7;
  RCC_OscInitStruct.PLL.PLLQ            = 4;
  RCC_OscInitStruct.PLL.PLLR            = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK){
    while(1);
  }
  
  /* Enable MSI Auto-calibration through LSE */
  HAL_RCCEx_EnableMSIPLLMode();
  
  /* Select MSI output as USB clock source */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USB;
  PeriphClkInitStruct.UsbClockSelection = RCC_USBCLKSOURCE_MSI;
  HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);
  
  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 
  clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK){
    while(1);
  }
}
#endif /* OSX_BMS_NUCLEO */
#endif /* USE_STM32L4XX_NUCLEO */

/******************* (C) COPYRIGHT 2015 STMicroelectronics *****END OF FILE****/
