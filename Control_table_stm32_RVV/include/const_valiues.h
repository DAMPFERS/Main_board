#pragma once

// Конфигурация оборудования
#define STEP_PER_REV          200    // Шагов на оборот
#define MICROSTEPS            16     // Делитель шага
#define MM_PER_REV_X          2.0    // мм на оборот по X
#define MM_PER_REV_Y          2.0    // мм на оборот по Y    
#define MM_PER_REV_Z          1.0    // мм на оборот по Z

// Расчет шагов на мм
#define STEPS_PER_MM_X        (STEP_PER_REV * MICROSTEPS / MM_PER_REV_X)
#define STEPS_PER_MM_Y        (STEP_PER_REV * MICROSTEPS / MM_PER_REV_Y)
#define STEPS_PER_MM_Z        (STEP_PER_REV * MICROSTEPS / MM_PER_REV_Z)

// Простейшие структуры очереди команд (можно заменить на более сложную реализацию)
#define MAX_QUEUE_SIZE 10

// Смещение рабочей области (абсолютные координаты)
#define X_OFFSET_MM          500     // мм
#define Y_OFFSET_MM          300
#define Z_OFFSET_MM          0

// Концевики
#define ENDSTOP_X_PIN        GPIO_PIN_13
#define ENDSTOP_X_PORT       GPIOC
#define ENDSTOP_Y_PIN        GPIO_PIN_12
#define ENDSTOP_Y_PORT       GPIOC
#define ENDSTOP_Z_PIN        GPIO_PIN_11
#define ENDSTOP_Z_PORT       GPIOC

#define FEEDER_PWM_PIN        GPIO_PIN_5
#define FEEDER_PWM_PORT       GPIOA
#define FEEDER_DIR_PIN        GPIO_PIN_6
#define FEEDER_DIR_PORT       GPIOA


// UART
#define UART_BAUDRATE           115200
#define RX_BUFFER_SIZE          256
#define GCODE_QUEUE_SIZE        16