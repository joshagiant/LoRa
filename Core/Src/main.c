/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "extern.h"
#include "i2c-lcd.h"
#include "lora.h"
#include <string.h>
#include <stdio.h>
#include "ssd1306.h"
#include "semphr.h"
#include "FreeRTOS.h"
#include <task.h>


/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;
DMA_HandleTypeDef hdma_i2c1_tx;

SPI_HandleTypeDef hspi1;
DMA_HandleTypeDef hdma_spi1_tx;
DMA_HandleTypeDef hdma_spi1_rx;

osThreadId defaultTaskHandle;
osThreadId loraTaskHandle;
osThreadId displayTaskHandle;
osTimerId msTickHandle;
osTimerId debounceTimerHandle;
osTimerId blinkTimerHandle;
osMutexId lora_mutexHandle;
/* USER CODE BEGIN PV */
//SemaphoreHandle_t xloraMutex;
LoRa myLoRa;


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_SPI1_Init(void);
static void MX_I2C1_Init(void);
void StartDefaultTask(void const * argument);
void StartLoraTask(void const * argument);
void StartDisplayTask(void const * argument);
void msTickCallback(void const * argument);
void debounceCallback(void const * argument);
void blinkTimerCallback(void const * argument);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_SPI1_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */



  /* USER CODE END 2 */

  /* Create the mutex(es) */
  /* definition and creation of lora_mutex */
  osMutexDef(lora_mutex);
  lora_mutexHandle = osMutexCreate(osMutex(lora_mutex));

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */

  //xloraMutex = xSemaphoreCreateMutex();

//debugBuddy = pdTRUE;
/*
if (xloraMutex != NULL) 
{
  debugBuddy = xloraMutex;
}
*/

  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* Create the timer(s) */
  /* definition and creation of msTick */
  osTimerDef(msTick, msTickCallback);
  msTickHandle = osTimerCreate(osTimer(msTick), osTimerPeriodic, NULL);

  /* definition and creation of debounceTimer */
  osTimerDef(debounceTimer, debounceCallback);
  debounceTimerHandle = osTimerCreate(osTimer(debounceTimer), osTimerOnce, NULL);

  /* definition and creation of blinkTimer */
  osTimerDef(blinkTimer, blinkTimerCallback);
  blinkTimerHandle = osTimerCreate(osTimer(blinkTimer), osTimerOnce, NULL);

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
 
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 128);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* definition and creation of loraRXTask */
  

  /* definition and creation of displayTask */
  

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 78;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Stream6_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream6_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream6_IRQn);
  /* DMA2_Stream0_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);
  /* DMA2_Stream3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream3_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream3_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LED1_Pin|LED2_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, LORA_RST_Pin|LORA_CS_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin : PLAYER_SEL_Pin */
  GPIO_InitStruct.Pin = PLAYER_SEL_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(PLAYER_SEL_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : BTN_IN_Pin */
  GPIO_InitStruct.Pin = BTN_IN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(BTN_IN_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LED1_Pin LED2_Pin */
  GPIO_InitStruct.Pin = LED1_Pin|LED2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : LORA_EXTI_Pin */
  GPIO_InitStruct.Pin = LORA_EXTI_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(LORA_EXTI_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LORA_RST_Pin LORA_CS_Pin */
  GPIO_InitStruct.Pin = LORA_RST_Pin|LORA_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
{
  /* USER CODE BEGIN 5 */
  

  osThreadDef(loraTask, StartLoraTask, osPriorityAboveNormal, 0, 128);
  loraTaskHandle = osThreadCreate(osThread(loraTask), NULL);

  osThreadDef(displayTask, StartDisplayTask, osPriorityNormal, 0, 128);
  displayTaskHandle = osThreadCreate(osThread(displayTask), NULL);

  if ( HAL_GPIO_ReadPin(PLAYER_SEL_GPIO_Port,PLAYER_SEL_Pin) == 0 )
  {
    THIS_PLAYER = BLUE;
    OTHER_PLAYER = RED;
  }

  else
  {
    THIS_PLAYER = RED;
    OTHER_PLAYER = BLUE;
  }

  REDLED_OFF;
  BLUELED_OFF;
  ssd1306_Init();
  ssd1306_SetCursor(0,0);
  ssd1306_WriteString("KING",Font_16x24,1);
  ssd1306_UpdateScreen();
  osDelay(500);
  ssd1306_Fill(0);
  ssd1306_SetCursor(64,0);
  ssd1306_WriteString("OF",Font_16x24,1);
  ssd1306_UpdateScreen();
  osDelay(500);
  ssd1306_Fill(0);
  ssd1306_SetCursor(0,8);
  ssd1306_WriteString("THE",Font_16x24,1);
  ssd1306_UpdateScreen();
  osDelay(500);
  ssd1306_Fill(0);
  ssd1306_SetCursor(64,8);
  ssd1306_WriteString("HILL",Font_16x24,1);
  ssd1306_UpdateScreen();

  

  osDelay(1000);

  resetGame();
  osTimerStart(msTickHandle, pdMS_TO_TICKS(10));
  

  /* Infinite loop */
  for(;;)
  {

    // if packet received, process it
    
      if(RXready_flag) decodeKOTHPacket();

      // Evaluate current state
      
      doGameState();
      
    
      osDelay(50);
    
  }
  /* USER CODE END 5 */
}

/* USER CODE BEGIN Header_StartLoraRXTask */
/**
* @brief Function implementing the loraRXTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartLoraRXTask */
void StartLoraTask(void const * argument)
{
  /* USER CODE BEGIN StartLoraTask */

    // Create LoRa instance
    myLoRa = newLoRa();

      // configure
      myLoRa.CS_port         = LORA_CS_GPIO_Port;
      myLoRa.CS_pin          = LORA_CS_Pin;
      myLoRa.reset_port      = LORA_RST_GPIO_Port;
      myLoRa.reset_pin       = LORA_RST_Pin;
      myLoRa.DIO0_port       = LORA_EXTI_GPIO_Port;
      myLoRa.DIO0_pin        = LORA_EXTI_Pin;
      myLoRa.hSPIx           = &hspi1;

      // Test it
      uint16_t LoRa_Status = LoRa_init(&myLoRa);
      sprintf(opcodeString,"Josh wins", 10);
      if(LoRa_Status == LORA_OK)
      {
        LoRa_transmit(&myLoRa, (uint8_t*)opcodeString, 12, 100);
      }

    LoRa_startReceiving(&myLoRa);

    rxCount = 0;
    txCount = 0;

  /* Infinite loop */
  for(;;)
  {
    // RX first
    // Get the data
    
    // to do: add 2nd/3rd broadcast to each event. Configurable. 

      packet_size = LoRa_receive(&myLoRa, loraRXbuf, 10);

      if( osMutexWait(lora_mutexHandle, osWaitForever) == osOK )        { 

        if(packet_size > 0)
        {
            RXready_flag = 1;

            if(loraRXbuf[0] != receivedPacket[0])
            {
                memcpy(receivedPacket,loraRXbuf,10);
            }

        }
          

   
        //osDelay(5);

        
        // TX for every meaningful button press
        if(TXready_flag)
        {
          TXready_flag = 0;
          // send a message
          LoRa_transmit(&myLoRa, (uint8_t*)opcodeString, 12, 100);

          // Twice for fun
          LoRa_transmit(&myLoRa, (uint8_t*)opcodeString, 12, 100);
        }
        
        osMutexRelease(lora_mutexHandle);

      } 
    

    osDelay(150);

  }
  /* USER CODE END StartLoraTask */
}

/* USER CODE BEGIN Header_StartDisplayTask */
/**
* @brief Function implementing the displayTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartDisplayTask */
void StartDisplayTask(void const * argument)
{
  /* USER CODE BEGIN StartDisplayTask */
  /* Infinite loop */
  for(;;)
  {
    switch(gameState)
    {
        case WAITING:
          ssd1306_Fill(0); 
          ssd1306_SetCursor(0,0);
          if (THIS_PLAYER == RED) 
          { 
              ssd1306_WriteString("RED",Font_11x18,1);
          }
          else    
          {
              ssd1306_WriteString("BLUE",Font_11x18,1);
          }
          ssd1306_SetCursor(25,21);
          ssd1306_WriteString("Push to play!",Font_7x10,1);
          ssd1306_UpdateScreen();

        break;


        case RED:
          ssd1306_Fill(0);
          ssd1306_SetCursor(0,0);
          ssd1306_WriteString("RED is king!",Font_7x10,1);
          ssd1306_SetCursor(0,12);
          sprintf(score, "R:%u", red_counter);
          ssd1306_WriteString(score,Font_7x10,1);
          ssd1306_UpdateScreen();               
        break;

        case BLUE:
      
          ssd1306_Fill(0);
          ssd1306_SetCursor(0,0);
          ssd1306_WriteString("BLUE is king!",Font_7x10,1);
          ssd1306_SetCursor(0,12);
          sprintf(score, "B:%u", blue_counter);
          ssd1306_WriteString(score,Font_7x10,1);
          ssd1306_UpdateScreen();
      
        break;

        case REDWINS:
  
          ssd1306_Fill(0);
          ssd1306_SetCursor(0,0);
          ssd1306_WriteString("Red wins!",Font_7x10,1);
          uint16_t difference = (red_counter - blue_counter);
          ssd1306_SetCursor(0,12);
          sprintf(finalScore, "Won by %ums", difference);
          ssd1306_WriteString(finalScore,Font_7x10,1);
          ssd1306_UpdateScreen();
            
        break;

        case BLUEWINS:

          ssd1306_Fill(0);
          ssd1306_SetCursor(0,0);
          ssd1306_WriteString("Blue wins!",Font_7x10,1);
          ssd1306_SetCursor(0,12);
          uint16_t differenceB = (blue_counter - red_counter);
          sprintf(finalScore, "Won by %ums", differenceB);
          ssd1306_WriteString(finalScore,Font_7x10,1);
          ssd1306_UpdateScreen();
          
        break;
    }


    osDelay(150);
  }
  /* USER CODE END StartDisplayTask */
}

/* msTickCallback function */
void msTickCallback(void const * argument)
{
  /* USER CODE BEGIN msTickCallback */
    if(red_flag == 1)
    {
      red_counter += 10;
      if(red_counter >= 10000)
      {
        red_counter = 10000;
        red_flag = 0;
        blue_flag = 0;
        gameState = REDWINS;
      }
    }

    if(blue_flag == 1)
    {
      blue_counter += 10;
      if(blue_counter >= 10000)
      { 
        blue_counter = 10000;
        red_flag = 0;
        blue_flag = 0;
        gameState = BLUEWINS;
      }
    }

    if(blinkRED_flag)
    {
        if(++redLED_timer >= 25)
        {
          blinkRED_flag = 0;
          REDLED_OFF;
        }
    }
    if(blinkBLUE_flag)
    {
      if(++blueLED_timer >= 25)
      {
        blinkBLUE_flag = 0;
        BLUELED_OFF;
      }
    }
  /* USER CODE END msTickCallback */
}

/* debounceCallback function */
void debounceCallback(void const * argument)
{
  /* USER CODE BEGIN debounceCallback */
  debouncing_Flag = 0;
  /* USER CODE END debounceCallback */
}

/* blinkTimerCallback function */
void blinkTimerCallback(void const * argument)
{
  /* USER CODE BEGIN blinkTimerCallback */

  /* USER CODE END blinkTimerCallback */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM1 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM1) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
