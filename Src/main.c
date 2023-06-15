
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2019 STMicroelectronics
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
#include "main.h"
#include "stm32f1xx_hal.h"

/* USER CODE BEGIN Includes */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "Tap.h"
#include "global.h"
#include "BoundaryScan.h"
#include "USART.h"
#include "BSReg_Def.h"
#include "myString.h"

/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/


TapState currentTapState = TEST_LOGIC_RESET;
JTAGInstruction currentIR = DONT_CARE;

extern BSCell bsc1;


char menuBuffer[BUFFER_SIZE] = {0};
char commandBuffer[BUFFER_SIZE] = {0};

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */






/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  *
  * @retval None
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
	volatile uint64_t val = 0, i = 0;
	sprintf( menuBuffer, "\t\tWelcome to JTAG Interface for STM32F103C8T6, \
			\n \t\t\t type 'help' for enquiry \n\n");


  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

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
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */

  switchSwdToJtagMode();
  jtagIoInit();

  /*	Bypass Instruction
  	   * 	Instruction op-code : 9 bit of 1
  	   * 	1. Load the DR of Boundary Scan(BS) and Cortex TAP
  	   * 	   with 1.
  	   * 	2. Send data of 0b1110001 to shift out.
  	   * 	3. Expected data is
  	   *
  	   * 							BS TAP  Cortex TAP
  	   * 	TDI : 	0b1110001		0b		0b
  	   * 			--------------> ------> ----> 	TDO
  	   * 	Expect result from TDO is 0b111000100	(LSB first)
  	   * */
  	  val = jtagBypass(BYPASS_BOTH_TAP, CORTEX_M3_JTAG_INSTRUCTION_LENGTH, 0b1110001, 10);
  	  val = 0;

  	  /*	IDCODE Instruction
  	   * 	Instruction op-code : 0b000011110
  	   *	1. Load the IR of Boundary Scan(BS) with 0b00001
  	   *	   and Cortex TAP with 0b1110
  	   *    2. Shift out the IDCODE from both TAP's DR.
  	   *    3. Expected result is
  	   *        			BS TAP		Cortex TAP
  	   *	TDI : 		0x16410041		0x3ba00477(LSB)
  	   *
  	   *	Expect result from TDO is 0x164100413ba00477
  	   * */
  	  val = jtagReadIdCode(READ_BOTH_IDCODE, CORTEX_M3_JTAG_INSTRUCTION_LENGTH, DUMMY_DATA, 64);
  	  val = 0;

  	  /*	Get IDCODE after reset TAP state to TEST_LOGIC_RESET
  	   * 	Expect result from TDO is 0x164100413ba00477
  	   * */
  	  val = jtagReadIDCodeResetTAP(DUMMY_DATA, 64);
  	  val = 0;

  	  /*	Get IDCODE for BS TAP and bypass Cortex TAP
  	   *	retval need to shift right by 1 bit bcoz of
  	   *	bypass bit for Cortex TAP
  	   *
  	   *	Expect result from TDO is 0xXXXXXXXX16410041
  	   * */
  	  val = jtagReadIdCode(READ_BSC_IDCODE_BYPASS_M3_TAP, CORTEX_M3_JTAG_INSTRUCTION_LENGTH, 0xffffffffffffffff, 64);
  	  val = val>>1;
  	  val = 0;

  	  /*	Bypass BS TAP and get IDCODE for Cortex TAP
  	   *
  	   * 	Expect result from TDO is 0xXXXXXXXX3ba00477
  	   * */
  	  val = jtagReadIdCode(BYPASS_BSC_TAP_READ_M3_IDCODE, CORTEX_M3_JTAG_INSTRUCTION_LENGTH, 0xffffffffffffffff, 64);
  	  val = 0;

  	  /*	SAMPLE Instruction
  	   * 	Sample the Gpio pin while the MCU is running in normal operation
  	   *
  	   * 	Example
  	   * 	When pin pa12 is connected to GND the expected val is 0,
  	   * 	if pin pa12 is connected to 3.3V the expected val is 1
  	   * */
  	  bSCInIt(&bsc1);
  	  bSCPinConfigure(&bsc1, pa12, INPUT_PIN);
  	  val = bSCSampleGpioPin(&bsc1, pa12);
  	  /*	PRELOAD Instruction
  	   * 	Loaded the test pattern that will use for EXTEST later
  	   *
  	   * 	Example
  	   *	Load '1' at the output cell of pb9 (tri-stated).
  	   *	When loading EXTEST instruction, pb9 will output HIGH '1'
  	   *
  	   * */
  	  writePreloadData(&bsc1, pa12, 1);
  	  val = jtagReadBSCPin(&bsc1, pa12.outputCellNum, PRELOAD_DATA);

  	  /*	EXTEST Instruction
  	   * 	Apply the test pattern preloaded from PRELOAD Instruction
  	   * 	to the circuit
  	   *
  	   * 	NOTICE : Make sure the currentIR is not EXTEST for first time
  	   * 			 then update the currentIR to EXTEST
  	   * */
  	 bSCPinConfigure(&bsc1, pa9, OUTPUT_PIN);
  	 bSCExtestGpioPin(&bsc1, pa9, 1);
  	 bSCExtestGpioPin(&bsc1, pa9, 0);
  	 bSCExtestGpioPin(&bsc1, pa9, 1);
  	 bSCExtestGpioPin(&bsc1, pa9, 1);
  	 bSCExtestGpioPin(&bsc1, pa9, 1);
  	 bSCExtestGpioPin(&bsc1, pa9, 0);
  	 bSCExtestGpioPin(&bsc1, pa9, 0);
  	 bSCExtestGpioPin(&bsc1, pa9, 0);
  	 uartTransmitBuffer(uart1, menuBuffer);

  	 char *str = "pa9";
  	 volatile int k = isBSRegValid(str);
	 bSCInIt(&bsc1);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

	  restart: // Label for goto statement
    // Check if there is new data received via UART
    if(usartIsRxRegNotEmpty(uart1)){
      // If data is received, read it into the commandBuffer
      commandBuffer[i] = (uart1)->DR;
      
      // Check if the UART is ready to transmit data
      if(usartIsTxRegEmpty(uart1)){
        // If ready, echo the received data back to the sender
        (uart1)->DR = commandBuffer[i];
      }
      
      // Check if the received data is a backspace character
      if(commandBuffer[i] == '\b'){
        // If it is, "erase" the last character received by decrementing the index
        i--;
        // Then, restart the loop from the beginning
        goto restart;
      }
      
      // Check if the received data is a newline character
      if(commandBuffer[i] == '\n'){
        // If it is, terminate the command string and process the command
        commandBuffer[i] = '\0';
        commandLineOperation(commandBuffer);
        
        // Reset the index to start a new command
        i = 0;
        
        // Then, restart the loop from the beginning
        goto restart;
      }
      
      // If the received data is neither backspace nor newline, increment the index for the next character
      i++;
    }
	  //uartTransmitAndReceive(uart1, commandBuffer);
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

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* USART1 init function */
static void MX_USART1_UART_Init(void)
{

  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/** Configure pins as 
        * Analog 
        * Input 
        * Output
        * EVENT_OUT
        * EXTI
*/
static void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, TCK_Pin|TMS_Pin|TDI_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : TDO_Pin */
  GPIO_InitStruct.Pin = TDO_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(TDO_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : TCK_Pin TMS_Pin TDI_Pin */
  GPIO_InitStruct.Pin = TCK_Pin|TMS_Pin|TDI_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

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
  * @param  file: The file name as string.
  * @param  line: The line in file as a number.
  * @retval None
  */
void _Error_Handler(char *file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1)
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
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
