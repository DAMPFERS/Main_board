#include "feeder.h"
#include "const_valiues.h"


void Feeder_Init(TIM_HandleTypeDef *htim2_feeder){
    // Инициализация управления подачей проволоки

    /*-- Настройка GPIO для PWM --*/
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    __HAL_RCC_GPIOA_CLK_ENABLE();

    // ШИМ выход (TIM2_CH1)
    // GPIO_InitStruct.Pin = FEEDER_PWM_PIN;
    // GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    // GPIO_InitStruct.Pull = GPIO_NOPULL;
    // GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
    // GPIO_InitStruct.Alternate = GPIO_AF1_TIM2;
    // HAL_GPIO_Init(FEEDER_PWM_PORT, &GPIO_InitStruct);

    // Обычный выход DIR
    GPIO_InitStruct = (GPIO_InitTypeDef){0}; // Обнуление структуры
    GPIO_InitStruct.Pin = FEEDER_DIR_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
    HAL_GPIO_Init(FEEDER_DIR_PORT, &GPIO_InitStruct);
    
    /*-- Настройка таймера TIM2 --*/
    // __HAL_RCC_TIM2_CLK_ENABLE();
    // htim2_feeder->Instance = TIM2;
    // htim2_feeder->Init.Prescaler = 1000 - 1; // Тактовая частота 100MHz / 1000 = 100kHz
    // htim2_feeder->Init.CounterMode = TIM_COUNTERMODE_UP;
    // htim2_feeder->Init.Period = 100 - 1;     // Период: 100 тактов → 1kHz PWM
    // htim2_feeder->Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    // HAL_TIM_PWM_Init(htim2_feeder);

    // /*-- Настройка канала PWM --*/
    // TIM_OC_InitTypeDef sConfigOC = {0};
    // sConfigOC.OCMode = TIM_OCMODE_PWM1;
    // sConfigOC.Pulse = 0;
    // sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    // sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    // HAL_TIM_PWM_ConfigChannel(htim2_feeder, &sConfigOC, TIM_CHANNEL_1);

    // HAL_TIM_PWM_Start(htim2_feeder, TIM_CHANNEL_1);

    return;
}


void Feeder_Update(uint8_t speed, uint8_t dir, TIM_HandleTypeDef *htim2_feeder){
    // Обновление состояния управления (обновление PWM, установка направления)
    // Пример: обновляем PWM-сигнал в зависимости от текущей команды движения
    // или можем получать команду из отдельной очереди.
    return;
}