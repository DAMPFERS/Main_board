
/*
Код измеряет частоту входного сигнала на таймере TIM1 (канал 1, пин PA8) с помощью режима захвата входного сигнала (Input Capture), 
а затем формирует два сигнала ШИМ (PWM) на таймере TIM3 (каналы 1 и 2, пины PA6 и PA7). 
В зависимости от состояния входа на пине PB1, ШИМ сигнал подается либо на PA6, либо на PA7. 
Скважность ШИМ регулируется пропорционально измеренной частоте входного сигнала.
*/


#include "main.h"
#include "stm32f0xx_hal.h"
#include "stm32f0xx_hal_tim.h"
#include "stm32f0xx_hal_gpio.h"
#include "stm32f0xx_hal_rcc.h"


// #include ""


// Глобальные переменные
TIM_HandleTypeDef htim1;  // Input capture
TIM_HandleTypeDef htim3;  // PWM


uint32_t last_capture = 0;
float frequency = 0;
float duty = 0;



// === Настройка тактирования, GPIO и таймеров ===
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
void MX_TIM1_Init(void);
void MX_TIM3_Init(void);
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim);

// void SystemClock_Config(void);
// static void MX_GPIO_Init(void);


int main(void) {
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_TIM1_Init();
    MX_TIM3_Init();

    HAL_TIM_IC_Start_IT(&htim1, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);

    while (1) {
        
        duty = frequency / 200000.0f;
        if (duty > 1.0f) duty = 1.0f;

        uint16_t pwm_val = (uint16_t)(duty * (__HAL_TIM_GET_AUTORELOAD(&htim3)));

        if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1) == GPIO_PIN_SET) {
            __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, pwm_val); // PA6
            __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, 0);       // PA7
        } else {
            __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 0);       // PA6
            __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, pwm_val); // PA7
        }

        HAL_Delay(10);
    }
}





// === Настройка TIM2 (входной сигнал) ===
// === Входной сигнал: TIM1 CH1 на PA8 ===

void MX_TIM1_Init(void) {
    __HAL_RCC_TIM1_CLK_ENABLE();
    htim1.Instance = TIM1;
    htim1.Init.Prescaler = 0;
    htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim1.Init.Period = 0xFFFF;
    htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    HAL_TIM_IC_Init(&htim1);

    TIM_IC_InitTypeDef sConfig = {0};
    sConfig.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
    sConfig.ICSelection = TIM_ICSELECTION_DIRECTTI;
    sConfig.ICPrescaler = TIM_ICPSC_DIV1;
    sConfig.ICFilter = 0;
    HAL_TIM_IC_ConfigChannel(&htim1, &sConfig, TIM_CHANNEL_1);
    HAL_NVIC_SetPriority(TIM1_BRK_UP_TRG_COM_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(TIM1_BRK_UP_TRG_COM_IRQn);
}


// === Обработчик захвата входного сигнала ===
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM1) {
        uint32_t capture = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
        uint32_t delta = (capture >= last_capture) ? (capture - last_capture) : (0xFFFF - last_capture + capture);
        last_capture = capture;

        if (delta != 0) {
            frequency = HAL_RCC_GetPCLK1Freq() / (float)delta;
        }
    }
}


void TIM1_BRK_UP_TRG_COM_IRQHandler(void) {
    HAL_TIM_IRQHandler(&htim1);
}


// === Настройка TIM3 (ШИМ) ===
// === PWM: TIM3 CH1 (PA6), CH2 (PA7) ===
void MX_TIM3_Init(void) {
    __HAL_RCC_TIM3_CLK_ENABLE();
    htim3.Instance = TIM3;
    htim3.Init.Prescaler = 47;  // 48 MHz / (47+1) = 1 MHz
    htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim3.Init.Period = 100;   // 10 kHz PWM
    htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    HAL_TIM_PWM_Init(&htim3);

    TIM_OC_InitTypeDef sConfig = {0};
    sConfig.OCMode = TIM_OCMODE_PWM1;
    sConfig.Pulse = 0;
    sConfig.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfig.OCFastMode = TIM_OCFAST_DISABLE;

    HAL_TIM_PWM_ConfigChannel(&htim3, &sConfig, TIM_CHANNEL_1);
    HAL_TIM_PWM_ConfigChannel(&htim3, &sConfig, TIM_CHANNEL_2);
}



// === GPIO: вход + PA6 и PA7 как AF (ШИМ) ===
// === GPIO ===
static void MX_GPIO_Init(void) {
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // TIM1 CH1 (PA8)
    GPIO_InitStruct.Pin = GPIO_PIN_8;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF2_TIM1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // TIM3 PWM (PA6, PA7)
    GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
    GPIO_InitStruct.Alternate = GPIO_AF1_TIM3;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // Выбор ШИМ (PB1)
    GPIO_InitStruct.Pin = GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}



// === Системный клок: 48 МГц от HSI + PLL ===
void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL12;
    HAL_RCC_OscConfig(&RCC_OscInitStruct);

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                                |RCC_CLOCKTYPE_PCLK1;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1);
}







// void SystemClock_Config(void)
// {
//   RCC_OscInitTypeDef RCC_OscInitStruct = {0};
//   RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

 
//   RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
//   RCC_OscInitStruct.HSIState = RCC_HSI_ON;
//   RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
//   RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
//   if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
//   {
//     Error_Handler();
//   }


//   RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
//                               |RCC_CLOCKTYPE_PCLK1;
//   RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
//   RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
//   RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

//   if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
//   {
//     Error_Handler();
//   }
// }


// static void MX_GPIO_Init(void)
// {
//   GPIO_InitTypeDef GPIO_InitStruct = {0};

//   __HAL_RCC_GPIOA_CLK_ENABLE();


//   HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET);


//   HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);


//   GPIO_InitStruct.Pin = GPIO_PIN_3|GPIO_PIN_4;
//   GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
//   GPIO_InitStruct.Pull = GPIO_NOPULL;
//   GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
//   HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);


// }




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
