#include "cursor.h"

void Cursor::setPosition(std::size_t new_line, std::size_t new_col)
{
    pos.line = new_line;
    pos.col  = new_col;
}

void Cursor::setLine(std::size_t new_line)
{
    pos.line = new_line;
}

void Cursor::setCol(std::size_t new_col)
{
    pos.col = new_col;
}

void Cursor::setOffset(std::size_t new_offset)
{
    offset.byte_offset = new_offset;
}

void Cursor::advanceLine()
{
    ++pos.line;
}

void Cursor::advanceCol()
{
    ++pos.col;
}

void Cursor::advanceOffset()
{
    ++offset.byte_offset;
}

void Cursor::retreatLine()
{
    --pos.line;
}

void Cursor::retreatCol()
{
    --pos.col;
}

void Cursor::retreatOffset()
{
    --offset.byte_offset;
}
