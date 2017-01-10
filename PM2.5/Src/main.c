/**
  ******************************************************************************
  * File Name          : main.c
  * Description        : Main program body
  ******************************************************************************
  *
  * COPYRIGHT(c) 2017 STMicroelectronics
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
#include "i2c.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void Error_Handler(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

int fputc(int ch, FILE *f)
{
  HAL_UART_Transmit(&huart1, (uint8_t *)&ch,1, 0xFFFF);
  return ch;
}

unsigned char data1=0, data2=0, data3=0;
unsigned char update = 0;

unsigned char askco2[7] = {0x42, 0x4D, 0xE3, 0x00, 0x00, 0x01, 0x72};
unsigned char co2s[12]={0}, co2state=0;
uint16_t co2=0, lastco2=0;
uint32_t now=0, last=0;

void dealco2(){
	//42 4D 00 08 0A 4D 3C 7F 0A EA 02 9D 
	co2s[co2state++] = data3;
	if(co2state == 1 && co2s[0] != 0x42) co2state = 0; //起始符1
	if(co2state == 2 && co2s[1] != 0x4d) co2state = 0; //起始符1
	if(co2state == 12){
		co2 = (co2s[4]<<8) + co2s[5];
		co2state = 0;	//接收完毕
		if(co2 != lastco2){
			lastco2 = co2;
			update = 1;
		}
	}
}

unsigned char pm[32]={0}, pmstate=0;
uint16_t pm1_0=0, pm2_5=0, pm10=0;

void dealpm(){
	// 42 4D 00 1C 00 09 00 0B 00 0C 00 09 00 0B 00 0C 09 5A 01 E3 00 2B 00 04 00 01 00 00 91 00 02 F3 
	pm[pmstate++] = data2;
	if(pmstate == 1 && pm[0] != 0x42) pmstate = 0; //起始符1
	if(pmstate == 2 && pm[1] != 0x4d) pmstate = 0; //起始符1
	if(pmstate == 32){
		pm1_0 = (pm[10]<<8) + pm[11];
		pm2_5 = (pm[12]<<8) + pm[13];
		pm10 = (pm[14]<<8) + pm[15];
		pmstate = 0;	//接收完毕
		update = 1;
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if(huart->Instance == USART1) {
		HAL_UART_Transmit(huart, &data1, 1, 0);
		HAL_UART_Receive_IT(huart, &data1, 1);
	}else if(huart->Instance == USART2) {
		dealpm();
		HAL_UART_Receive_IT(huart, &data2, 1);
	}else if(huart->Instance == USART3) {
		dealco2();
		HAL_UART_Receive_IT(huart, &data3, 1);
	}
}

__forceinline void set_bit(unsigned char *data, unsigned char bit, unsigned char value){
	if(value == 0) *data &= ~(1<<bit);
	else           *data |=  (1<<bit);
}

// Color Map for aqi
uint32_t Good        = 0x009966;
uint32_t Moderate    = 0xFFDE33;
uint32_t Unhealthy1  = 0xFF9933;
uint32_t Unhealthy2  = 0xCC0033;
uint32_t Unhealthy3  = 0x660099;
uint32_t Hazardous   = 0x7E0023;
uint32_t *colortable[6] = {&Good, &Moderate, &Unhealthy1, &Unhealthy2, &Unhealthy3, &Hazardous}, colorindex=0;
uint8_t colors[15]={0};
uint8_t spaces[10]={0};

void set_color(uint32_t color){
	HAL_SPI_Transmit(&hspi2, spaces, 10, 0xFF);
	
	uint8_t i=0, j=0;
	for(i=0; i<24; i++){
		if( color&&0x01 == 0){
			set_bit(colors+(j/8), j%8, 1);j++;
			set_bit(colors+(j/8), j%8, 1);j++;
			set_bit(colors+(j/8), j%8, 1);j++;
			set_bit(colors+(j/8), j%8, 1);j++;
			set_bit(colors+(j/8), j%8, 0);j++;
		}else{
			set_bit(colors+(j/8), j%8, 1);j++;
			set_bit(colors+(j/8), j%8, 0);j++;
			set_bit(colors+(j/8), j%8, 0);j++;
			set_bit(colors+(j/8), j%8, 0);j++;
			set_bit(colors+(j/8), j%8, 0);j++;
		}
		color = color >> 1;
	}
	HAL_SPI_Transmit(&hspi2, colors, 15, 0xFF);
}

/* USER CODE END 0 */

int main(void)
{

  /* USER CODE BEGIN 1 */
	
  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_USART3_UART_Init();
  MX_TIM3_Init();
  MX_SPI2_Init();

  /* USER CODE BEGIN 2 */
	HAL_UART_Receive_IT(&huart1, &data1, 1);
	HAL_UART_Receive_IT(&huart2, &data2, 1);
	HAL_UART_Receive_IT(&huart3, &data3, 1);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */
		
		if(update == 1){
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
			printf("PM1.0: %d, PM2.5: %d, PM10: %d, CO2: %d\n", pm1_0, pm2_5, pm10, co2);
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
			update = 0;
		}
		
		now = HAL_GetTick();
		if(now - last > 1000){
			HAL_UART_Transmit(&huart3, askco2, 7, 0xFF);
			last = now;
			set_color(*colortable[colorindex++ % 6]);
		}
		
  }
  /* USER CODE END 3 */

}

/** System Clock Configuration
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
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL8;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
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
    Error_Handler();
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

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler */
  /* User can add his own implementation to report the HAL error return state */
  while(1) 
  {
  }
  /* USER CODE END Error_Handler */ 
}

#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */

}

#endif

/**
  * @}
  */ 

/**
  * @}
*/ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
