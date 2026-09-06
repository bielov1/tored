#include "cursor.h"

void Cursor::setDrawType(CursorDrawType new_draw_type)
{
    draw_type = new_draw_type;
}

void Cursor::setPosition(std::size_t new_line, std::size_t new_col)
{
    line_idx = new_line;
    col_idx  = new_col;
}
void Cursor::setLine(std::size_t new_line)
{
    line_idx = new_line;
}

void Cursor::setCol(std::size_t new_col)
{
    col_idx = new_col;
}

void Cursor::advanceCol()
{
    ++col_idx;
}

void Cursor::retreatCol()
{
    --col_idx;
}

void Cursor::advanceLine()
{
    ++line_idx;
}

void Cursor::retreatLine()
{
    --line_idx;
}
