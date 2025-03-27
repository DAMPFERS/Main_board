#ifndef ENDSTOPS_H
#define ENDSTOPS_H


// Инициализация концевиков
void EndStops_Init(void);

// Проверка срабатывания концевиков
char EndStops_CheckTriggered(void);

#endif // ENDSTOPS_H