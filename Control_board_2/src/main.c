// #include "stm32f4xx.h"

// volatile uint32_t tick = 0;
// void SystemClock_HSI_Init(void);

// void SysTick_Handler(void) { tick++; }

// void Custom_Delay(uint32_t ms) {
//     uint32_t start = tick;
//     while (tick - start < ms);
// }

// int main(void) {
//     // 1. Настройка тактовой частоты
//     SystemClock_HSI_Init(); // Из примера выше
    
//     // 2. Инициализация SysTick
//     SysTick_Config(SystemCoreClock / 1000);
    
//     // 3. Настройка GPIO
//     RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
//     GPIOA->MODER |= GPIO_MODER_MODE5_0;
    
//     while(1) {
//         GPIOA->ODR ^= GPIO_ODR_OD5;
//         Custom_Delay(500);
//     }
// }


// void SystemClock_HSI_Init(void) {
//     FLASH->ACR = FLASH_ACR_LATENCY_2WS; // Задержка FLASH
//     RCC->CR |= RCC_CR_HSION; // Включить HSI
//     while(!(RCC->CR & RCC_CR_HSIRDY)); // Ждем стабилизации
  
//     RCC->CFGR = RCC_CFGR_SW_HSI; // Источник SYSCLK = HSI
//     RCC->PLLCFGR = 0; // Отключить PLL
//     RCC->CFGR |= RCC_CFGR_HPRE_DIV1; // AHB = 16 МГц
//     SystemCoreClock = 16000000; // Явное задание частоты
// }






#include "main.h"




volatile uint8_t rx_buffer[RX_BUFFER_SIZE];
volatile uint8_t tx_buffer[] = "HULLO\r\n";

uint32_t a = 0;

// Инициализация тактирования (CMSIS версия)
void SystemClock_Config(void) {
    // Включение HSE (внешний кварц 25 МГц)
    RCC->CR |= RCC_CR_HSEON;
    while (!(RCC->CR & RCC_CR_HSERDY)); // Ждем стабилизации HSE

    // Настройка PLL для получения 100 МГц
    // PLL_M = 25, PLL_N = 200, PLL_P = 2 (200/2 = 100 МГц)
    RCC->PLLCFGR = (25 << 0) |  // PLL_M
                  (200 << 6) |  // PLL_N
                  (0 << 16) |   // PLL_P = /2 (00)
                  (RCC_PLLCFGR_PLLSRC_HSE); // Источник PLL - HSE

    // Включение PLL
    RCC->CR |= RCC_CR_PLLON;
    while (!(RCC->CR & RCC_CR_PLLRDY)); // Ждем стабилизации PLL

    // Настройка делителей
    FLASH->ACR = FLASH_ACR_PRFTEN | FLASH_ACR_ICEN | FLASH_ACR_DCEN | FLASH_ACR_LATENCY_3WS;

    // Переключение на PLL
    RCC->CFGR |= RCC_CFGR_SW_PLL;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL); // Ждем переключения

    // Настройка делителей шин
    RCC->CFGR |= RCC_CFGR_HPRE_DIV1  |  // HCLK = SYSCLK (100 МГц)
                 RCC_CFGR_PPRE1_DIV2 |  // APB1 = 50 МГц
                 RCC_CFGR_PPRE2_DIV1;   // APB2 = 100 МГц
}

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
    DMA2_Stream2->NDTR = RX_BUFFER_SIZE;      // Размер буфера\
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

int main(void) {
    SystemClock_Config();
    
    USART1_Init();
    NVIC_Config();
    
    
    while (1) {
        // USART1_SendData(tx_buffer, sizeof(tx_buffer) - 1);
        // Основной цикл
    }
}