#ifndef FEEDER_H
#define FEEDER_H

#include "stm32f4xx_hal.h"
#include <stdint.h>

// Инициализация управления подачей проволоки
void Feeder_Init(TIM_HandleTypeDef *htim2_feeder);

// Обновление состояния управления (обновление PWM, установка направления)
void Feeder_Update(uint8_t speed, uint8_t dir, TIM_HandleTypeDef *htim2_feeder);

#endif // FEEDER_H