#include "graphic.h"

template<typename DrawCharFunc>
void BufferView::drawCursor(const Vec2f& cursor_draw_pos, DrawCharFunc&& drawChar, const CharParams params)
{
    Rectangle cursor_rec = {
	.x      = cursor_draw_pos.x,
	.y      = cursor_draw_pos.y,
	.width  = params.width,
	.height = params.height
    };

    DrawRectangleRec(cursor_rec, WHITE);
        
    char c = buffer->getCharAt(cursor->getOffset());
    if (c != '\n' && c != '\t' && c != '\0') {
	Vec2f temp_pos = cursor_draw_pos;
	drawChar(c, temp_pos, BLACK);
    }
}

template<typename DrawCharFunc, typename WrapFunc>
void BufferView::drawOverlay(Vec2f& pos, Vec2f& cursor_draw_pos,
                             std::size_t offset_counter,
                             DrawCharFunc&& drawChar,
                             WrapFunc&& getWrappedPos)
{
    for (const auto& c : overlay_buf->text) {
        pos = getWrappedPos(pos, c);

        if (cursor->getOffset() == offset_counter) {
            cursor_draw_pos = pos;
        }

        drawChar(c, pos, WHITE);
    }
}

void BufferView::draw(const Font& font, const CharParams params)
{
    Vec2f start_pos = { window_rect->x, window_rect->y };
    Vec2f draw_char_pos = start_pos;
    
    auto drawChar = [&](char c, Vec2f& pos, Color color) {
	if (c == '\n') {
	    pos.x  = start_pos.x;
	    pos.y += params.height;
	    return;
	} else if (c == '\t') {
	    pos.x += params.width * 4;
	    return;
	}
	
	int idx = GetGlyphIndex(font, c);
	if (idx >= 0 && idx < font.glyphCount) {
	    Rectangle src = font.recs[idx];
	    Rectangle dst = {
		pos.x,
		pos.y,
		params.width,
		params.height
	    };
        
	    DrawTexturePro(font.texture, src, dst, Vector2{ 0.0f, 0.0f }, 0.0f, color);
	}
	pos.x += params.width;
    };

    auto getWrappedPos = [&](Vec2f pos, char c) -> Vec2f {
	if (c != '\n' && pos.x + params.width > view_port->visible_cols * params.width) {
	    pos.x  = start_pos.x;
	    pos.y += params.height;
	}
	return pos;
    };
    
    Vec2f cursor_draw_pos = { 0.f, 0.f };
    assert(overlay_buf);
    if (buffer->empty() && overlay_buf->empty()) {
	drawCursor(cursor_draw_pos, drawChar, params);
	return;
    }
    
    // draw text    
    std::size_t line_count = buffer->getLineCount();
    std::size_t last_visible_line = view_port->first_visible_line + view_port->visible_lines;

    std::size_t offset_counter = buffer->getLineStart(view_port->first_visible_line);
    for (std::size_t i = view_port->first_visible_line; i < line_count && i < last_visible_line; ++i) {
	auto line_slices = buffer->getLineSlices(i);
	if (line_slices.empty()) break;
	for (const auto& slice : line_slices) {
	    for (const auto& c : slice) {
		if (c == '\0') break;
		if (!overlay_buf->empty() && offset_counter == overlay_buf->start_offset) {
		    drawOverlay(draw_char_pos, cursor_draw_pos, offset_counter, drawChar, getWrappedPos);
		}

		draw_char_pos = getWrappedPos(draw_char_pos, c);
		
		if (cursor->getOffset() == offset_counter) {
		    cursor_draw_pos = draw_char_pos;
		}
		
		drawChar(c, draw_char_pos, WHITE);
		offset_counter++;
	    }
	}
    }

    // overlay buffer at the end of the text
    if (!overlay_buf->empty() && offset_counter == overlay_buf->start_offset) {
	drawOverlay(draw_char_pos, cursor_draw_pos, offset_counter, drawChar, getWrappedPos);
    }
    
    // draw cursor
    if (cursor->getOffset() == offset_counter) {
	cursor_draw_pos = draw_char_pos;
    }
    
    drawCursor(cursor_draw_pos, drawChar, params);
}
// ===============================================================================

void Window::dumpOverlayBuffer()
{
    if (overlay_buf.empty()) return;

    buffer->insert(overlay_buf.start_offset, overlay_buf.text);
    cursor.setOffset(overlay_buf.start_offset + overlay_buf.text.size());
    overlay_buf.clear();
}

void Window::handleCharInput(char c)
{
    if (overlay_buf.empty()) {
	overlay_buf.start_offset = cursor.getOffset();
    }

    overlay_buf.append(c);
}

void Window::handleNavigationOrActionKey(KeyInputTag key)
{
    dumpOverlayBuffer();
    switch (key) {
    case KeyInputTag::KIT_BACKSPACE:
	backspaceOnCursor();
	break;
    case KeyInputTag::KIT_ENTER:
	newlineOnCursor();
	break;
    case KeyInputTag::KIT_LEFT:
	moveCursorLeft();
	break;
    case KeyInputTag::KIT_RIGHT:
	moveCursorRight();
	break;
    case KeyInputTag::KIT_UP:
	moveCursorUp();
	break;
    case KeyInputTag::KIT_DOWN:
	moveCursorDown();
	break;
    default:
	std::fprintf(stderr, "[WARNING] uknown key input\n");
    }
    scrollToCursor();
}

std::size_t Window::lineSize(std::size_t line) const
{
    return buffer->getLineEnd(line) - buffer->getLineStart(line);
}


void Window::moveCursorLeft()
{
    if (buffer->empty()) return;
    if (cursor.getOffset() > 0) {
	cursor.retreatOffset();
    }
}

void Window::moveCursorRight()
{
    if (buffer->empty()) return;
    if (cursor.getOffset() < buffer->getTotalLength()) {
	cursor.advanceOffset();
    }
}

void Window::moveCursorUp()
{
    if (buffer->empty()) return;
    auto [line, col] = buffer->offsetToLineCol(cursor.getOffset());
    std::size_t width = view_port.visible_cols;

    std::size_t row = col / width;
    std::size_t norm_col = col % width;

    if (row > 0) {
	std::size_t new_col_within_line = (row - 1) * width + norm_col;
	cursor.setOffset(buffer->getLineStart(line) + new_col_within_line);
    } else if (line > 0) {
	std::size_t prev_line_size = lineSize(line - 1);
	std::size_t last_row  = prev_line_size / width;
	std::size_t col_within_prev_line = std::min(prev_line_size, last_row * width + norm_col);
	cursor.setOffset(buffer->getLineStart(line - 1) + col_within_prev_line);
    }
}

void Window::moveCursorDown()
{
    if (buffer->empty()) return;
    // include <iostream>int main(){    while (true) {\n\tstatic int count = 0;\n
    //  | cursor pos                       | view_cols
    // to go to the next line from current cursor position,
    auto [line, col] = buffer->offsetToLineCol(cursor.getOffset());
    std::size_t width = view_port.visible_cols;
    std::size_t line_size = lineSize(line);

    std::size_t row = col / width;
    std::size_t last_row = line_size / width;
    std::size_t norm_col = col % width;

    if (row < last_row) {
	// same logical line
	std::size_t new_col_within_line = std::min(line_size, (row + 1) * width + norm_col);
	cursor.setOffset(buffer->getLineStart(line) + new_col_within_line);
    } else if (line + 1 < buffer->getLineCount()) {
	std::size_t col_within_next_line = std::min(lineSize(line + 1), norm_col);
	cursor.setOffset(buffer->getLineStart(line + 1) + col_within_next_line);  
    }
}

void Window::backspaceOnCursor()
{
    if (buffer->empty()) return;
    if (cursor.getOffset() > 0) {
	buffer->remove(cursor.getOffset() - 1, 1);
	cursor.retreatOffset();
    }
}

void Window::newlineOnCursor()
{
    std::size_t current_offset = cursor.getOffset();
    buffer->insert(current_offset, "\n");
    cursor.advanceOffset();
}

void Window::scrollToCursor()
{
    LineCol cursor_lc = buffer->offsetToLineCol(cursor.getOffset());

    if (cursor_lc.line >= view_port.first_visible_line + view_port.visible_lines) {
        view_port.first_visible_line = cursor_lc.line - view_port.visible_lines + 1;
    } else if (cursor_lc.line < view_port.first_visible_line) {
        view_port.first_visible_line = cursor_lc.line;
    }
}

void Window::loadFile(const char *file_path)
{
    std::ifstream ifs{file_path, std::ios_base::binary | std::ios_base::ate};
    if (!ifs.good()) {
	std::println("Unable to open {}", file_path);
	return;
    }
    std::size_t file_size = ifs.tellg();
    std::string content(file_size, '\0');
    ifs.seekg(0, std::ios::beg);
    ifs.read(content.data(), file_size);
    std::println("{}", content);
    buffer->init(std::move(content));
}

void Window::recalcViewPort(int char_width, int char_height)
{
    assert(char_width > 0);
    assert(char_height > 0);
    const auto cols  = static_cast<std::size_t>(rect.width  / static_cast<float>(char_width));
    const auto lines = static_cast<std::size_t>(rect.height / static_cast<float>(char_height));

    view_port.visible_cols  = std::max<std::size_t>(1, cols);
    view_port.visible_lines = std::max<std::size_t>(1, lines);
}

void Window::attachBufferView()
{
    auto view = std::make_shared<BufferView>(&rect, &view_port, &cursor, &overlay_buf, buffer.get());
    add(view);
}
// ===============================================================================
