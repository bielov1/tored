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
    void insertCharAtCursor(std::size_t line, std::size_t col, char c);
    Text& getText() { return text; }

private:
    Text text;
};
