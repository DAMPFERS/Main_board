#include "uart_handler.h"
#include "main.h"


volatile uint8_t rx_buffer[RX_BUFFER_SIZE];
volatile uint8_t tx_buffer[TX_BUFFER_SIZE];

volatile uint16_t dma_head = 0; // Текущая позиция DMA




// Инициализация USART1 с DMA
void USART1_Init(void) {
    // Включение тактирования
    RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN;  // DMA2 для USART1
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;

    // Настройка GPIO (PA9 - TX, PA10 - RX)
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;

    
    GPIOA->MODER |= (2 << 18) | (2 << 20);  // Альтернативные функции для PA9 и PA10
    GPIOA->AFR[1] |= (7 << (4 * (9 - 8))) | (7 << (4 * (10 - 8))); // AF7 для USART1
    // GPIOA->AFR[1] |= (7 << 4) | (7 << 8);   // AF7 для USART1 на PA9 и PA10
    GPIOA->PUPDR |= GPIO_PUPDR_PUPDR10_0; // Pull-up на RX (PA10)000000000000000000

    // Конфигурация USART1
    USART1->BRR = (100000000 + 115200 / 2) / 115200; // Бодрейт 115200
    USART1->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_UE; // Включение передатчика, приемника и USART
    USART1->CR2 &= ~USART_CR2_STOP; // 1 стоп-бит00000000000000
    USART1->CR3 |= USART_CR3_DMAR;  // Включить DMA для приема00000000000000000

    // Настройка DMA для приема (Stream2, Channe4)
    DMA2_Stream2->CR &= ~DMA_SxCR_EN; // Отключаем поток перед настройкой
    while (DMA2_Stream2->CR & DMA_SxCR_EN); // Ждем отключения

    DMA2_Stream2->CR = DMA_SxCR_CHSEL_4 | // Канал 4 для USART1_RX  CHSEL[2:0] = 4 (Channel 4)
                      DMA_SxCR_MINC |     // Инкремент памяти
                      DMA_SxCR_CIRC ;     // Циркулярный режим
                    //   DMA_SxCR_DMEIE |    // Прерывание ошибки Direct Mode
                    //   DMA_SxCR_TEIE;      // Прерывание ошибки передачи
                    //   DMA_SxCR_TCIE |     // Прерывание по завершению передачи

    DMA2_Stream2->PAR = (uint32_t)&USART1->DR;// Периферийный адрес
    DMA2_Stream2->M0AR = (uint32_t)rx_buffer; // Ваш буфер приема
    DMA2_Stream2->NDTR = RX_BUFFER_SIZE;      // Размер буфера
    // DMA2_Stream2->FCR = 0;              // Direct mode (DMDIS = 0)
    DMA2_Stream2->CR |= DMA_SxCR_EN;          // Включаем поток

    // Настройка DMA для передачи
    DMA2_Stream7->CR &= ~DMA_SxCR_EN; // Отключаем поток перед настройкой
    while (DMA2_Stream7->CR & DMA_SxCR_EN); // Ждем отключения
    DMA2_Stream7->CR = DMA_SxCR_CHSEL_4 | // Канал 4 для USART1_TX
                      DMA_SxCR_DIR_0 |   // Направление: память->периферия
                      DMA_SxCR_MINC |    // Инкремент памяти
                      DMA_SxCR_TCIE;     // Прерывание по завершению передачи
    DMA2_Stream7->PAR = (uint32_t)&USART1->DR;
    DMA2_Stream7->M0AR = (uint32_t)tx_buffer; // Ваш буфер передачи
    DMA2_Stream7->NDTR = 0;                   // Изначально 0
    // Пока не включаем, включим при необходимости передачи

    // Включение DMA в USART
    USART1->CR3 |= USART_CR3_DMAT | USART_CR3_DMAR;


    USART1->CR1 |= USART_CR1_IDLEIE; // Прерывание по IDLE
}

// Отправка данных через DMA
void USART1_SendData(uint8_t* data, uint16_t length) {
    while (DMA2_Stream7->CR & DMA_SxCR_EN); // Ждем завершения предыдущей передачи
    
    DMA2_Stream7->M0AR = (uint32_t)data;
    DMA2_Stream7->NDTR = length;
    DMA2_Stream7->CR |= DMA_SxCR_EN; // Включаем поток DMA
    
    USART1->CR1 |= USART_CR1_TE; // Включаем передатчик
}


// Включение прерываний
void NVIC_Config(void) {
    // NVIC_EnableIRQ(DMA2_Stream2_IRQn); // Прерывание приема
    NVIC_EnableIRQ(DMA2_Stream7_IRQn); // Прерывание передачи
    // NVIC_SetPriority(DMA2_Stream2_IRQn, 0);
    NVIC_SetPriority(DMA2_Stream7_IRQn, 0);
    NVIC_EnableIRQ(USART1_IRQn);
    NVIC_SetPriority(USART1_IRQn, 1);


    // Включение глобальных прерываний
    __enable_irq();
}




// UART_HandleTypeDef huart1;
// DMA_HandleTypeDef hdma_usart1_rx;
// DMA_HandleTypeDef hdma_usart1_tx;

// volatile uint8_t rx_buffer[UART_BUFFER_SIZE];







// Данные будут автоматически сохраняться в rx_buffer через DMA.

// Здесь необходимо использовать выбранный UART (например, USART2) и настройки DMA/прерываний по необходимости.


// void UART_Init(void){ // Инициализация GPIO и USART1
//     huart1.Instance = USART1;
//     huart1.Init.BaudRate = UART_BAUDRATE;
//     huart1.Init.WordLength = USART_WORDLENGTH_8B;
//     huart1.Init.StopBits = UART_STOPBITS_1;
//     huart1.Init.Parity = UART_PARITY_NONE;
//     huart1.Init.Mode = UART_MODE_TX_RX;
//     huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
//     huart1.Init.OverSampling = UART_OVERSAMPLING_16;

//     // Настройка пинов PA9 (TX) и PA10 (RX)
//     __HAL_RCC_GPIOA_CLK_ENABLE();
//     GPIO_InitTypeDef GPIO_InitStruct = {0};
//     GPIO_InitStruct.Pin = GPIO_PIN_9 | GPIO_PIN_10;
//     GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
//     GPIO_InitStruct.Pull = GPIO_PULLUP;
//     GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
//     GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
//     HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

//     // Включаем тактирование USART1 и DMA2
//     __HAL_RCC_USART1_CLK_ENABLE();
    

//     // Инициализация USART1
//     HAL_UART_Init(&huart1);

//     return;
// }

// void MX_DMA_Init(void){
//     __HAL_RCC_DMA2_CLK_ENABLE();
//     // Настройка DMA для приема (USART1 -> memory)
//     hdma_usart1_rx.Instance = DMA2_Stream2; // Для USART1_RX
//     hdma_usart1_rx.Init.Channel = DMA_CHANNEL_4;
//     hdma_usart1_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
//     hdma_usart1_rx.Init.PeriphInc = DMA_PINC_DISABLE;
//     hdma_usart1_rx.Init.MemInc = DMA_MINC_ENABLE;
//     hdma_usart1_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
//     hdma_usart1_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
//     hdma_usart1_rx.Init.Mode = DMA_CIRCULAR;
//     hdma_usart1_rx.Init.Priority = DMA_PRIORITY_HIGH;
//     hdma_usart1_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
//     if (HAL_DMA_Init(&hdma_usart1_rx) != HAL_OK) {
//         // Обработка ошибки
//         while (1);
//     }
//     __HAL_LINKDMA(&huart1, hdmarx, hdma_usart1_rx);

//     hdma_usart1_tx.Instance = DMA2_Stream7;          // Поток для USART1 TX
//     hdma_usart1_tx.Init.Channel = DMA_CHANNEL_4;     // Канал 4 (см. документацию)
//     hdma_usart1_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
//     hdma_usart1_tx.Init.PeriphInc = DMA_PINC_DISABLE;
//     hdma_usart1_tx.Init.MemInc = DMA_MINC_ENABLE;
//     hdma_usart1_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
//     hdma_usart1_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
//     hdma_usart1_tx.Init.Mode = DMA_NORMAL;           // Одноразовая передача
//     hdma_usart1_tx.Init.Priority = DMA_PRIORITY_HIGH;
//     hdma_usart1_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
//     if (HAL_DMA_Init(&hdma_usart1_tx) != HAL_OK) {
//         // Обработка ошибки
//         while (1);
//     }
//     __HAL_LINKDMA(&huart1, hdmatx, hdma_usart1_tx);  // Привязка DMA к USART1


//     // Включаем прерывание DMA
//     HAL_NVIC_SetPriority(DMA2_Stream2_IRQn, 0, 0);
//     HAL_NVIC_EnableIRQ(DMA2_Stream2_IRQn);
//     HAL_NVIC_SetPriority(DMA2_Stream7_IRQn, 0, 0);
//     HAL_NVIC_EnableIRQ(DMA2_Stream7_IRQn);
    

//     return;

// }

// void Start_UART_Reception(void){ // Запуск приема данных через DMA
//     HAL_UART_Receive_DMA(&huart1, (uint8_t*)rx_buffer, UART_BUFFER_SIZE);
//     return;
// }

// void UART_SendString(const char *str){ // Отправка данных в монитор порта:
//     // __HAL_UART_CLEAR_FLAG(&huart1, UART_FLAG_TC);
//     HAL_UART_Transmit_DMA(&huart1, (uint8_t*)str, strlen(str));
//     // Или блокирующая версия: 
//     // HAL_UART_Transmit(&huart1, (uint8_t*)str, strlen(str), HAL_MAX_DELAY);
//     return;
// }



// int UART_DataAvailabel(void){

//     // Простейшая реализация (например, проверка флага буфера DMA)
//     return 1;
// }

// HAL_StatusTypeDef UART_ReadLine(char *buffer, int len){
//     // Простейшая реализация чтения строки
//     // Обычно следует использовать DMA и ожидание символа конца строки '\n'
//     // Здесь можно сделать блокирующее чтение для примера.
//     return HAL_OK;
// }

// void UART_SendString(const char *str){
//     // Передача строки по UART (блокирующий вызов для примера)
//     // HAL_UART_Transmit(&huart2, (uint8_t*)str, strlen(str), HAL_MAX_DELAY);
//     return;
// }