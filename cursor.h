#pragma once

#include <cstddef>

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
	, offset{ BufferOffset{ 0 } }
    {}

    Cursor(CursorDrawType type,
	   BufferOffset bo)
	: draw_type{ type }
	, offset{ bo }
    {}
    
    void setPosition(std::size_t new_line, std::size_t new_col);
    void setOffset(std::size_t new_offset);

    void advanceOffset();
    void retreatOffset();
    
    void setDrawType(CursorDrawType new_type) { draw_type = new_type; }
    CursorDrawType getDrawType() const { return draw_type; }
    std::size_t getOffset() const { return offset.byte_offset; }

    
private:
    CursorDrawType draw_type;
    BufferOffset offset;
};
