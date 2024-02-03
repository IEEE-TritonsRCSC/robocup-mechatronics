/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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
#include "adc.h"
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
#define Calibrate 1
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint32_t canTxMailbox;
uint32_t canRxMailbox;
uint8_t CAN_TxData[8];
uint8_t CAN_RxData[8];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

uint16_t ADC_Data=0;
int ADC_Converted = 0;
long map(long x, long in_min, long in_max, long out_min, long out_max)
{
 return (x - in_min) * (out_max - out_min + 1) / (in_max - in_min + 1) + out_min;
}
// Called when buffer is completely filled
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
	ADC_Converted = map(ADC_Data, 0, 4095, 50, 100);

	TIM1->CCR1 = ADC_Converted;
}


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
  MX_TIM1_Init();
  MX_USART2_UART_Init();
  MX_ADC1_Init();
  /* USER CODE BEGIN 2 */
  //UART setup
  unsigned char runMotorHeader = 0x01;
  unsigned char dribblerHeader = 0x02;
  uint32_t uart_rx_buffer_size = 9;  //set to the size we want to limit receive messages to
  uint32_t uart_tx_buffer_size = 33;  //set to the size we want to limit send messages to
  uint8_t uart_rx_buffer[uart_rx_buffer_size]; //buffer that stores in an array of characters user inputs, aka a string
  uint8_t uart_tx_buffer[uart_tx_buffer_size];
  uint8_t headers[] = {runMotorHeader, dribblerHeader};
  int ms_to_listen = 4000;  //set the number of ms we keep the uart line in receive mode for

  //Motor setup
  //HAL_GPIO_TogglePin(Motor_Port, Motor1_Pin);
  //HAL_GPIO_TogglePin(Motor_Port, Motor2_Pin);
  //HAL_GPIO_TogglePin(Motor_Port, Motor3_Pin);
  //HAL_GPIO_TogglePin(Motor_Port, Motor4_Pin);

  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);

#if Calibrate
  TIM1->CCR1 = 100;  // Set the maximum pulse (2ms)
  HAL_Delay (2000);  // wait for 1 beep
  TIM1->CCR1 = 50;   // Set the minimum Pulse (1ms)
  HAL_Delay (1000);  // wait for 2 beeps
  TIM1->CCR1 = 0;    // reset to 0, so it can be controlled via ADC
#endif

  HAL_ADC_Start_DMA(&hadc1, (uint32_t *)&ADC_Data, 1);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

  }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 72;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
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
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV16;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void forward(int motorSpeed, int runDuration){          //speed can be 16 bits, split into high and low bytes
	CAN_TxData[0] = (-motorSpeed) >> 8;  //high byte for speed, shifted 8 because only buffer is only 8 bits
	CAN_TxData[1] = (-motorSpeed);       //low bytes for speed
	CAN_TxData[2] = (motorSpeed) >> 8;
	CAN_TxData[3] = (motorSpeed);
	CAN_TxData[4] = (-motorSpeed) >> 8;
	CAN_TxData[5] = (-motorSpeed);
	CAN_TxData[6] = (motorSpeed) >> 8;
	CAN_TxData[7] = (motorSpeed);

	int i = 0;
	while(i < runDuration){
		//HAL_CAN_AddTxMessage(&hcan1, &canTxHeader, CAN_TxData, &canTxMailbox);
	    HAL_Delay(0.5);
	    i++;
	}
}

void runMotors(unsigned char motorOneHigh, unsigned char motorOneLow, unsigned char motorTwoHigh, unsigned char motorTwoLow, unsigned char motorThreeHigh, unsigned char motorThreeLow, unsigned char motorFourHigh, unsigned char motorFourLow){          //speed can be 16 bits, split into high and low bytes
	CAN_TxData[0] = motorOneHigh;  //high byte for speed, shifted 8 because only buffer is only 8 bits
	CAN_TxData[1] = motorOneLow;       //low bytes for speed
	CAN_TxData[2] = motorTwoHigh;
	CAN_TxData[3] = motorTwoLow;
	CAN_TxData[4] = motorThreeHigh;
	CAN_TxData[5] = motorThreeLow;
	CAN_TxData[6] = motorFourHigh;
	CAN_TxData[7] = motorFourLow;

	int i = 0;
	while(i < 1000){
		//HAL_CAN_AddTxMessage(&hcan1, &canTxHeader, CAN_TxData, &canTxMailbox);
	    HAL_Delay(0.5);
	    i++;
	}
}

void dribble() {

}

void noDribble(){

}

void kick(int kickDuration){

}

void charge(int chargeDuration){

}



/* USER CODE END 4 */

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
