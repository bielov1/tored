#include "graphic.h"

void BufferView::drawChar(Font font, char c, Vec2f pos, int scale, Color color)
{
    int idx = GetGlyphIndex(font, (int)c);
    if (idx >= 0 && idx < font.glyphCount) {
	Rectangle src = font.recs[idx];
	Rectangle dst = {
	    pos.x,
	    pos.y,
	    src.width * scale,
	    src.height * scale
	};
	DrawTexturePro(font.texture, src, dst, Vector2{ 0.0f, 0.0 }, 0.0f, color);
    }    
}

void BufferView::draw(Font font, int scale)
{
    assert(font.recs);
    assert(font.glyphCount > 0);
   
    const auto& text = buffer->getText();
    
    if (text.empty()) return;

    std::size_t i;
    std::size_t j;
    
    std::size_t text_size = text.size();    
    float char_w = font.recs[0].width * scale;
    float char_h = font.recs[0].height * scale;

    Vec2f start_pos = { window_rect->x, window_rect->y };
    Vec2f draw_char_pos = start_pos;
    
    // render text on screen
    std::size_t end_line = view_port->first_visible_line + view_port->visible_lines;
    for (i = view_port->first_visible_line; i < end_line && i < text_size; ++i) {
	const auto& [line_size, line_text] = text[i];
	
	std::size_t col_idx = 0;
	for (j = view_port->first_visible_col; j < line_size; ++j) {
            if (col_idx >= view_port->visible_cols) break;

            drawChar(font, line_text[j], draw_char_pos, scale, WHITE);
            draw_char_pos.x += char_w;
            col_idx++;
        }

        draw_char_pos.y += char_h;
        draw_char_pos.x  = start_pos.x;
    }

    // render cursor
    float rel_col = static_cast<float>(cursor->getCol() - view_port->first_visible_col);
    float rel_line = static_cast<float>(cursor->getLine() - view_port->first_visible_line);
   
    Vec2f cursor_pixel_pos{
	start_pos.x + rel_col * char_w,
	start_pos.y + rel_line * char_h
    };
    
    Rectangle rec = {
	.x      = cursor_pixel_pos.x,
	.y      = cursor_pixel_pos.y,
	.width  = char_w,
	.height = char_h
    };
    
    DrawRectangleRec(rec, WHITE);

    if (cursor->getLine() < text_size) {
	const auto& [line_size, line_text] = text[cursor->getLine()];
	if (cursor->getCol() < line_size) {
	    char c = line_text[cursor->getCol()];
	    if (c != '\n')
		drawChar(font, c, cursor_pixel_pos, scale, BLACK);
	}
    }
}


void Window::recalcViewPort(int char_width, int char_height)
{
    view_port.visible_cols = static_cast<std::size_t>(rect.width / char_width);
    view_port.visible_lines = static_cast<std::size_t>(rect.height / char_height);
}
