#ifndef COORD_CONVERT_H
#define COORD_CONVERT_H

#include "gcode_parser.h"

// Функция преобразования координат рабочей области в абсолютные координаты стола
void Coord_Convert(GCodeCommand *cmd);

#endif // COORD_CONVERT_H