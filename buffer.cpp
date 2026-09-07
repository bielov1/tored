#include "buffer.h"

void Buffer::insertLine(std::string line)
{
    getText().emplace_back(line.size(), std::move(line));
}

void Buffer::removeLine(std::size_t cursor_line)
{
    if (cursor_line < getText().size()) {
	getText().erase(getText().begin() + cursor_line);
    }
}

void Buffer::insertCharAt(std::size_t cursor_line, std::size_t cursor_col, char c)
{
    if (cursor_line >= getText().size()) {
	getText().emplace_back(1, std::string(1, c));
        return;
    }

    getText()[cursor_line].second.insert(cursor_col, 1, c);
    getText()[cursor_line].first += 1;
}

void Buffer::eraseCharAt(std::size_t cursor_line, std::size_t cursor_col)
{
    getText()[cursor_line].second.erase(cursor_col, 1);
    getText()[cursor_line].first -= 1;
}

void Buffer::appendLineTo(std::size_t target_line, std::size_t source_line) {
    if (target_line < getText().size() && source_line < getText().size()) {
	getText()[target_line].second += getText()[source_line].second;
	getText()[target_line].first  = getText()[target_line].second.size();
    }
}

void Buffer::splitLineAt(std::size_t cursor_line, std::size_t cursor_col)
{
    if (cursor_line >= getText().size()) return;

    auto& current_str = getText()[cursor_line].second;

    if (cursor_col > current_str.size()) {
        cursor_col = current_str.size();
    }

    std::string right_part = current_str.substr(cursor_col);

    current_str.erase(cursor_col);
    getText()[cursor_line].first = current_str.size();

    getText().insert(getText().begin() + cursor_line + 1, {right_part.size(), right_part});
}
