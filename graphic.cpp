#include "graphic.h"

void BufferView::drawChar(const Font& font, char c, int char_width, int char_height, Vec2f pos, Color color)
{
    int idx = GetGlyphIndex(font, (int)c);
    if (idx >= 0 && idx < font.glyphCount) {
	Rectangle src = font.recs[idx];
	Rectangle dst = {
	    pos.x,
	    pos.y,
	    static_cast<float>(char_width),
	    static_cast<float>(char_height)
	};
	DrawTexturePro(font.texture, src, dst, Vector2{ 0.0f, 0.0 }, 0.0f, color);
    }    
}

void BufferView::draw(const Font& font, int char_width, int char_height)
{
    assert(font.recs);
    assert(font.glyphCount > 0);
    
    if (getBufferText().empty()) return;

    std::size_t i;
    std::size_t j;

    Vec2f start_pos = { window_rect->x, window_rect->y };
    Vec2f draw_char_pos = start_pos;

    // draw windows bounds
    DrawRectangleLinesEx(*window_rect, 2.f, WHITE);
    
    // render text on screen
    std::size_t end_line = view_port->first_visible_line + view_port->visible_lines;
    for (i = view_port->first_visible_line; i < end_line && i < getBufferText().size(); ++i) {
	const auto& [line_size, line_text] = getBufferText()[i];
	
	std::size_t col_idx = 0;
	for (j = view_port->first_visible_col; j < line_size; ++j) {
            if (col_idx >= view_port->visible_cols) break;

            drawChar(font, line_text[j], char_width, char_height, draw_char_pos, WHITE);
            draw_char_pos.x += char_width;
            col_idx++;
        }

        draw_char_pos.y += char_height;
        draw_char_pos.x  = start_pos.x;
    }

    // render cursor
    float rel_col = static_cast<float>(getCursorCol() - view_port->first_visible_col);
    float rel_line = static_cast<float>(getCursorLine() - view_port->first_visible_line);
   
    Vec2f cursor_pixel_pos{
	start_pos.x + rel_col * char_width,
	start_pos.y + rel_line * char_height
    };
    
    Rectangle cursor_rec = {
	.x      = cursor_pixel_pos.x,
	.y      = cursor_pixel_pos.y,
	.width  = static_cast<float>(char_width),
	.height = static_cast<float>(char_height)
    };

    
    auto curr_cursor_draw_type = getCursorDrawType();
    if (curr_cursor_draw_type == CursorDrawType::Filled) {
	DrawRectangleRec(cursor_rec, WHITE);
    } else if (curr_cursor_draw_type == CursorDrawType::Hollow) {
	DrawRectangleLinesEx(cursor_rec, 1.f, WHITE);
    }

    if (getCursorLine() < getBufferText().size()) {
	const auto& [line_size, line_text] = getBufferText()[getCursorLine()];
	if (getCursorCol() < line_size) {
	    char c = line_text[getCursorCol()];
	    if (c != '\n') {
		if (curr_cursor_draw_type == CursorDrawType::Filled) {
		    drawChar(font, c, char_width, char_height, cursor_pixel_pos, BLACK);
		} else if (curr_cursor_draw_type == CursorDrawType::Hollow) {
		    drawChar(font, c, char_width, char_height, cursor_pixel_pos, WHITE);
		}
	    }
	}
    }
}

void Window::moveCursorLeft()
{
    if (getBufferText().empty()) return;    
    if (getCursorCol() > 0) {
	getCursor().retreatCol();
    } else if (getCursorLine() > 0) {
	std::size_t prev_line_size = getBufferText()[getCursorLine() - 1].first;
	getCursor().setPosition(getCursorLine() - 1, prev_line_size);
    }
    scrollToCursor();
}

void Window::moveCursorRight()
{
    if (getBufferText().empty()) return;
    std::size_t line_size = getBufferText()[getCursorLine()].first;
    if (getCursorCol() < line_size) {
        getCursor().advanceCol();
    } else if (getCursorLine() + 1 < getBufferText().size()) {
        getCursor().setPosition(getCursorLine() + 1, 0);
    }
    scrollToCursor();
}

void Window::moveCursorUp()
{
    if (getBufferText().empty()) return;
    if (getCursorLine() > 0) {
	std::size_t prev_line_size = getBufferText()[getCursorLine() - 1].first;
	std::size_t new_col = std::ranges::clamp(getCursorCol(), std::size_t{0}, prev_line_size);
	getCursor().setPosition(getCursorLine() - 1, new_col);
    }
    scrollToCursor();
}

void Window::moveCursorDown()
{    
    if (getBufferText().empty()) return;
    if (getCursorLine() + 1 < getBufferText().size()) {
	std::size_t next_line_size = getBufferText()[getCursorLine() + 1].first;
	std::size_t new_col = std::ranges::clamp(getCursorCol(), std::size_t{0}, next_line_size);
	getCursor().setPosition(getCursorLine() + 1, new_col);
    }
    scrollToCursor();
}

void Window::backspaceOnCursor(LayoutTree& root_tree)
{
    if (getBufferText().empty()) return;
    if (getCursorCol() > 0) {
	getBuffer().eraseCharAt(getCursorLine(), getCursorCol() - 1);
	getCursor().retreatCol();
    } else if (getCursorLine() > 0) {
        std::size_t prev_line_size = getBufferText()[getCursorLine() - 1].first;
	getBuffer().appendLineTo(getCursorLine() - 1, getCursorLine());
	getBuffer().removeLine(getCursorLine());
	getCursor().setPosition(getCursorLine() - 1, prev_line_size);
    }
    recalculateCursor(root_tree, getCursorLine(), getCursorCol());
}

void Window::newlineOnCursor()
{
    getBuffer().splitLineAt(getCursorLine(), getCursorCol());
    getCursor().setPosition(getCursorLine() + 1, 0);
    scrollToCursor();
}

void Window::insertChar(char c)
{
    getBuffer().insertCharAt(getCursorLine(), getCursorCol(), c);
    getCursor().advanceCol();
    scrollToCursor();
}

void Window::scrollToCursor()
{
    std::size_t padding = 5;   
    if (getCursorLine() < view_port.first_visible_line)
	view_port.first_visible_line = getCursorLine() < padding ? 0 : getCursorLine() - padding;

    if (getCursorLine() >= view_port.first_visible_line + view_port.visible_lines) {
	if (padding > view_port.visible_lines) padding = view_port.visible_lines - 1;
	view_port.first_visible_line = getCursorLine() - view_port.visible_lines + padding;
    }

    if (getCursorCol() < view_port.first_visible_col)
	view_port.first_visible_col = getCursorCol() < padding ? 0 : getCursorCol() - padding;

    if (getCursorCol() >= view_port.first_visible_col + view_port.visible_cols) {
	if (padding > view_port.visible_cols) padding = view_port.visible_cols - 1;
	view_port.first_visible_col = getCursorCol() - view_port.visible_cols + padding;
    }
}

void Window::recalcViewPort(int char_width, int char_height)
{
    view_port.visible_cols = static_cast<std::size_t>(rect.width / char_width);
    view_port.visible_lines = static_cast<std::size_t>(rect.height / char_height);
}

void Window::attachBufferView()
{
    auto view = std::make_shared<BufferView>(&rect, &view_port, &cursor, buffer.get());
    add(view);
}
