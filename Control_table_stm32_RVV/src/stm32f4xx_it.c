#include "stm32f4xx_hal.h"
#include "main.h"
#include "uart_handler.h"
#include "gcode_parser.h"




// Обработчик прерывания DMA (для передачи)
void DMA2_Stream7_IRQHandler(void) {
    if (DMA2->HISR & DMA_HISR_TCIF7) {
        DMA2->HIFCR = DMA_HIFCR_CTCIF7; // Сбрасываем флаг прерывания
        DMA2_Stream7->CR &= ~DMA_SxCR_EN; // Отключаем поток DMA
        // Здесь можно добавить обработку завершения передачи
    }
}

void USART1_IRQHandler(void) {
    if (USART1->SR & USART_SR_IDLE) {
        volatile uint32_t tmp = USART1->SR;
        tmp = USART1->DR; // Сброс IDLE-флага чтением DR
        (void)tmp;


        uint16_t dma_tail = RX_BUFFER_SIZE - DMA2_Stream2->NDTR;
        uint16_t bytes_received = (dma_tail >= dma_head)
            ? (dma_tail - dma_head)
            : (RX_BUFFER_SIZE - dma_head + dma_tail);

        // Обрабатываем данные от dma_head до dma_tail
        char cmd_buf[64]; // Временный буфер для команды
        uint16_t idx = 0;

        for (uint16_t i = 0; i < bytes_received; i++) {
            char c = rx_buffer[(dma_head + i) % RX_BUFFER_SIZE];

            if (c == '\n' || c == '\r') {
                if (idx > 0) {
                    cmd_buf[idx] = '\0';
                    // if (GCode_Parse(cmd_buf, &cmd) == 0) {
                    
                    // }
                    idx = 0;
                }
            } else if (idx < sizeof(cmd_buf) - 1)
                cmd_buf[idx++] = c;
        }
        dma_head = dma_tail; // Обновляем позицию


        // Подсчет количества принятых байт
        // uint16_t received = RX_BUFFER_SIZE - DMA2_Stream2->NDTR;
        // USART1_SendData(tx_buffer, sizeof(tx_buffer) - 1);
        // USART1_SendData(tx_buffer, strlen(tx_buffer));

        // --- здесь можно обработать rx_buffer[0..received-1] ---

        // Если используешь кольцевой буфер — сбрасывать NDTR не нужно,
        // просто сбросить счётчик вручную или сохранять позицию

        // Например: обнуляем DMA, если обрабатываем весь буфер
        // DMA2_Stream2->CR &= ~DMA_SxCR_EN;
        // while (DMA2_Stream2->CR & DMA_SxCR_EN);
        // DMA2_Stream2->NDTR = RX_BUFFER_SIZE;
        // DMA2_Stream2->CR |= DMA_SxCR_EN;
    }
}