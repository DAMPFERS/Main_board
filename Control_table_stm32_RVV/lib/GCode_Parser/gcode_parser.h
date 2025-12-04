#ifndef GCODE_PARSER_H
#define GCODE_PARSER_H
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

typedef struct{
    char command[4]; // Например, "G0" или "G1"
    float x;
    float y;
    float z;
    float feedrate;
    float speed;
    float acceleration;
} GCodeCommand;

// Функция парсинга строки G-кода. Возвращает 0 при успешном разборе.
unsigned char GCode_Parse(const char *line, GCodeCommand *cmd);

void Buffer_Fill_TX(const uint8_t* data, uint16_t length);


#endif  // GCODE_PARSER_H