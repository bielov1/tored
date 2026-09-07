#pragma once

#include <cstddef>

enum class CursorDrawType{ None, Filled, Hollow };
class Cursor
{
public:
    Cursor()
	: draw_type{ CursorDrawType::None }
	, line{ 0 }
	, col{ 0 }
    {}

    Cursor(CursorDrawType type, std::size_t l, std::size_t c)
	: draw_type{ type }
	, line{ l }
	, col{ c }
    {}

    void setPosition(std::size_t new_line, std::size_t new_col);
    void setLine(std::size_t new_line);
    void setCol(std::size_t new_col);
	
    void advanceCol();
    void retreatCol();
    void advanceLine();
    void retreatLine();

    void setDrawType(CursorDrawType new_type) { draw_type = new_type; }
    CursorDrawType getDrawType() const { return draw_type; }
    std::size_t getLine() const { return line; }
    std::size_t getCol() const { return col; }
    
private:
    CursorDrawType draw_type;
    std::size_t line;
    std::size_t col;
};
