#include "uart_handler.h"
#include "stm32f4xx_hal.h"


// Здесь необходимо использовать выбранный UART (например, USART2) и настройки DMA/прерываний по необходимости.


void UART_Init(void){
    // Инициализация UART через HAL
    return;
}

int UART_DataAvailabel(void){

    // Простейшая реализация (например, проверка флага буфера DMA)
    return 1;
}

HAL_StatusTypeDef UART_ReadLine(char *buffer, int len){
    // Простейшая реализация чтения строки
    // Обычно следует использовать DMA и ожидание символа конца строки '\n'
    // Здесь можно сделать блокирующее чтение для примера.
    return HAL_OK;
}

void UART_SendString(const char *str){
    // Передача строки по UART (блокирующий вызов для примера)
    // HAL_UART_Transmit(&huart2, (uint8_t*)str, strlen(str), HAL_MAX_DELAY);
    return;
}