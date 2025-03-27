#ifndef STEPPER_H
#define STEPPER_H

#include "gcode_parser.h"

// Инициализация шаговых двигателей и таймеров
void Stepper_Init(void);

// Добавление команды движения в очередь
void Stepper_EnqueueCommand(const GCodeCommand *cmd);

// Проверка наличия ожидающих команд
char Stepper_HasPendingCommands(void);

// Обработка очереди движения (интерполяция, генерация шаговых импульсов)
void Stepper_ProcessMotion(void);

// Аварийная остановка шаговых двигателе
void Stepper_EmerhencyStop(void);

#endif // STEPPER_H