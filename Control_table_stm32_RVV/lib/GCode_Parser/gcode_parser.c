#include "gcode_parser.h"
#include <stdio.h>
#include <string.h>

// Функция парсинга строки G-кода. Возвращает 0 при успешном разборе.
unsigned char GCode_Parse(const char *line, GCodeCommand *cmd){
    
    // Простейшая реализация для команд G0/G1
    char parsed = sscanf(line, "%s X%f Y%f Z%F F%f", cmd->command, &cmd->x, &cmd->y, &cmd->z, &cmd->feedrate);
    if (parsed < 4) return 1; // ошибка парсинга
    return 0;
}