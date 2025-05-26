#ifndef GCODE_PARSER_H
#define GCODE_PARSER_H

typedef struct{
    char command[4]; // Например, "G0" или "G1"
    float x;
    float y;
    float z;
    float feedrate;
} GCodeCommand;

// Функция парсинга строки G-кода. Возвращает 0 при успешном разборе.
unsigned char GCode_Parse(const char *line, GCodeCommand *cmd);


#endif  // GCODE_PARSER_H