#ifndef __MAIN_HEAD


#define __MAIN_HEAD
#include "stm32f4xx.h"
#include "stm32f4xx_it.h"


#define RX_BUFFER_SIZE 256
#define DMA_SxCR_CHSEL_4 (4 << 25)

void SystemClock_Config(void);
void USART1_Init(void);
void USART1_SendData(uint8_t* data, uint16_t length);
void NVIC_Config(void);


extern volatile uint8_t rx_buffer[RX_BUFFER_SIZE];
extern volatile uint8_t tx_buffer[8];
extern uint32_t a;

#endif
