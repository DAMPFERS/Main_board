#ifndef UART_HANDLER_H
#define UART_HANDLER_H


#define DMA_SxCR_CHSEL_4 (4 << 25)

#include "stm32f4xx_hal.h"
#include "const_valiues.h"
#include "stm32f4xx.h"
#include "stm32f4xx_it.h"

extern volatile uint8_t rx_buffer[RX_BUFFER_SIZE];
extern volatile uint8_t tx_buffer[8];

// Инициализация USART1 с DMA
void USART1_Init(void);
// Включение прерываний
void NVIC_Config(void);
// Отправка данных через DMA
void USART1_SendData(uint8_t* data, uint16_t length);



// void UART_Init(void);
// void MX_DMA_Init(void);
// void Start_UART_Reception(void);
// void UART_SendString(const char *str);

// int UART_DataAvailabel(void);

// HAL_StatusTypeDef UART_ReadLine(char *buffer, int len);

// void UART_SendString(const char *str);

#endif // UART_HANDLER_H