#ifndef FEEDER_H
#define FEEDER_H

// Инициализация управления подачей проволоки
void Feeder_Init(void);

// Обновление состояния управления (обновление PWM, установка направления)
void Feeder_Update(void);

#endif // FEEDER_H