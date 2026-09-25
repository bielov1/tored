#include "cursor.h"

void Cursor::setOffset(std::size_t new_offset)
{
    offset.byte_offset = new_offset;
}

void Cursor::advanceOffset()
{
    ++offset.byte_offset;
}

void Cursor::retreatOffset()
{
    --offset.byte_offset;
}
