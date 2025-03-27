#include "stm32f4xx_hal.h"
#include "const_valiues.h"
// #include "stm32f4xx_it.c"

//Модули

#include "uart_handler.h"
#include "gcode_parser.h"
#include "coord_convert.h"
#include "stepper.h"
#include "feeder.h"
#include "endstops.h"



// Функция настройки тактовой частоты (определяется отдельно)
void SystemClock_Config(void); 


// Инициализация GPIO, таймеров, UART и других периферийных модулей
void MX_GPIO_Init();
void MX_USARTx_UART_Init();
void MX_TIMx_Init();


// Прототип функции калибровки
void Calibrate(void);


int main(void){
    HAL_Init();
    SystemClock_Config(); 

    
    MX_GPIO_Init();
    MX_USARTx_UART_Init();
    MX_TIMx_Init();


    UART_Init();
    Stepper_Init();
    Feeder_Init();
    EndStops_Init();

    Calibrate();
    
    while(1){

        // Прием G-кода по UART → Буфер
        if (UART_DataAvailabel()){
            char buffer[UART_BUFFER_SIZE];
            if (UART_ReadLine(buffer, sizeof(buffer)) == HAL_OK){

                // Парсинг G-кода: извлечение G0/G1, X, Y, Z, F
                GCodeCommand cmd;
                if (GCode_Parse(buffer, &cmd) == 0){
                    
                    // Преобразование координат (рабочие -> абсолютные)
                    Coord_Convert(&cmd);

                    // Отправка подтверждения
                    UART_SendString("OK\n");

                    // Добавление команды в очередь движений
                    Stepper_EnqueueCommand(&cmd);


                }
                else
                    UART_SendString("ERR: Parsing\n");
            }
        }

        // Планирование движения: если в очереди есть команды, выполняем интерполяцию и генерацию импульсов
        if (Stepper_HasPendingCommands())
            Stepper_ProcessMotion();
        
        // Управление подачей проволоки (PWM + DIR)    
        Feeder_Update();

        // Проверка концевиков (аварийный стоп)
        if (EndStops_CheckTriggered()){

            Stepper_EmerhencyStop();
            UART_SendString("ERR: Endstop Triggered\n");
        }
    }
}






void SystemClock_Config(void){
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    // 1. Настройка осцилляторов:
    // Выбираем HSE (High Speed External) в качестве источника тактирования.
    // Включаем PLL для получения частоты ядра, например 100 МГц.

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;  // Используем внешний кварц
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;                    // Включаем HSE
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;                // Включаем PLL
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;        // PLL принимает сигнал от HSE
    
    // Конфигурация множителей PLL:
    // Например, если HSE = 8 МГц, чтобы получить 100 МГц ядра,
    // можно задать PLL_M = 8, PLL_N = 200, PLL_P = 2 (200/2 = 100)
    RCC_OscInitStruct.PLL.PLLM = 8;
    RCC_OscInitStruct.PLL.PLLN = 200;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;     // Делитель PLL_P = 2
    RCC_OscInitStruct.PLL.PLLQ = 7;                 // Для USB, если нужно 48 МГц (пример)
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK){
        while (1)
        ;
    }

    // 2. Настройка тактовых делителей для шин:
    // Устанавливаем системный такт (SYSCLK) равным PLL (100 МГц).
    // AHB (High-speed bus) делится без делителя.
    // APB1 и APB2 могут иметь собственные делители (APB1 обычно меньше, так как поддерживает максимум 50 МГц).
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;   // SYSCLK из PLL
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;          // HCLK = SYSCLK (100 МГц)
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;           // PCLK1 = 50 МГц (максимум для некоторых периферийных блоков)
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;           // PCLK2 = 100 МГц

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK){
        // Обработка ошибки
        while(1)
        ;
    }

    return;
}


void MX_GPIO_Init(){
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // Включение тактирования для портов, например, GPIOA и GPIOB
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    // Настройка пинов для вывода (например, для управления шаговыми двигателями)
    // Допустим, пины PA0 и PA1 используются как выходы для STEP/DIR
    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;         // Режим push-pull
    GPIO_InitStruct.Pull = GPIO_NOPULL;             // Без подтяжек (можно использовать GPIO_PULLUP если требуется)
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;   // Высокая скорость переключения
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    // Настройка пина для входа (например, концевик)
    // Допустим, пин PB0 подключен к концевому переключателю
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;         // Режим входа
    GPIO_InitStruct.Pull = GPIO_PULLUP;             // Включаем подтяжку к VCC (если концевик замыкается на землю)
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    // Если пин требуется для альтернативной функции (например, UART), то его настройка будет произведена в MX_USARTx_UART_Init.
    return;
}


void MX_USARTx_UART_Init(){
    UART_HandleTypeDef huart;

    // для USART2
    huart.Instance = USART2;                
    huart.Init.BaudRate = UART_BAUDRATE;            // Скорость передачи
    huart.Init.WordLength = USART_WORDLENGTH_8B;    // 8 бит данных
    huart.Init.StopBits = UART_STOPBITS_1;          // 1 стоп-бит
    huart.Init.Parity = UART_PARITY_NONE;           // Без бита четности
    huart.Init.Mode = UART_MODE_TX_RX;              // Режим работы: прием и передача
    huart.Init.HwFlowCtl = UART_HWCONTROL_NONE;     // Аппаратное управление потоком: отсутствует
    huart.Init.OverSampling = UART_OVERSAMPLING_16; // Параметр оверсемплинга
    if (HAL_UART_Init(&huart) != HAL_OK){
        // Обработка ошибки инициализации
        while(1)
        ;
    }

    return;
}

void MX_TIMx_Init(){
    
    TIM_HandleTypeDef htim;
    TIM_OC_InitTypeDef sConfigOC = {0};

    htim.Instance = TIM3;
    // Период и предделитель зависят от тактовой частоты таймера.
    // Допустим, при тактовой частоте 100 МГц мы хотим получить частоту таймера 100 кГц:
    // Предделитель = 1000-1, период = 100-1 (100 МГц/1000 = 100 кГц, 100 кГц/100 = 1 кГц)
    htim.Init.Prescaler = 1000 - 1;             // Таймер работает от тактовой частоты APB, делённой на это значение плюс один
    htim.Init.CounterMode = TIM_COUNTERMODE_UP; // Режим счета
    htim.Init.Period = 100 - 1;                 // Значение автоперезагрузки, определяющее период таймера)
    htim.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;   //Деление тактовой частоты для внутренней логики таймера.
    htim.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&htim) != HAL_OK){
        // Обработка ошибки
        while(1)
        ;
    }

    // Если используется PWM-режим, его также нужно настроить.
    if(HAL_TIM_PWM_Init(&htim) != HAL_OK){
        // Обработка ошибки
        while(1)
        ;
    }

    // Настройка канала для PWM (например, канал 1)
    sConfigOC.OCMode = TIM_OCMODE_PWM1;             // Режим ШИМ
    sConfigOC.Pulse = 50;                           // Значение сравнения (начальный скважность, 50 из 100)
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;     // Полярность сигнала
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    if (HAL_TIM_PWM_ConfigChannel(&htim, &sConfigOC, TIM_CHANNEL_1) != HAL_OK){
        // Обработка ошибки
        while(1)
        ;
    }

    // Запуск таймера в режиме PWM на выбранном канале
    if (HAL_TIM_PWM_Start(&htim, TIM_CHANNEL_1) != HAL_OK){
        // Обработка ошибки
        while(1)
        ;
    }
    return;
}



void Calibrate(void){
    return;
}