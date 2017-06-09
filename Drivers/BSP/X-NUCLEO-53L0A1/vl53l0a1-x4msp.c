/**
 * @file vl53l0A1-x4msp.c
 *
 * interrupt MSP and User example code for 53L0A1 BPS and STM32x4
 */

#include  "X-NUCLEO-53L0A1.h"


#if  VL53L0A1_HAVE_UART
/**
 * uart2 handle
 */
UART_HandleTypeDef huart2;

#if VL53L0A1_UART_DMA_RX
DMA_HandleTypeDef hdma_usart2_rx;
#endif

#if VL53L0A1_UART_DMA_TX
DMA_HandleTypeDef hdma_usart2_tx;
#endif
#endif

 /**
 * User override callback for all interrupt
 *
 * @note End user application is responsible to find initiator sensor for shared interrupt configuration
 *
 * @note If several interrupt line shares the same vector it may be called several time in one interrupt.
 *
 * @param DevNo     DeviceNumber  (for shared interrupt is  center always)
 * @param GPIO_Pin  EXTI Gpio Pin associated to the interrupt
 *
 * @ingroup   MSP_implement_common
 */
__weak void VL53L0A1_EXTI_Callback(int DevNo, int GPIO_Pin){
    /**
     * built-in Does nothing redefined your's
     */
}

/**
 * @ingroup  MSP_implement
 * @defgroup MSP_implement_STM32x4  MSP STM32F4xx and STM32L4xx
 *
 * MSP code for STM32F4xxx and STM3L4xx tested with Nucleo F401RE and L476
 *
 * @{
 */

#if  VL53L0A1_HAVE_UART

/** Pin used by USART TX */
#define USART_TX_Pin GPIO_PIN_2
/** port used by USART TX */
#define USART_TX_GPIO_Port GPIOA
/** pin used by USART RX */
#define USART_RX_Pin GPIO_PIN_3
/** port used by USART RX */
#define USART_RX_GPIO_Port GPIOA


#ifdef STM32F401xE

#ifndef VL53L0A1_UART_IRQ_PRI
/**
 * User can override default uart irq priority 0 to fit aplciation needs
 */
#   define VL53L0A1_UART_IRQ_PRI    0
#endif

#if  VL53L0A1_UART_DMA_RX
/**
* @brief This function handles DMA1 stream5 global interrupt.
*/
void DMA1_Stream5_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&hdma_usart2_rx);
}
#endif // VL53L0A1_UART_DMA_RX

#if VL53L0A1_UART_DMA_TX
/**
* @brief This function handles DMA1 stream6 global interrupt.
*/
void DMA1_Stream6_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&hdma_usart2_tx);
}
#endif  //VL53L0A1_UART_DMA_TX


static void XNUCLEO53L0A1_DMA_Init(void)
{
  /* DMA controller clock enable */
  __DMA1_CLK_ENABLE();

  /* DMA interrupt init */
#if  VL53L0A1_UART_DMA_RX
  HAL_NVIC_SetPriority(DMA1_Stream5_IRQn, VL53L0A1_UART_DMA_TX_IRQ_PRI, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream5_IRQn);
#endif
#if  VL53L0A1_UART_DMA_TX
  HAL_NVIC_SetPriority(DMA1_Stream6_IRQn, VL53L0A1_UART_DMA_TX_IRQ_PRI, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream6_IRQn);
#endif

}
/* USART2 init function */

void XNUCLEO53L0A1_USART2_UART_Init(void) {

    XNUCLEO53L0A1_DMA_Init();

    huart2.Instance = USART2;
    huart2.Init.BaudRate = USART2_BAUD_RATE;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = UART_PARITY_NONE;
    huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;
    HAL_UART_Init(&huart2);


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
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* Peripheral DMA init*/
#if VL53L0A1_UART_DMA_TX
    hdma_usart2_tx.Instance = DMA1_Stream6;
    hdma_usart2_tx.Init.Channel = DMA_CHANNEL_4;
    hdma_usart2_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_usart2_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart2_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart2_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart2_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart2_tx.Init.Mode = DMA_NORMAL;
    hdma_usart2_tx.Init.Priority = DMA_PRIORITY_LOW;
    hdma_usart2_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    HAL_DMA_Init(&hdma_usart2_tx);

    __HAL_LINKDMA(huart,hdmatx,hdma_usart2_tx);
#endif
#if  VL53L0A1_UART_DMA_RX
    hdma_usart2_rx.Instance = DMA1_Stream5;
    hdma_usart2_rx.Init.Channel = DMA_CHANNEL_4;
    hdma_usart2_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_usart2_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart2_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart2_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart2_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart2_rx.Init.Mode = DMA_NORMAL;
    hdma_usart2_rx.Init.Priority = DMA_PRIORITY_LOW;
    hdma_usart2_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    HAL_DMA_Init(&hdma_usart2_rx);

    __HAL_LINKDMA(huart,hdmarx,hdma_usart2_rx);
#endif
    HAL_NVIC_SetPriority(USART2_IRQn, VL53L0A1_UART_IRQ_PRI, 0);
    HAL_NVIC_EnableIRQ(USART2_IRQn);
  }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* huart)
{

  if(huart->Instance==USART2)
  {
    /* Peripheral clock disable */
    __USART2_CLK_DISABLE();

    /**USART2 GPIO Configuration
    PA2     ------> USART2_TX
    PA3     ------> USART2_RX
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_2|GPIO_PIN_3);

    /* Peripheral DMA DeInit*/
#if  VL53L0A1_UART_DMA_RX
    HAL_DMA_DeInit(huart->hdmarx);
#endif
#if  VL53L0A1_UART_DMA_TX
    HAL_DMA_DeInit(huart->hdmatx);
#endif
    /* Peripheral interrupt DeInit*/
    HAL_NVIC_DisableIRQ(USART2_IRQn);

  }
}

#endif

#ifdef STM32L476xx

#if VL53L0A1_UART_DMA_RX
/**
* @brief This function handles DMA1 channel6 global interrupt.
*/
void DMA1_Channel6_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&hdma_usart2_rx);
}
#endif

#if VL53L0A1_UART_DMA_TX
/**
* @brief This function handles DMA1 channel7 global interrupt.
*/
void DMA1_Channel7_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&hdma_usart2_tx);
}
#endif
/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{
  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
#if VL53L0A1_UART_DMA_RX
  HAL_NVIC_SetPriority(DMA1_Channel6_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel6_IRQn);
#endif
#if VL53L0A1_UART_DMA_TX
  HAL_NVIC_SetPriority(DMA1_Channel7_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel7_IRQn);
#endif

}

/* USART2 init function */
void XNUCLEO53L0A1_USART2_UART_Init(void)
{
  MX_DMA_Init();
  huart2.Instance = USART2;
  huart2.Init.BaudRate = USART2_BAUD_RATE;
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

void HAL_UART_MspInit(UART_HandleTypeDef* huart)
{

  GPIO_InitTypeDef GPIO_InitStruct;
  if(huart->Instance==USART2)
  {
  /* USER CODE BEGIN USART2_MspInit 0 */

  /* USER CODE END USART2_MspInit 0 */
    /* Peripheral clock enable */
    __USART2_CLK_ENABLE();

    /**USART2 GPIO Configuration
    PA2     ------> USART2_TX
    PA3     ------> USART2_RX
    */
    GPIO_InitStruct.Pin = USART_TX_Pin|USART_RX_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* Peripheral DMA init*/
#if VL53L0A1_UART_DMA_TX
    hdma_usart2_tx.Instance = DMA1_Channel7;
    hdma_usart2_tx.Init.Request = DMA_REQUEST_2;
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
    hdma_usart2_rx.Instance = DMA1_Channel6;
    hdma_usart2_rx.Init.Request = DMA_REQUEST_2;
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
/* USER CODE BEGIN USART2_MspInit 1 */
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* huart)
{

  if(huart->Instance==USART2)
  {
    /* Peripheral clock disable */
    __USART2_CLK_DISABLE();

    /**USART2 GPIO Configuration
    PA2     ------> USART2_TX
    PA3     ------> USART2_RX
    */
    HAL_GPIO_DeInit(GPIOA, USART_TX_Pin|USART_RX_Pin);

    /* Peripheral DMA DeInit*/
#if VL53L0A1_UART_DMA_TX
    HAL_DMA_DeInit(huart->hdmatx);
#endif
#if VL53L0A1_UART_DMA_RX
    HAL_DMA_DeInit(huart->hdmarx);
#endif
  }

}
#endif //STM32L476


/**
* @brief This function handles USART2 global interrupt.
*/
void USART2_IRQHandler(void)
{
  HAL_UART_IRQHandler(&huart2);
}

#endif //ifdef  VL53L0A1_HAVE_UART

/**
 * HAl Callback for EXTI
 * @param GPIO_Pin The GPIO pin EXTI was invoked
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
#if VL53L0A1_GPIO1_SHARED
    if( GPIO_Pin == VL53L0A1_GPIO1_C_GPIO_PIN ){
        VL53L0A1_EXTI_Callback(XNUCLEO53L0A1_DEV_CENTER, GPIO_Pin);
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


#ifdef VL53L0A1_EXTI1_USE_PIN
/**
 * interrupt Handler for EXTI lines 1
 *
 * @note  is only implemented if needed by configuration
 */
void EXTI1_IRQHandler(void)
{
   HAL_GPIO_EXTI_IRQHandler(VL53L0A1_EXTI1_USE_PIN);
}
#endif //ifdef VL53L0A1_EXTI9_5_USAGE

#ifdef VL53L0A1_EXTI4_USE_PIN

/**
 * interrupt Handler for EXTI lines 4
 *
 * @note is only implemented if needed by configuration
 */
void EXTI4_IRQHandler(void)
{
   HAL_GPIO_EXTI_IRQHandler(VL53L0A1_EXTI4_USE_PIN);
}
#endif //ifdef VL53L0A1_EXTI9_5_USAGE


#ifdef VL53L0A1_EXTI9_5_USE_PIN
/**
 * interrupt Handler for EXTI lines 9 to  5
 *
 * @note is only implemented if needed by configuration
 * @warning this handler assume ther's no shared pin on the EXTI
 * For share interrupt  end user must fix this code to find the originating pins
 * and dispatch accordingly see __HAL_GPIO_EXTI_GET_FLAG  etc ..
 */
void EXTI9_5_IRQHandler(void)
{
   HAL_GPIO_EXTI_IRQHandler(VL53L0A1_EXTI9_5_USE_PIN);
}
#endif //ifdef VL53L0A1_EXTI9_5_USAGE

#ifdef VL53L0A1_EXTI15_10_USE_PIN
/**
 * interrupt Handler for EXTI lines 15 to  10
 *
 * @note is only implemented if needed by configuration
 * @warning this handler assume ther's no shared pin on the EXTI
 * For share interrupt  end user must fix this code to find the originating pins
 * and dispatch accordingly see __HAL_GPIO_EXTI_GET_FLAG  etc ..
 */
void EXTI15_10_IRQHandler(void){
    HAL_GPIO_EXTI_IRQHandler(VL53L0A1_EXTI15_10_USE_PIN);
}
#endif //VL53L0A1_EXTI15_10_USE_PIN

void VL53L0A1_EXTI_Init(int  IntPriority, int SubPriority){
    GPIO_InitTypeDef GPIO_InitStruct;


    VL53L0A1_GPIO1_C_CLK_ENABLE();
    /*Configure GPIO pin : PA4 */
    GPIO_InitStruct.Pin = VL53L0A1_GPIO1_C_GPIO_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
    GPIO_InitStruct.Pull = VL53L0A1_INTR_PIN_PUPD;

    XNUCLEO53L0A1_SetIntrStateId(0,XNUCLEO53L0A1_DEV_CENTER);
    HAL_GPIO_Init(VL53L0A1_GPIO1_C_GPIO_PORT, &GPIO_InitStruct);

#if VL53L0A1_GPIO1_SHARED == 0
    /* non shared config enable also satellite i/o as interrupt*/
    VL53L0A1_GPIO1_L_CLK_ENABLE();
    XNUCLEO53L0A1_SetIntrStateId(0,XNUCLEO53L0A1_DEV_LEFT);
    GPIO_InitStruct.Pin = VL53L0A1_GPIO1_L_GPIO_PIN;
    HAL_GPIO_Init(VL53L0A1_GPIO1_L_GPIO_PORT, &GPIO_InitStruct);
    HAL_NVIC_SetPriority(VL53L0A1_GPIO1_L_INTx, IntPriority, SubPriority);

    VL53L0A1_GPIO1_R_CLK_ENABLE();
    XNUCLEO53L0A1_SetIntrStateId(0,XNUCLEO53L0A1_DEV_RIGHT);
    GPIO_InitStruct.Pin = VL53L0A1_GPIO1_R_GPIO_PIN;
    HAL_GPIO_Init(VL53L0A1_GPIO1_R_GPIO_PORT, &GPIO_InitStruct);
    HAL_NVIC_SetPriority(VL53L0A1_GPIO1_R_INTx, IntPriority, SubPriority);
#endif
}

/**  @} */ /* defgroup MSP_implement_STM32x4 */
