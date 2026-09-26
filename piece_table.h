#pragma once

#include <string>
#include <algorithm>
#include <set>
#include <functional>
#include <ranges>

#include "btree.h"

struct Data;
struct OverlayBuffer;
struct LineCol;
class IBuffer
{
public:
    IBuffer() = default;
    virtual ~IBuffer() = default;
    virtual void insert(std::size_t offset, const std::string& text) = 0;
    virtual void remove(std::size_t offset, std::size_t length) = 0;
    virtual char getCharAt(std::size_t offset) = 0;
    //virtual std::vector<std::string_view> getLine(std::size_t line_idx, const OverlayBuffer& overlay) = 0;
    virtual std::vector<std::string_view> getLineSlices(std::size_t line_idx) noexcept = 0;
    virtual std::size_t getLineStart(std::size_t line_idx) const = 0;
    virtual std::size_t getLineEnd(std::size_t line_idx) const = 0;
    virtual std::size_t getLineCount() const = 0;
    virtual std::size_t getTotalLength() const = 0;
    virtual LineCol offsetToLineCol(std::size_t offset) const = 0;
    virtual bool empty() const = 0;
};

struct OverlayBuffer
{
    std::size_t start_offset{0};
    std::string text;

    OverlayBuffer(std::size_t offset)
	: start_offset{ offset }
	, text{ "" }
    {}

    bool empty() const { return text.empty(); }

    void append(char c) {
        text.push_back(c);
    }

    void append(std::string_view sv) {
        text.append(sv);
    }

    void clear() {
        text.clear();
        start_offset = 0;
    }
};

enum class SourceType { ORIGINAL, ADD };
struct Piece
{
    SourceType source;
    std::size_t offset;
    std::size_t length;
};

struct Data
{
    std::size_t key;
    Piece piece;
};

struct LineCol
{
    std::size_t line;
    std::size_t col;
};

class PieceTable : public IBuffer
{
public:    
    PieceTable(const std::string& text_buffer = "");
    ~PieceTable() = default;

    void insert(std::size_t offset, const std::string& text) override final;
    void remove(std::size_t offset, std::size_t length) override final;
    char getCharAt(std::size_t offset) override final;
    //std::vector<std::string_view> getLineSlices(std::size_t line_idx, const OverlayBuffer& overlay) override final;
    std::vector<std::string_view> getLineSlices(std::size_t line_idx) noexcept override final;
    std::size_t getLineStart(std::size_t line_idx) const override final;
    std::size_t getLineEnd(std::size_t line_idx) const override final;
    std::size_t getLineCount() const override final;
    std::size_t getTotalLength() const override final;
    LineCol offsetToLineCol(std::size_t offset) const override final;
    bool empty() const override final;
    
    // void insertLine(std::string line);
    // void removeLine(std::size_t cursor_line);
    // void insertCharAt(std::size_t cursor_line, std::size_t cursor_col, char c);
    // void eraseCharAt(std::size_t cursor_line, std::size_t cursor_col);
    // void appendLineTo(std::size_t target_line, std::size_t source_line);
    // void splitLineAt(std::size_t cursor_line, std::size_t cursor_col);
    
private:
    void init(const std::string& text_buffer);
    void replace(std::size_t piece_with_key,
		 std::vector<std::unique_ptr<Data>> with_elems,
		 std::size_t delta);
    void replaceRange(const std::vector<const Data*>& affected,
		      std::vector<std::unique_ptr<Data>> with_elems,
		      std::size_t range_start,
		      std::size_t delta);
    std::vector<std::size_t> findNewlineOffsets(std::string_view text);
    void updateLineStartsOnInsert(std::size_t insert_offset, std::string_view inserted_text);
    void updateLineStartsOnRemove(std::size_t remove_offset, std::size_t remove_length);
    std::pair<const Piece, std::size_t> findPieceAt(std::size_t cur_offset);
    std::vector<const Data*> findPiecesInRange(std::size_t left_bound,
					       std::size_t right_bound);
    std::size_t pieceStart(const Data& d) const { return d.key; }
    std::size_t pieceEnd(const Data& d) const { return d.key + d.piece.length; }
    
    static constexpr std::size_t M = 4;
    std::string original_buf;
    std::string add_buf;
    
    using BufferOffset = std::size_t;
    std::vector<BufferOffset> line_starts;
    btree::BTree<Data, M> pieces;
};

static Piece makePiece(SourceType type, std::size_t offset, std::size_t len)
{
    Piece piece = {
	.source = type,
	.offset = offset,
	.length = len
    };
    return piece;
}
