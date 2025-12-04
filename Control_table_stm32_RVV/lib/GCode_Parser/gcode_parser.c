#include "gcode_parser.h"

#include "uart_handler.h"



void Buffer_Fill_TX(const uint8_t* data, uint16_t length){
    if (length >= TX_BUFFER_SIZE) return;

    for(uint16_t i = 0; (i < length); i++)
        tx_buffer[i] = data[i];
    tx_buffer[length];
    
    return;
}


// Функция парсинга строки G-кода. Возвращает 0 при успешном разборе.
unsigned char GCode_Parse(const char *line, GCodeCommand *cmd) {
    // Инициализация значений по умолчанию
    // cmd->x = cmd->y = cmd->z = NAN;
    // cmd->feedrate = cmd->speed = cmd->acceleration = NAN;
    
    const char *p = line;
    
    // Пропуск ведущих пробелов
    while (*p == ' ') p++;
    if (*p == '\0') return 1;

    // Парсинг команды
    if (!isalpha((unsigned char)*p)) return 1;
    char cmd_letter = *p++;
    
    // Обработка числовой части команды
    if (isdigit((unsigned char)*p) || *p == '-' || *p == '.') {
        char *end;
        strtof(p, &end);
        size_t len = end - p;
        if (len > 0) {
            len = (len > 2) ? 2 : len; // Ограничение длины
            snprintf(cmd->command, sizeof(cmd->command), 
                    "%c%.*s", cmd_letter, (int)len, p);
            p = end;
        } else {
            cmd->command[0] = cmd_letter;
            cmd->command[1] = '\0';
        }
    } else {
        cmd->command[0] = cmd_letter;
        cmd->command[1] = '\0';
    }

    // Парсинг параметров
    while (*p != '\0') {
        while (*p == ' ') p++; // Пропуск пробелов
        if (*p == '\0') break;
        
        char param = *p++;
        if (!isalpha((unsigned char)param)) continue;

        char *end;
        float value = strtof(p, &end);
        if (end == p) continue; // Не число
        p = end;

        switch (toupper(param)) {
            case 'X': cmd->x = value; break;
            case 'Y': cmd->y = value; break;
            case 'Z': cmd->z = value; break;
            case 'F': cmd->feedrate = value; break;
            case 'S': cmd->speed = value; break;
            case 'A': cmd->acceleration = value; break;
        }
    }
    return 0;
}