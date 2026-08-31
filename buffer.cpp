#include "buffer.h"

void Buffer::insertLine(std::string line)
{
    text.emplace_back(line.size(), std::move(line));
}

void Buffer::removeLine(std::size_t cursor_line)
{
    if (cursor_line < text.size()) {
        text.erase(text.begin() + cursor_line);
    }
}

void Buffer::insertCharAt(std::size_t cursor_line, std::size_t cursor_col, char c)
{
    if (cursor_line >= text.size()) {
	text.emplace_back(1, std::string(1, c));
        return;
    }

    text[cursor_line].second.insert(cursor_col, 1, c);
    text[cursor_line].first += 1;
}

void Buffer::eraseCharAt(std::size_t cursor_line, std::size_t cursor_col)
{
    text[cursor_line].second.erase(cursor_col, 1);
    text[cursor_line].first -= 1;
}

void Buffer::appendLineTo(std::size_t target_line, std::size_t source_line) {
    if (target_line < text.size() && source_line < text.size()) {
        text[target_line].second += text[source_line].second;
        text[target_line].first  = text[target_line].second.size();
    }
}
