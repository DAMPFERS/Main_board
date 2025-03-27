#include "stepper.h"
#include "const_valiues.h"



static GCodeCommand motion_queue[MAX_QUEUE_SIZE];
static char queueHead = 0;
static char queueTail = 0;


void Stepper_Init(void){
    // for(char i = 0; i < MAX_QUEUE_SIZE; i++){
    //     motion_queue[i].x = 0;
    //     motion_queue[i].y = 0;
    //     motion_queue[i].x = 0;
    //     motion_queue[i].x = 0;
    //     motion_queue[i].command[0] = '0';
    //     motion_queue[i].command[1] = '-';
    //     motion_queue[i].command[2] = 'o';
    //     motion_queue[i].command[3] = '\0';
    // }
        
    // Инициализация драйверов шаговых двигателей, таймеров и прерываний
    
}


void Stepper_EnqueueCommand(const GCodeCommand *cmd){
    // Добавляем команду в очередь (без проверки переполнения )
    motion_queue[queueTail] = *cmd;
    queueTail = (queueTail + 1) % MAX_QUEUE_SIZE;
}


char Stepper_HasPendingCommands(void){
    // Проверка наличия ожидающих команд
    return (queueHead != queueTail);
}


void Stepper_ProcessMotion(void){
    // Обработка очереди движения (интерполяция, генерация шаговых импульсов)
    // Получаем команду из очереди
    GCodeCommand current_Cmd = motion_queue[queueHead];
    queueHead = (queueHead + 1) %MAX_QUEUE_SIZE;

    // Здесь выполняется расчёт шагов для осей, интерполяция,
    // расчет профилей ускорения/торможения и генерация STEP/DIR импульсов.
    // Пример (заглушка):
    //   - Вычисляем количество шагов по каждой оси
    //   - Настраиваем таймеры для синхронизации шагов
    //   - Запускаем процесс движения

}

// Аварийная остановка шаговых двигателе
void Stepper_EmerhencyStop(void){
    // Отключаем таймеры, сбрасываем очередь и выставляем состояние ошибки
}