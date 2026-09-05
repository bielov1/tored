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
    
    Rectangle rec = {
	.x      = cursor_pixel_pos.x,
	.y      = cursor_pixel_pos.y,
	.width  = static_cast<float>(char_width),
	.height = static_cast<float>(char_height)
    };
    
    DrawRectangleRec(rec, WHITE);

    if (cursor->getLine() < text_size) {
	const auto& [line_size, line_text] = text[cursor->getLine()];
	if (cursor->getCol() < line_size) {
	    char c = line_text[cursor->getCol()];
	    if (c != '\n')
		drawChar(font, c, char_width, char_height, cursor_pixel_pos, BLACK);
	}
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
