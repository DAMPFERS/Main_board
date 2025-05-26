#include "main.h"


// Обработчик прерывания DMA (для передачи)
void DMA2_Stream7_IRQHandler(void) {
    if (DMA2->HISR & DMA_HISR_TCIF7) {
        DMA2->HIFCR = DMA_HIFCR_CTCIF7; // Сбрасываем флаг прерывания
        DMA2_Stream7->CR &= ~DMA_SxCR_EN; // Отключаем поток DMA
        // Здесь можно добавить обработку завершения передачи
    }
}

// Обработчик прерывания DMA (для приема)
void DMA2_Stream2_IRQHandler(void) {
    if (DMA2->LISR & DMA_LISR_TCIF2) {
        DMA2->LIFCR = DMA_LIFCR_CTCIF2; // Сбрасываем флаг прерывания
        // Здесь можно добавить обработку полученных данных
        a++;
        // USART1_SendData(tx_buffer, sizeof(tx_buffer) - 1);

        // Очищаем ошибки USART
        volatile uint32_t sr = USART1->SR;
        volatile uint32_t dr = USART1->DR;
        (void)sr; (void)dr;


        // Вычисляем количество принятых данных
        uint16_t received_bytes = RX_BUFFER_SIZE - DMA2_Stream2->NDTR;

        if (received_bytes > 0) {
            USART1_SendData(tx_buffer, sizeof(tx_buffer) - 1);
        }

        // DMA2_Stream2->NDTR = RX_BUFFER_SIZE;
        // DMA2_Stream2->CR |= DMA_SxCR_EN;
    }
}




void USART1_IRQHandler(void) {
    if (USART1->SR & USART_SR_IDLE) {
        volatile uint32_t tmp = USART1->SR;
        tmp = USART1->DR; // Сброс IDLE-флага чтением DR
        (void)tmp;

        // Подсчет количества принятых байт
        uint16_t received = RX_BUFFER_SIZE - DMA2_Stream2->NDTR;
        USART1_SendData(tx_buffer, sizeof(tx_buffer) - 1);

        // --- здесь можно обработать rx_buffer[0..received-1] ---

        // Если используешь кольцевой буфер — сбрасывать NDTR не нужно,
        // просто сбросить счётчик вручную или сохранять позицию

        // Например: обнуляем DMA, если обрабатываем весь буфер
        DMA2_Stream2->CR &= ~DMA_SxCR_EN;
        while (DMA2_Stream2->CR & DMA_SxCR_EN);
        DMA2_Stream2->NDTR = RX_BUFFER_SIZE;
        DMA2_Stream2->CR |= DMA_SxCR_EN;
    }
}
