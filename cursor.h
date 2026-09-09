#pragma once

#include <cstddef>

struct BufferPosition
{
    std::size_t line;
    std::size_t col;
};

struct BufferOffset
{
    std::size_t byte_offset;
};

enum class CursorDrawType{ None, Filled, Hollow };
class Cursor
{
public:    
    Cursor()
	: draw_type{ CursorDrawType::None }
	, pos{ BufferPosition{ 0, 0 } }
	, offset{ BufferOffset{ 0 } }
    {}

    Cursor(CursorDrawType type,
	   BufferPosition bp,
	   BufferOffset bo)
	: draw_type{ type }
	, pos{ bp }
	, offset{ bo }
    {}
    
    void setPosition(std::size_t new_line, std::size_t new_col);
    void setLine(std::size_t new_line);
    void setCol(std::size_t new_col);
    void setOffset(std::size_t new_offset);
    
    void advanceLine();
    void advanceCol();
    void advanceOffset();
    void retreatLine();
    void retreatCol();
    void retreatOffset();

    void setDrawType(CursorDrawType new_type) { draw_type = new_type; }
    CursorDrawType getDrawType() const { return draw_type; }
    std::size_t getLine() const { return pos.line; }
    std::size_t getCol() const { return pos.col; }
    std::size_t getOffset() const { return offset.byte_offset; }
    
private:
    CursorDrawType draw_type;
    BufferPosition pos;
    BufferOffset offset;
};
