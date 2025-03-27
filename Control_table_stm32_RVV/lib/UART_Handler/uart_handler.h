#ifndef UART_HANDLER_H
#define UART_HANDLER_H

#include "stm32f4xx_hal.h"

void UART_Init(void);

int UART_DataAvailabel(void);

HAL_StatusTypeDef UART_ReadLine(char *buffer, int len);

void UART_SendString(const char *str);

#endif // UART_HANDLER_H