#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stm32l0xx_hal.h>

#define LED_PORT                GPIOB
#define LED_PIN                 GPIO_PIN_3
#define LED_PORT_CLK_ENABLE     __HAL_RCC_GPIOB_CLK_ENABLE
#define VCP_TX_Pin GPIO_PIN_2
#define VCP_RX_Pin GPIO_PIN_15

UART_HandleTypeDef huart2;
int batteryLevel = 100; // Declare battery level globally

void SystemClock_Config(void);
static void MX_USART2_UART_Init(void);
void osSystickHandler(void);
void initGPIO(void);
void Error_Handler(void);
int _write(int file, uint8_t *ptr, int len);

void decreaseBattery(void)
{
  batteryLevel -= 1;
  if (batteryLevel < 0)
  {
    // Battery is empty, end the program
    HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_RESET); // Turn off the LED
    while (1)
    {
      // Stay in a loop or perform other actions as needed
    }
  }
}

void osSystickHandler(void)
{
  static uint8_t ledState = 0; // 0: LED is off, 1: LED is on

  // Toggle LED every 500ms
  if ((HAL_GetTick() % 500) == 0)
  {
    ledState = !ledState;
    HAL_GPIO_WritePin(LED_PORT, LED_PIN, ledState ? GPIO_PIN_SET : GPIO_PIN_RESET);

    if (ledState == 0)
    {
      // Decrease battery only after one round of LED on and off
      decreaseBattery();
    }
  }

  // Simulate battery drain: decrease battery level every second
  if ((HAL_GetTick() % 1000) == 0)
  {
    batteryLevel -= 1;
    if (batteryLevel < 0)
      batteryLevel = 0;
  }
}

void initGPIO()
{
  GPIO_InitTypeDef GPIO_Config;

  GPIO_Config.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_Config.Pull = GPIO_NOPULL;
  GPIO_Config.Speed = GPIO_SPEED_FREQ_HIGH;

  GPIO_Config.Pin = LED_PIN;

  LED_PORT_CLK_ENABLE();
  HAL_GPIO_Init(LED_PORT, &GPIO_Config);

  __HAL_RCC_GPIOB_CLK_ENABLE();
}

int main(void)
{
  HAL_Init();
  SystemClock_Config();

  initGPIO();
  MX_USART2_UART_Init();

  printf("Hello, %s!\n", "Yi Yang");

  while (1)
  {
    // Print the current battery level
    printf("Battery Level: %d%%\n", batteryLevel);

    // Add a delay to avoid printing too frequently
    HAL_Delay(1000);
  }

  return 0;
}




void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
    in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLLMUL_4;
  RCC_OscInitStruct.PLL.PLLDIV = RCC_PLLDIV_2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART2;
  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}


/**
    @brief USART2 Initialization Function
    @param None
    @retval None
*/
static void MX_USART2_UART_Init(void)
{
  __HAL_RCC_GPIOA_CLK_ENABLE();
  /**USART2 GPIO Configuration
    PA2     ------> USART2_TX
    PA15     ------> USART2_RX
  */
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = VCP_TX_Pin | VCP_RX_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF4_USART2;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }

  __HAL_RCC_USART2_CLK_ENABLE();
}

void Error_Handler(void)
{
  /* User can add his own implementation to report the HAL error return state */
}


// The following makes printf() write to USART2:

#define STDOUT_FILENO   1
#define STDERR_FILENO   2

int _write(int file, uint8_t *ptr, int len)
{
  switch (file)
  {
    case STDOUT_FILENO:
      HAL_UART_Transmit(&huart2, ptr, len, HAL_MAX_DELAY);
      break;

    case STDERR_FILENO:
      HAL_UART_Transmit(&huart2, ptr, len, HAL_MAX_DELAY);
      break;

    default:
      return -1;
  }

  return len;
}
