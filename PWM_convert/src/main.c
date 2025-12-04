#include "stm32f0xx.h"

int main(void) {
    volatile int t; // volatile для предотвращения оптимизации

    RCC->CR |= RCC_CR_HSION; // Включаем HSI
    while (!(RCC->CR & RCC_CR_HSIRDY)); // Ждем готовности HSI

    // Сбрасываем делитель частоты (HSI 8 MHz -> системная частота 8 MHz)
    RCC->CFGR &= ~RCC_CFGR_HPRE;

    RCC->AHBENR |= RCC_AHBENR_GPIOAEN; // Включаем тактирование порта A
    while (!(RCC->AHBENR & RCC_AHBENR_GPIOAEN)); // Ждем активации

    // Настройка PA3 как выход
    GPIOA->MODER &= ~GPIO_MODER_MODER2; // Очищаем биты
    GPIOA->MODER |= GPIO_MODER_MODER2_0; // Режим: выход (01)

    GPIOA->OTYPER &= ~GPIO_OTYPER_OT_2; // Push-pull (0)
    GPIOA->OSPEEDR &= ~GPIO_OSPEEDER_OSPEEDR2; // Скорость: по умолчанию
    GPIOA->PUPDR &= ~GPIO_PUPDR_PUPDR2; // Без подтяжки (00)

    GPIOA->BSRR = GPIO_BSRR_BS_2; // Устанавливаем HIGH на PA3

    while(1) {
        t = 400000;
        while (t > 0) t--; // Задержка
    }
}