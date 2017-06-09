/*
 * vl53l0xa1-l053msp.c
 *
 * interrupt MSP and User example code for 53L0XA1 BSP on STM32L053
 */

#include  "X-NUCLEO-53L0A1.h"


__weak void VL53L0A1_EXTI_Callback(int DevNo, int GPIO_Pin){
}


#ifdef VL53L0A1_HAVE_UART

UART_HandleTypeDef huart2;
#if VL53L0A1_UART_DMA_TX
DMA_HandleTypeDef hdma_usart2_tx;
#endif
#if VL53L0A1_UART_DMA_RX
DMA_HandleTypeDef hdma_usart2_rx;
#endif


static void MX_DMA_Init(void)
{
  /* DMA controller clock enable */
  __DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  HAL_NVIC_SetPriority(DMA1_Channel4_5_6_7_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel4_5_6_7_IRQn);

}


/* USART2 init function */
void XNUCLEO53L0A1_USART2_UART_Init()
{
  MX_DMA_Init();
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONEBIT_SAMPLING_DISABLED;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  HAL_UART_Init(&huart2);

}

/**
* @brief This function handles DMA1 channel 4, channel 5, channel 6 and channel 7 interrupts.
*/
void DMA1_Channel4_5_6_7_IRQHandler(void)
{
#if VL53L0A1_UART_DMA_TX
  HAL_DMA_IRQHandler(&hdma_usart2_tx);
#endif
#if VL53L0A1_UART_DMA_RX
  HAL_DMA_IRQHandler(&hdma_usart2_rx);
#endif
}


/**
* @brief This function handles USART2 global interrupt.
*/
void USART2_IRQHandler(void)
{
  HAL_UART_IRQHandler(&huart2);
}

void HAL_UART_MspInit(UART_HandleTypeDef* huart)
{

  GPIO_InitTypeDef GPIO_InitStruct;
  if(huart->Instance==USART2)
  {
    /* Peripheral clock enable */
    __USART2_CLK_ENABLE();

    /**USART2 GPIO Configuration
    PA2     ------> USART2_TX
    PA3     ------> USART2_RX
    */
    GPIO_InitStruct.Pin = USART_TX_Pin|USART_RX_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF4_USART2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* Peripheral DMA init*/
#if VL53L0A1_UART_DMA_TX
    hdma_usart2_tx.Instance = DMA1_Channel4;
    hdma_usart2_tx.Init.Request = DMA_REQUEST_4;
    hdma_usart2_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_usart2_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart2_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart2_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart2_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart2_tx.Init.Mode = DMA_NORMAL;
    hdma_usart2_tx.Init.Priority = DMA_PRIORITY_LOW;
    HAL_DMA_Init(&hdma_usart2_tx);

    __HAL_LINKDMA(huart,hdmatx,hdma_usart2_tx);
#endif
#if VL53L0A1_UART_DMA_RX
    hdma_usart2_rx.Instance = DMA1_Channel5;
    hdma_usart2_rx.Init.Request = DMA_REQUEST_4;
    hdma_usart2_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_usart2_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart2_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart2_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart2_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart2_rx.Init.Mode = DMA_NORMAL;
    hdma_usart2_rx.Init.Priority = DMA_PRIORITY_LOW;
    HAL_DMA_Init(&hdma_usart2_rx);

    __HAL_LINKDMA(huart,hdmarx,hdma_usart2_rx);
#endif
    HAL_NVIC_SetPriority(USART2_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USART2_IRQn);
  }

}

void HAL_UART_MspDeInit(UART_HandleTypeDef* huart)
{

  if(huart->Instance==USART2){
    /* Peripheral clock disable */
    __USART2_CLK_DISABLE();

    /**USART2 GPIO Configuration
    PA2     ------> USART2_TX
    PA3     ------> USART2_RX
    */
    HAL_GPIO_DeInit(GPIOA, USART_TX_Pin|USART_RX_Pin);

    /* Peripheral DMA DeInit*/
    HAL_DMA_DeInit(huart->hdmatx);
    HAL_DMA_DeInit(huart->hdmarx);
  }
}
#endif //VL53L0A1_HAVE_UART

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
#if VL53L0A1_GPIO1_SHARED
    if( GPIO_Pin == VL53L0A1_GPIO1_T_GPIO_PIN ){
        VL53L0A1_EXTI_Callback(XNUCLEO53L0A1_DEV_TOP, GPIO_Pin);
    }
#else
    switch(  GPIO_Pin  ){
    case VL53L0A1_GPIO1_C_GPIO_PIN :
        VL53L0A1_EXTI_Callback(XNUCLEO53L0A1_DEV_CENTER, GPIO_Pin);
        break;
    case VL53L0A1_GPIO1_L_GPIO_PIN :
        VL53L0A1_EXTI_Callback(XNUCLEO53L0A1_DEV_LEFT, GPIO_Pin);
        break;
    case VL53L0A1_GPIO1_R_GPIO_PIN :
        VL53L0A1_EXTI_Callback(XNUCLEO53L0A1_DEV_RIGHT, GPIO_Pin);
        break;

    default:
        break;
    }
#endif
}


#ifdef VL53L0A1_EXTI0_1_USE_PIN
void EXTI0_1_IRQHandler(void)
{
   HAL_GPIO_EXTI_IRQHandler(VL53L0A1_EXTI0_1_USE_PIN);
}
#endif //ifdef VL53L0A1_EXTI0_1_USE_PIN

#ifdef VL53L0A1_EXTI4_15_USE_PIN
void EXTI4_15_IRQHandler(void)
{
    int ExtiFlag;
    /*this vector may be shared for several line (all 3 )
     * we must check for any of the lane and call their relative handler
     * FIXME this may not work well and we may miss or double sereve line if it appear during handler execution */
    ExtiFlag = __HAL_GPIO_EXTI_GET_IT(0xFFF0);
    if( ExtiFlag &VL53L0A1_EXTI4_15_USE_PIN ){
        HAL_GPIO_EXTI_IRQHandler(VL53L0A1_EXTI4_15_USE_PIN);
        ExtiFlag &= ~VL53L0A1_EXTI4_15_USE_PIN;
    }
#ifdef VL53L0A1_EXTI4_15_USE_PIN_2
    if( ExtiFlag&VL53L0A1_EXTI4_15_USE_PIN_2 ){
        HAL_GPIO_EXTI_IRQHandler(VL53L0A1_EXTI4_15_USE_PIN_2);
        ExtiFlag &=~VL53L0A1_EXTI4_15_USE_PIN_2;
    }
#endif
#ifdef VL53L0A1_EXTI4_15_USE_PIN_3
    if( ExtiFlag &VL53L0A1_EXTI4_15_USE_PIN_3 ){
        HAL_GPIO_EXTI_IRQHandler(VL53L0A1_EXTI4_15_USE_PIN_3);
        ExtiFlag &=~VL53L0A1_EXTI4_15_USE_PIN_3;
    }
#endif
}
#endif //ifdef VL53L0A1_EXTI4_15_USE_PIN

