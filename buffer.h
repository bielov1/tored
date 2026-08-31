#pragma once

#include <vector>
#include <string>

using Line = std::pair<std::size_t, std::string>;
using Text = std::vector<Line>;

class Buffer
{
public:
    Buffer()
         : text{}
    {}

    void insertLine(std::string line);
    void removeLine(std::size_t cursor_line);
    void insertCharAt(std::size_t cursor_line, std::size_t cursor_col, char c);
    void eraseCharAt(std::size_t cursor_line, std::size_t cursor_col);
    void appendLineTo(std::size_t target_line, std::size_t source_line);

    
    Text& getText() { return text; }

private:
    Text text;
};
