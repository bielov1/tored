#include "cursor.h"

void Cursor::setPosition(std::size_t new_line, std::size_t new_col)
{
    line = new_line;
    col  = new_col;
}

void Cursor::setLine(std::size_t new_line)
{
    line = new_line;
}

void Cursor::setCol(std::size_t new_col)
{
    col = new_col;
}

void Cursor::advanceCol()
{
    ++col;
}

void Cursor::retreatCol()
{
    --col;
}

void Cursor::advanceLine()
{
    ++line;
}

void Cursor::retreatLine()
{
    --line;
}
