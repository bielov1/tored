#pragma once

#include <cstddef>

class Cursor
{
public:

    void setPosition(std::size_t new_line, std::size_t new_col);
    void advanceCol();
    void retreatCol();
    void advanceLine();
    void retreatLine();
        
    std::size_t getLine() const { return line_idx; }
    std::size_t getCol() const { return col_idx; }
private:
    std::size_t line_idx = 0;
    std::size_t col_idx  = 0;
};
