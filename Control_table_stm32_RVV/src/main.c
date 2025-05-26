#include "main.h"


/*Явное определение обработчика прерываний SysTick (если его нет), используется для работы HAL_Delay(2000)*/
void SysTick_Handler(void) {
  HAL_IncTick(); // Обязательный вызов для обновления uwTick
}


//Модули
#include "uart_handler.h"
#include "gcode_parser.h"
#include "coord_convert.h"
#include "stepper.h"
#include "feeder.h"
#include "endstops.h"





volatile uint8_t uart_rx_byte = 0;
volatile uint8_t uart_data_ready = 0;

TIM_HandleTypeDef htim2_feeder;


// Функция настройки тактовой частоты (определяется отдельно)
void SystemClock_Config(void); 


// Инициализация GPIO, таймеров, UART и других периферийных модулей
void MX_GPIO_Init(void);
void MX_USARTx_UART_Init(void);
void MX_TIMx_Init(void);


// Прототип функции калибровки
void Calibrate(void);



void Feeder_Test(void);

int main(void){
    // uint8_t feeder_speed = 0;
    // uint8_t feeder_dir = 0;


    HAL_Init();
    SystemClock_Config(); 


    USART1_Init();
    NVIC_Config();
    
    // MX_GPIO_Init();
    // MX_USARTx_UART_Init();
    // MX_TIMx_Init();

    
    // UART_Init();
    // MX_DMA_Init();   
    // Start_UART_Reception();

    // Включение глобальных прерываний
    // __enable_irq();

    // Stepper_Init();
    // Feeder_Init(&htim2_feeder);
    // EndStops_Init();
    
    Feeder_Test();

    // Calibrate();
    
    // while(1){

    //     // Прием G-кода по UART → Буфер
    //     if (UART_DataAvailabel()){
    //         char buffer[UART_BUFFER_SIZE];
    //         if (UART_ReadLine(buffer, sizeof(buffer)) == HAL_OK){

    //             // Парсинг G-кода: извлечение G0/G1, X, Y, Z, F
    //             GCodeCommand cmd;
    //             if (GCode_Parse(buffer, &cmd) == 0){
                    
    //                 // Преобразование координат (рабочие -> абсолютные)
    //                 Coord_Convert(&cmd);

    //                 // Отправка подтверждения
    //                 UART_SendString("OK\n");

    //                 // Добавление команды в очередь движений
    //                 Stepper_EnqueueCommand(&cmd);


    //             }
    //             else
    //                 UART_SendString("ERR: Parsing\n");
    //         }
    //     }

    //     // Планирование движения: если в очереди есть команды, выполняем интерполяцию и генерацию импульсов
    //     if (Stepper_HasPendingCommands())
    //         Stepper_ProcessMotion();
        
    //     // Управление подачей проволоки (PWM + DIR)    
    //     Feeder_Update(feeder_speed, feeder_dir, &htim2_feeder);

    //     // Проверка концевиков (аварийный стоп)
    //     if (EndStops_CheckTriggered()){

    //         Stepper_EmerhencyStop();
    //         UART_SendString("ERR: Endstop Triggered\n");
    //     }
    // }
}






void SystemClock_Config(void){
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    // 1. Настройка осцилляторов:
    // Выбираем HSE (High Speed External) в качестве источника тактирования.
    // Включаем PLL для получения частоты ядра, например 100 МГц.


    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;  // Используем внешний кварц
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;                    // Включаем HSE
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;                // Включаем PLL
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;        // PLL принимает сигнал от HSE
    
    // Конфигурация множителей PLL:
    // Например, если HSE = 25 МГц, чтобы получить 100 МГц ядра,
    // можно задать PLL_M = 25, PLL_N = 200, PLL_P = 2 (200/2 = 100)
    RCC_OscInitStruct.PLL.PLLM = 25;
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
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;           // PCLK2 = 100 МГц

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK){
        // Обработка ошибки
        while(1)
        ;
    }

    return;
}


void MX_GPIO_Init(void){
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


void MX_USARTx_UART_Init(void){
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

void MX_TIMx_Init(void){



    
    
    // TIM_HandleTypeDef htim;
    // TIM_OC_InitTypeDef sConfigOC = {0};

    // htim.Instance = TIM3;
    // // Период и предделитель зависят от тактовой частоты таймера.
    // // Допустим, при тактовой частоте 100 МГц мы хотим получить частоту таймера 100 кГц:
    // // Предделитель = 1000-1, период = 100-1 (100 МГц/1000 = 100 кГц, 100 кГц/100 = 1 кГц)
    // htim.Init.Prescaler = 1000 - 1;             // Таймер работает от тактовой частоты APB, делённой на это значение плюс один
    // htim.Init.CounterMode = TIM_COUNTERMODE_UP; // Режим счета
    // htim.Init.Period = 100 - 1;                 // Значение автоперезагрузки, определяющее период таймера)
    // htim.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;   //Деление тактовой частоты для внутренней логики таймера.
    // htim.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    // if (HAL_TIM_Base_Init(&htim) != HAL_OK){
    //     // Обработка ошибки
    //     while(1)
    //     ;
    // }

    // // Если используется PWM-режим, его также нужно настроить.
    // if(HAL_TIM_PWM_Init(&htim) != HAL_OK){
    //     // Обработка ошибки
    //     while(1)
    //     ;
    // }

    // Настройка канала для PWM (например, канал 1)
    // sConfigOC.OCMode = TIM_OCMODE_PWM1;             // Режим ШИМ
    // sConfigOC.Pulse = 50;                           // Значение сравнения (начальный скважность, 50 из 100)
    // sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;     // Полярность сигнала
    // sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    // if (HAL_TIM_PWM_ConfigChannel(&htim, &sConfigOC, TIM_CHANNEL_1) != HAL_OK){
    //     // Обработка ошибки
    //     while(1)
    //     ;
    // }

    // // Запуск таймера в режиме PWM на выбранном канале
    // if (HAL_TIM_PWM_Start(&htim, TIM_CHANNEL_1) != HAL_OK){
    //     // Обработка ошибки
    //     while(1)
    //     ;
    // }
    return;
}



void Calibrate(void){
    return;
}



void Feeder_Test(void){
    // uint16_t duty = 0;
    // int8_t dir = 1;



    GPIO_InitTypeDef GPIO_InitStruct = {0};
    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitStruct = (GPIO_InitTypeDef){0}; // Обнуление структуры
    GPIO_InitStruct.Pin = FEEDER_DIR_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
    HAL_GPIO_Init(FEEDER_DIR_PORT, &GPIO_InitStruct);
    
    while(1){
        HAL_GPIO_WritePin(FEEDER_DIR_PORT, FEEDER_DIR_PIN, GPIO_PIN_SET);
        HAL_Delay(1000);
        HAL_GPIO_WritePin(FEEDER_DIR_PORT, FEEDER_DIR_PIN, GPIO_PIN_RESET);
        HAL_Delay(1000);
        // USART1_SendData(tx_buffer, sizeof(tx_buffer) - 1);
    }
    // while(1){
    //     duty += dir;

    //     if (dir == 1)
    //         HAL_GPIO_WritePin(FEEDER_DIR_PORT, FEEDER_DIR_PIN, GPIO_PIN_SET);
    //     else 
    //         HAL_GPIO_WritePin(FEEDER_DIR_PORT, FEEDER_DIR_PIN, GPIO_PIN_RESET);

    //     if (duty >= htim2_feeder.Init.Period) dir = -1;
    //     else if (duty == 0) dir = 1;

    //     __HAL_TIM_SET_COMPARE(&htim2_feeder, TIM_CHANNEL_1, duty);
    //     HAL_Delay(50);
    // }
}



/*

#include "stm32f4xx_hal.h"

#define LED_PIN        GPIO_PIN_13
#define LED_PORT       GPIOC

void SystemClock_Config(void);
void Error_Handler(void);

int main(void)
{
    HAL_Init();                   // Инициализация HAL-библиотеки
    SystemClock_Config();         // Настройка системного тактового генератора
    
    // Включаем тактирование порта GPIOA
    __HAL_RCC_GPIOC_CLK_ENABLE();
    
    // Настройка пина PA5 в режим push-pull без подтяжки
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = LED_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_PORT, &GPIO_InitStruct);
    
    while (1)
    {
        HAL_GPIO_TogglePin(LED_PORT, LED_PIN); // Переключаем состояние светодиода
        HAL_Delay(500);                         // Задержка 500 мс (SysTick)
    }
}

void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /* Включаем тактирование модуля управления питанием и настраиваем регулятор напряжения.
       Это необходимо для работы PLL на более высокой частоте, даже если система будет работать на 84 МГц. //
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

    /* Настройка источника тактирования:
       Используем HSI (16 МГц) с PLL.
       Вместо деления на 16 (PLLM=16, что даёт 1 МГц на вход PLL) используем PLLM=8 – получаем 2 МГц,
       что зачастую предпочтительнее для стабильной работы PLL.
       Далее PLLN=168 и PLLP=DIV4 дают SYSCLK = (16/8)*168/4 = 84 МГц. 
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLM = 8;      // Повышаем частоту входного сигнала PLL до 2 МГц
    RCC_OscInitStruct.PLL.PLLN = 168;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4; // SYSCLK = 84 МГц
    RCC_OscInitStruct.PLL.PLLQ = 7;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }
    
    /* Настройка делителей для шин AHB, APB1 и APB2 
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK |
                                  RCC_CLOCKTYPE_PCLK1  | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK; // Выбираем PLL как источник SYSCLK
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;         // Максимум 42 МГц для APB1
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;         // До 84 МГц для APB2

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
    {
        // Error_Handler();
    }
}

void Error_Handler(void)
{
    // Если происходит ошибка конфигурации, застреваем здесь.
    while (1)
    {
    }
}

*/



/*
#include "stm32f4xx_hal.h"


#define LED_PIN GPIO_PIN_13
#define LED_PORT GPIOC

void SystemClock_Config(void);
static void MX_GPIO_Init(void);


void SysTick_Handler(void) {
  HAL_IncTick(); // Обязательный вызов для обновления uwTick
}

void Error_Handler(void) {
  while(1) {}
}


int main(void) {
  HAL_Init();
  SystemClock_Config(); 
  MX_GPIO_Init();


  while (1) {
    // My_Delay(4294967295); // Задержка
    HAL_Delay(2000);
    HAL_GPIO_TogglePin(LED_PORT, LED_PIN);
  }
}


void SystemClock_Config(void) {
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;     // HSI = 16 МГц
  RCC_OscInitStruct.PLL.PLLN = 200;    // 16 МГц * 200 = 3200 МГц
  RCC_OscInitStruct.PLL.PLLP = 2;      // SYSCLK = 3200 МГц / 2 = 100 МГц
  RCC_OscInitStruct.PLL.PLLQ = 4;      // Для USB/SDIO
//   RCC_OscInitStruct.PLL.PLLR = 2;      // Не используется в F411
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                              | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK) {
    Error_Handler();
  }
}


static void MX_GPIO_Init(void) {
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  // Включение тактирования порта C
  __HAL_RCC_GPIOC_CLK_ENABLE();

  // Настройка пина PC13
  GPIO_InitStruct.Pin = LED_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;  // Режим push-pull
  GPIO_InitStruct.Pull = GPIO_NOPULL;          // Без подтяжки
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW; // Низкая скорость
  HAL_GPIO_Init(LED_PORT, &GPIO_InitStruct);

  // Изначально выключаем светодиод
  HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_RESET);
}
*/
