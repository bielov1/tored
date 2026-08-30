#include "buffer.h"

void Buffer::insertLine(std::string line)
{
    text.emplace_back(line.size(), std::move(line));
}

void Buffer::insertCharAtCursor(std::size_t line, std::size_t col, char c)
{
    if (line >= text.size()) {
	text.emplace_back(1, std::string(1, c));
        return;
    }

    text[line].second.insert(col, 1, c);
    text[line].first += 1;
}
