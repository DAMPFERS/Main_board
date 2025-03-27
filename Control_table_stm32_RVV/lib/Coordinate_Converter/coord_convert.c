#include "coord_convert.h"
#include "const_valiues.h" // здесь определены смещения, размеры поля и т.п.

void Coord_Convert(GCodeCommand *cmd){

     // Если рабочая область имеет смещение (X_offset, Y_offset)
     cmd->x += (float)X_OFFSET_MM;
     cmd->y += (float)Y_OFFSET_MM;
}