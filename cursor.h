#pragma once

#include <cstddef>

enum class CursorDrawType{ Filled, Hollow };
class Cursor
{
public:    
    Cursor(CursorDrawType type,
	   std::size_t l,
	   std::size_t c)
	: draw_type{ type }
	, line_idx{ l }
	, col_idx{ c }
    {}

    void setDrawType(CursorDrawType type);
    void setPosition(std::size_t new_line, std::size_t new_col);
    void setLine(std::size_t new_line);
    void setCol(std::size_t new_col);
	
    void advanceCol();
    void retreatCol();
    void advanceLine();
    void retreatLine();

    CursorDrawType getDrawType() const { return draw_type; }
    std::size_t getLine() const { return line_idx; }
    std::size_t getCol() const { return col_idx; }
private:
    CursorDrawType draw_type;
    std::size_t line_idx;
    std::size_t col_idx;
};
