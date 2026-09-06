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
   
    const auto& text = buffer->getText();
    
    if (text.empty()) return;

    std::size_t i;
    std::size_t j;
    
    std::size_t text_size = text.size();    

    Vec2f start_pos = { window_rect->x, window_rect->y };
    Vec2f draw_char_pos = start_pos;

    // draw windows bounds
    DrawRectangleLinesEx(*window_rect, 2.f, WHITE);
    
    // render text on screen
    std::size_t end_line = view_port->first_visible_line + view_port->visible_lines;
    for (i = view_port->first_visible_line; i < end_line && i < text_size; ++i) {
	const auto& [line_size, line_text] = text[i];
	
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
    float rel_col = static_cast<float>(cursor->getCol() - view_port->first_visible_col);
    float rel_line = static_cast<float>(cursor->getLine() - view_port->first_visible_line);
   
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

    
    auto curr_cursor_draw_type = cursor->getDrawType();
    if (curr_cursor_draw_type == CursorDrawType::Filled) {
	DrawRectangleRec(cursor_rec, WHITE);
    } else if (curr_cursor_draw_type == CursorDrawType::Hollow) {
	DrawRectangleLinesEx(cursor_rec, 1.f, WHITE);
    }

    if (cursor->getLine() < text_size) {
	const auto& [line_size, line_text] = text[cursor->getLine()];
	if (cursor->getCol() < line_size) {
	    char c = line_text[cursor->getCol()];
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
    const auto& text = buffer->getText();
    if (text.empty()) return;

    std::size_t current_line = cursor.getLine();
    
    if (cursor.getCol() > 0) {
	cursor.retreatCol();
    } else if (current_line > 0) {
	std::size_t prev_line_size = text[current_line - 1].first;
	cursor.setPosition(current_line - 1, prev_line_size);
    }
    
    scrollToCursor();
}

void Window::moveCursorRight()
{
    const auto& text = buffer->getText();
    if (text.empty()) return;

    std::size_t current_line = cursor.getLine();
    std::size_t line_size = text[current_line].first;

    if (cursor.getCol() < line_size) {
        cursor.advanceCol();
    } else if (current_line + 1 < text.size()) {
        cursor.setPosition(current_line + 1, 0);
    }
    scrollToCursor();
}

void Window::moveCursorUp()
{
    const auto& text = buffer->getText();
    if (text.empty()) return;

    std::size_t current_line = cursor.getLine();
	
    if (current_line > 0) {
	std::size_t prev_line_size = text[current_line - 1].first;
	std::size_t new_col = std::ranges::clamp(cursor.getCol(), std::size_t{0}, prev_line_size);
	cursor.setPosition(current_line - 1, new_col);
    }
    scrollToCursor();
}

void Window::moveCursorDown()
{    
    const auto& text = buffer->getText();
    if (text.empty()) return;

    std::size_t current_line = cursor.getLine();
	
    if (current_line + 1 < text.size()) {
	std::size_t next_line_size = text[current_line + 1].first;
	std::size_t new_col = std::ranges::clamp(cursor.getCol(), std::size_t{0}, next_line_size);
	cursor.setPosition(current_line + 1, new_col);
    }
    scrollToCursor();
}

void Window::backspaceOnCursor(LayoutTree& root_tree)
{
    const auto& text = buffer->getText();
    if (text.empty()) return;

    std::size_t current_line = cursor.getLine();
    std::size_t current_col = cursor.getCol();

    if (cursor.getCol() > 0) {
	buffer->eraseCharAt(current_line, current_col - 1);
	cursor.retreatCol();
    } else if (current_line > 0) {
	std::size_t prev_line = current_line - 1;
        std::size_t prev_line_size = text[prev_line].first;
	
	buffer->appendLineTo(prev_line, current_line);
	buffer->removeLine(current_line);
	cursor.setPosition(prev_line, prev_line_size);
    }

    recalculateCursor(root_tree, cursor.getLine(), cursor.getCol());
}

void Window::newlineOnCursor()
{
    buffer->splitLineAt(cursor.getLine(), cursor.getCol());
    cursor.setPosition(cursor.getLine() + 1, 0);

    scrollToCursor();
}

void Window::insertChar(char c)
{
    buffer->insertCharAt(cursor.getLine(), cursor.getCol(), c);
    cursor.advanceCol();
    scrollToCursor();
}

void Window::scrollToCursor()
{
    std::size_t cur_line = cursor.getLine();
    std::size_t cur_col = cursor.getCol();

    std::size_t padding = 5;
    
    if (cur_line < view_port.first_visible_line)
	view_port.first_visible_line = cur_line < padding ? 0 : cur_line - padding;

    // fix in scrollToCursor bug when padding not being adaptive to view_port.visible_lines, causing first_visible_line/col jump over its visible lines
    if (cur_line >= view_port.first_visible_line + view_port.visible_lines) {
	if (padding > view_port.visible_lines) padding = view_port.visible_lines - 1;
	view_port.first_visible_line = cur_line - view_port.visible_lines + padding;
    }

    if (cur_col < view_port.first_visible_col)
	view_port.first_visible_col = cur_col < padding ? 0 : cur_col - padding;

    if (cur_col >= view_port.first_visible_col + view_port.visible_cols) {
	if (padding > view_port.visible_cols) padding = view_port.visible_cols - 1;
	view_port.first_visible_col = cur_col - view_port.visible_cols + padding;
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
