#ifndef __STM32_IT
#define __STM32_IT


// Обработчик прерывания DMA (для приема)
void DMA2_Stream5_IRQHandler(void);
// Обработчик прерывания DMA (для передачи)
void DMA2_Stream7_IRQHandler(void);

void USART1_IRQHandler(void);


#endif