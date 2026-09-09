#include "piece_table.h"

PieceTable::PieceTable(const std::string& text_buffer)
    : original_buffer{text_buffer}
    , add_buffer{""}
    , pieces{}
{
    pieces.emplace(0, createPiece(0, static_cast<int>(text_buffer.length()), ORIGINAL))
}

PieceTable::~PieceTable() = default;

void PieceTable::insert(std::size_t cursor_offset, const std::string& text)
{
    std::size_t add_buffer_offset = add_buffer.size();
    add_buffer += text;

    auto [piece, offset_within_piece] = findPieceAt(cursor_offset);

    
}

static Piece PieceTable::createPiece(const std::size_t start_index, std::size_t len, Source source_type)
{
    Piece new_piece{
	.source = source_type,
	.offset = start_index,
	.length = len
    };
    return newpiece;
}

auto PieceTable::findPieceAt(std::size_t offset)
{

}

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
	std::size_t offset = 0;
	getText().emplace_back(offset, std::string(1, c));
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
