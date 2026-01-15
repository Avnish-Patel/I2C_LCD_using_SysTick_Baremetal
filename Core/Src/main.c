/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
                   //task structure

typedef struct{
	uint32_t period;
	uint32_t priority;
	uint32_t last_run;

	void(*task_func)(void);
}task_t;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define  LCD_ADDR  0X27                                //default I2C address
#define  LCD_RS  (1 << 0)                                  //register select(0000 0001)
//#define  LCD_RW  (1 << 1)
#define   LCD_EN (1 << 2)                                 // LOW pulse trigger the LCD to read data(0000 0010)
#define   LCD_BL (1 << 3)                                //  back light on/off(0000 0100)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
volatile uint32_t  SysTick_ms=0;                        // starting from zero
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_I2C1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */


void SysTick_Init(void){
	SysTick->LOAD = 15999;                                //16000-1
	SysTick->VAL  = 0;
	SysTick->CTRL=(1<<2)|(1<<1)|(1<<0);
}
                                                       // delay function
void delay_ms(uint32_t ms)
{
    uint32_t start = SysTick_ms;
    while ((SysTick_ms - start) < ms);
}


                    //SCHEDULER

void scheduler_run(task_t *tasks, uint8_t num_tasks){
	 // priority loop
	for(uint8_t pr=0; pr<3 ; pr++)
	{
		for(uint8_t i =0 ; i<num_tasks ; i++ )
		{
			if(tasks[i].priority == pr)
			{
				if(SysTick_ms - tasks[i].last_run>=tasks[i].period){
					tasks[i].last_run = SysTick_ms;
					tasks[i].task_func();
				}
			}
		}
	}
}

void i2c_write(uint8_t addr , uint8_t data){
	I2C1->CR1 |= I2C_CR1_START;                             //  sets start bit in control register
	while(!(I2C1->SR1 & I2C_SR1_SB));                       //  until start bit is set in status register

	I2C1->DR = addr<<1;                                     //   address is shifted by 1 in data register
	while(!(I2C1->SR1 & I2C_SR1_ADDR ));                     // ack for the sent address
	(void)I2C1->SR2;                                           //  void read to clear ADDR flag

	I2C1->DR = data;                                         //  actual data is placed in data register
	while(!(I2C1-> SR1 & I2C_SR1_BTF));                      //  waits for byte data transfer flag

	I2C1->CR1 |= I2C_CR1_STOP;                               //   sets the stop bit
}


////  send Byte to LCD

void lcd_i2c_send(uint8_t data , uint8_t rs){

	uint8_t high = data & 0XF0;
	uint8_t low = (data << 4) & 0XF0;

	i2c_write(LCD_ADDR , high|LCD_BL|rs);
	i2c_write(LCD_ADDR , high|LCD_BL|rs|LCD_EN);                 //  higher nibble
	i2c_write(LCD_ADDR , high|LCD_BL|rs);

	i2c_write(LCD_ADDR , low|LCD_BL|rs);
	i2c_write(LCD_ADDR , low|LCD_BL|rs|LCD_EN);                   // lower nibble
	i2c_write(LCD_ADDR , low|LCD_BL|rs);

}


                      //  LCD  API  ///////

void  lcd_cmd(uint8_t cmd){
	lcd_i2c_send(cmd,0);
}

void  lcd_data(uint8_t data){
	lcd_i2c_send(data,LCD_RS);
}

void lcd_print(char *str){
	while(*str)
		lcd_data(*str++);
}

                                              //    LCD    INIT   //

void lcd_Init(void){
	delay_ms(50);    // wait for vcc to settle

	lcd_cmd(0X33);     //initialize
	lcd_cmd(0X32);     //set to 4 bit mode
	lcd_cmd(0X28);     // 2 line , 5x8 font
	lcd_cmd(0X0C);     // display on , cursor off
	lcd_cmd(0X06);     // cursor increment (entry mode)
	lcd_cmd(0X01);      //clear display

	delay_ms(2);
}


                                        ///UART////

void uart_print(char *str){
	while(*str){
		while(!(USART1->SR & USART_SR_TXE));
		USART1->DR = *str++;
	}
}


                                         /// GPIO LED/////

void LED_Toggle(void){
	GPIOG->ODR ^= GPIO_PIN_13;
}


                                      //  tasks    ///

 //  LCD Display

void task_lcd(void){
	static uint8_t done =0;
	if(!done){
	        lcd_cmd(0x01);
	        lcd_cmd(0x80);
	        lcd_print("STM32 Scheduler");
	        lcd_cmd(0xC0);
	        lcd_print("I2C LCD Ready");

	        done = 1;
	}

}

  //  UART

void task_uart(void){

	uart_print("System Running  \r\n");
}


/////    LED

void task_led(void){
	LED_Toggle();
}




                                ////    task table //////

task_t tasks[]=
{
		{5000,0,0,task_lcd},
		{10000,0,1,task_uart},
		{2000,0,2,task_led}
};


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
  MX_USART1_UART_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */
  lcd_Init();
  SysTick_Init();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  scheduler_run(tasks,3);

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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 72;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 3;
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
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
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
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOG, GPIO_PIN_13, GPIO_PIN_RESET);

  /*Configure GPIO pin : PG13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

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
#ifdef USE_FULL_ASSERT
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
