#include "piece_table.h"

static Piece makePiece(SourceType type, std::size_t offset, std::size_t len)
{
    Piece piece = {
	.source = type,
	.offset = offset,
	.length = len
    };
    return piece;
}

PieceTable::PieceTable(const std::string text_buffer)
    : original_buf{ text_buffer }
    , add_buf{ "" }
    , line_starts{ 0 }
    , pieces{}
{
    init(text_buffer);
}

void PieceTable::insert(std::size_t cursor_offset, const std::string& text)
{
    updateLineStartsOnInsert(cursor_offset, text);
    std::size_t add_buf_offset = add_buf.size();
    add_buf += text;
    
    if (pieces.empty()) {
	pieces.insert(std::make_unique<Data>(Data{
		    .key = 0,
		    .piece = makePiece(SourceType::ADD, add_buf_offset, text.size())
		}));
	return;
    }

    const auto& [piece, accum_piece_offset] = findPieceAt(cursor_offset);
    std::size_t offset_within_piece = cursor_offset < accum_piece_offset ? 0 : cursor_offset - accum_piece_offset;

    std::unique_ptr<Data> left{nullptr};
    std::unique_ptr<Data> newIn{nullptr};
    std::unique_ptr<Data> right{nullptr};

    if (offset_within_piece == 0) {
	// cursor at the start of the piece
	newIn = std::make_unique<Data>(Data{
		.key = accum_piece_offset,
		.piece = makePiece(SourceType::ADD, add_buf_offset, text.size())
	    });
	right = std::make_unique<Data>(Data{
		.key = newIn->key + newIn->piece.length,
		.piece = piece
	    });
    } else if (offset_within_piece == piece.length) {
	// cursor at the end of the piece
	left = std::make_unique<Data>(Data{
		.key = accum_piece_offset,
		.piece = piece
	    });
	newIn = std::make_unique<Data>(Data{
		.key = left->key + offset_within_piece,
		.piece = makePiece(SourceType::ADD, add_buf_offset, text.size())
	    });
    } else {
	// cursor inbetween of the piece
	left = std::make_unique<Data>(Data{
		.key = accum_piece_offset,
		.piece = makePiece(piece.source, piece.offset, offset_within_piece)
	    });
	newIn = std::make_unique<Data>(Data{
		.key = left->key + offset_within_piece,
		.piece = makePiece(SourceType::ADD, add_buf_offset, text.size())
	    });
	right = std::make_unique<Data>(Data{
		.key = newIn->key + newIn->piece.length,
		.piece = makePiece(piece.source,
				   piece.offset + offset_within_piece,
				   piece.length - offset_within_piece)
	    });
    }

    std::vector<std::unique_ptr<Data>> new_pieces;
    if (left)  new_pieces.push_back(std::move(left));
    if (newIn) new_pieces.push_back(std::move(newIn));
    if (right) new_pieces.push_back(std::move(right));
    
    replace(accum_piece_offset, std::move(new_pieces), text.size());
}

void PieceTable::remove(std::size_t cursor_offset, std::size_t length)
{
    if (pieces.empty()) return;
    // find all pieces that overlap [offset, offset + length)
    auto affected_pieces = findPiecesInRange(cursor_offset, cursor_offset + length);
    if (affected_pieces.empty()) return;

    updateLineStartsOnRemove(cursor_offset, length);
    
    std::vector<std::unique_ptr<Data>> new_pieces;
    for (const auto* p : affected_pieces) {
	std::size_t left_trim = (cursor_offset > pieceStart(*p)) ? cursor_offset - pieceStart(*p) : 0;

	std::size_t right_trim = (pieceEnd(*p) > (cursor_offset + length)) ? pieceEnd(*p) - (cursor_offset + length) : 0;

	if (left_trim > 0) {
	    new_pieces.push_back(std::make_unique<Data>(Data{
			.key = pieceStart(*p),
			.piece = makePiece(p->piece.source,
					   p->piece.offset,
					   left_trim)
		    }));
	}

	if (right_trim > 0) {
	    new_pieces.push_back(std::make_unique<Data>(Data{
			.key = cursor_offset,
			.piece = makePiece(p->piece.source,
					   p->piece.offset + p->piece.length - right_trim,
					   right_trim)
		    }));
	}
    }
    replaceRange(affected_pieces, std::move(new_pieces), cursor_offset, length);
}

char PieceTable::getCharAt(std::size_t offset) const
{
    if (offset == getTotalLength()) return '\0';
    const auto& [piece, accum_piece_offset] = findPieceAt(offset);
    std::size_t offset_within_piece = offset < accum_piece_offset ? 0 : offset - accum_piece_offset;
    if (piece.source == SourceType::ORIGINAL) {
	return original_buf.at(piece.offset + offset_within_piece);
    } else {
	return add_buf.at(piece.offset + offset_within_piece);
    }
}

std::vector<std::string_view> PieceTable::getLineSlices(std::size_t line_idx) const noexcept
{
    std::vector<std::string_view> views{};

    std::size_t buffer_length = getTotalLength();
    std::size_t line_start = getLineStart(line_idx);
    std::size_t line_end   = (line_idx + 1 < line_starts.size()) ? getLineStart(line_idx + 1) : buffer_length;

    std::size_t offset_within_line = line_start;
    while (offset_within_line < line_end) {
	const auto& [piece, accum_piece_offset] = findPieceAt(offset_within_line);
	const auto* base_buf = (piece.source == SourceType::ORIGINAL) ? original_buf.data() : add_buf.data();
	views.push_back(std::string_view{base_buf + piece.offset, piece.length});	
	offset_within_line += piece.length;
    }
    return views;
}

std::size_t PieceTable::getLineStart(std::size_t line_idx) const
{
    assert(line_starts.size() > line_idx);
    return line_starts[line_idx];
}

std::size_t PieceTable::getLineEnd(std::size_t line_idx) const
{
    if (line_idx + 1 < line_starts.size()) {
	return line_starts[line_idx + 1] - 1;
    } else {
	return getTotalLength();
    }
}

std::size_t PieceTable::getLineCount() const
{
    return line_starts.size();
}

std::size_t PieceTable::getTotalLength() const
{
    if (pieces.empty()) return 0;
    const auto* data = pieces.getRoot()->getRightMostData();
    return data->key + data->piece.length;
}

LineCol PieceTable::offsetToLineCol(std::size_t offset) const
{
    LineCol lc{0, 0};

    auto it = std::upper_bound(line_starts.begin(), line_starts.end(), offset);

    if (it != line_starts.begin()) {
        lc.line = static_cast<std::size_t>(std::distance(line_starts.begin(), it) - 1);
    } else {
        lc.line = 0;
    }
    
    std::size_t line_start_offset = line_starts[lc.line];
    lc.col = (offset >= line_start_offset) ? (offset - line_start_offset) : 0;

    return lc;
}

bool PieceTable::empty() const
{
    return pieces.empty();
}

void PieceTable::init(const std::string& text_buffer)
{
    if (text_buffer.empty()) {
        return;
    }

    auto initial_piece = std::make_unique<Data>(Data{
        .key = 0,
        .piece = makePiece(SourceType::ORIGINAL, 0, text_buffer.size())
    });
    pieces.insert(std::move(initial_piece));
    updateLineStartsOnInsert(0, text_buffer);
}

void PieceTable::replace(std::size_t piece_with_key,
			 std::vector<std::unique_ptr<Data>> with_elems,
			 std::size_t delta)
{
    /* before inserting new elems into the tree, we should update nodes keys above piece_with_key*/

    pieces.erase(piece_with_key);
    pieces.incrementKeysAbove(piece_with_key, delta);
    pieces.insert(std::move(with_elems));
}

void PieceTable::replaceRange(const std::vector<const Data*>& affected,
		  std::vector<std::unique_ptr<Data>> with_elems,
		  std::size_t range_start,
		  std::size_t delta)
{
    for (const auto* p : affected) {
	pieces.erase(p->key);
    }

    pieces.decrementKeysAbove(range_start, delta);
    if (!with_elems.empty()) {
	pieces.insert(std::move(with_elems));
    }
}

std::vector<std::size_t> PieceTable::findNewlineOffsets(std::string_view text) {
    std::vector<std::size_t> offsets;
    for (std::size_t i = 0; i < text.size(); ++i) {
        if (text[i] == '\n') {
            offsets.push_back(i);
        }
    }
    return offsets;
}

void PieceTable::updateLineStartsOnInsert(std::size_t insert_offset, std::string_view inserted_text)
{
    if (inserted_text.empty()) return;

    std::size_t delta = inserted_text.size();

    for (auto& line_offset : line_starts) {
        if (line_offset > insert_offset) {
            line_offset += delta;
        }
    }

    auto nl_positions = findNewlineOffsets(inserted_text);
    if (nl_positions.empty()) return;

    auto insert_pos = std::upper_bound(line_starts.begin(), line_starts.end(), insert_offset);

    std::vector<BufferOffset> new_line_starts;
    new_line_starts.reserve(nl_positions.size());

    for (std::size_t pos : nl_positions) {
        new_line_starts.push_back(insert_offset + pos + 1);
    }

    line_starts.insert(insert_pos, new_line_starts.begin(), new_line_starts.end());
}

void PieceTable::updateLineStartsOnRemove(std::size_t remove_offset, std::size_t remove_length)
{
    if (remove_length == 0 || line_starts.empty()) return;

    std::size_t remove_end = remove_offset + remove_length;

    std::erase_if(line_starts, [remove_offset, remove_end](BufferOffset offset) {
        return offset > remove_offset && offset <= remove_end;
    });

    for (auto& line_offset : line_starts) {
        if (line_offset > remove_end) {
            line_offset -= remove_length;
        }
    }
}

std::pair<const Piece, std::size_t> PieceTable::findPieceAt(std::size_t cur_offset) const
{
    const auto* data = pieces.findDataAt(cur_offset);

    if (!data) {
        data = pieces.getRightMostPiece();
    }
    assert(data);
    return std::make_pair(data->piece, data->key);
}

std::vector<const Data*> PieceTable::findPiecesInRange(std::size_t left_bound,
							std::size_t right_bound) const
{
    return pieces.findDataInRange(left_bound, right_bound);
}
