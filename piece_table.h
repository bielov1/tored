#pragma once

#include <print>
#include <set>
#include <string>

class PieceTable
{
public:
    enum Source { ORIGINAL, ADD };
    struct Piece
    {
	Source source;
	std::size_t offset;
	std::size_t length;
    };

    PieceTable(const std::string& text_buffer = "");
    ~PieceTable();

    std::string& getText();
    void insert(std::size_t offset, const std::string& text);
    
    // void insertLine(std::string line);
    // void removeLine(std::size_t cursor_line);
    // void insertCharAt(std::size_t cursor_line, std::size_t cursor_col, char c);
    // void eraseCharAt(std::size_t cursor_line, std::size_t cursor_col);
    // void appendLineTo(std::size_t target_line, std::size_t source_line);
    // void splitLineAt(std::size_t cursor_line, std::size_t cursor_col);
    
private:
    static Piece createPiece(const std::size_t offset, std::size_t length, Source source_type);
    auto findPieceAt(std::size_t offset);
    std::string original_buffer;
    std::string add_buffer;
    std::map<std::size_t, Piece> piece_sequence;
};

std::shared_ptr<PieceTable> ptCreate(std::string text)
{
    
}
