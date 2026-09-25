#include "graphic.h"

template<typename DrawCharFunc>
void BufferView::drawOverlay(Vec2f& pos, DrawCharFunc&& drawChar)
{
    for (const auto& c : overlay_buf->text) {
	drawChar(c, pos, WHITE);
    }
}

void BufferView::draw(const Font& font, const CharParams params)
{
    if (buffer->empty()) return;
    assert(overlay_buf);

    Vec2f start_pos = { window_rect->x, window_rect->y };
    Vec2f draw_char_pos = start_pos;
    
    // draw text
    auto drawChar = [&](char c, Vec2f& pos, Color color) {
	if (c == '\n') {
	    pos.x  = start_pos.x;
	    pos.y += params.height;
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
	    
	    if (pos.x + params.width >= start_pos.x + (view_port->visible_cols * params.width)) {
		pos.x = start_pos.x;
		pos.y += params.height;
	    } else {
		pos.x += params.width;
	    } 
	}
    };

    std::size_t line_count = buffer->getLineCount();
    std::size_t last_visible_line = view_port->first_visible_line + view_port->visible_lines;
    std::size_t end_line = last_visible_line > 0 ? last_visible_line : 1;

    std::size_t offset_counter = buffer->getLineStart(view_port->first_visible_line);
    for (std::size_t i = view_port->first_visible_line; i < line_count && i < end_line; ++i) {
	auto line_slices = buffer->getLineSlices(i);
	if (line_slices.empty()) break;
	
	for (const auto& slice : line_slices) {
	    for (const auto& c : slice) {
		if (!overlay_buf->empty() && offset_counter == overlay_buf->start_offset) {
		    drawOverlay(draw_char_pos, drawChar);
		}
		
		drawChar(c, draw_char_pos, WHITE);
		offset_counter++;
	    }
	}
    }

    // overlay buffer at the end of the text
    if (!overlay_buf->empty() && offset_counter == overlay_buf->start_offset) {
	drawOverlay(draw_char_pos, drawChar);
    }
    
    // draw cursor
    LineCol base = buffer->offsetToLineCol(cursor->getOffset());

    std::size_t visible_cols = view_port->visible_cols > 0 ? view_port->visible_cols : 1;
    std::size_t total_cols   = base.col + overlay_buf->text.size();
    std::size_t wrapped_rows = total_cols / visible_cols;
    std::size_t final_col    = total_cols % visible_cols;
    std::size_t absolute_visual_line = base.line + wrapped_rows;

    if (absolute_visual_line >= view_port->first_visible_line) {
        std::size_t screen_line = absolute_visual_line - view_port->first_visible_line;

        Vec2f cursor_draw_pos = {
            start_pos.x + final_col * params.width,
            start_pos.y + screen_line * params.height
        };
        
        Rectangle cursor_rec = {
            .x      = cursor_draw_pos.x,
            .y      = cursor_draw_pos.y,
            .width  = params.width,
            .height = params.height
        };

        DrawRectangleRec(cursor_rec, WHITE);
        
        char c = buffer->getCharAt(cursor->getOffset());
        if (c != '\n' && c != '\0') {
            Vec2f temp_pos = cursor_draw_pos;
            drawChar(c, temp_pos, BLACK);
        }
    }
}

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
	// backspace();
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
    case KeyInputTag::KIT_F1:
	//switchActiveWindow();
	break;    
    case KeyInputTag::KIT_F2:
	//splitActiveWindow(SplitType::Horizontal);
	break;
    case KeyInputTag::KIT_F3:
	//splitActiveWindow(SplitType::Vertical);
	break;
    case KeyInputTag::KIT_F4:
	//closeAndSwitchActiveWindow();
	break;
	// case GLFW_KEY_F5:
	// 	std::fprintf(stdout, "F5 was pressed\n");
	// 	saveToFile(std::string{"output"});
	// 	break;
    default:
	std::fprintf(stderr, "[WARNING] uknown key input\n");
    }
    scrollToCursor();
}

// void Window::closeAndSwitchActiveWindow()
// {
//     // TODO
//     // if (auto result = std::ranges::find(window_list, active_window) != window_list.end()) {
//     // 	window_list.remove(active_window);
//     // }   
// }

void Window::moveCursorLeft()
{
    if (buffer->empty()) return;
    if (cursor.getOffset() > 0) {
	cursor.retreatOffset();
    }
    //scrollToCursor();
}

void Window::moveCursorRight()
{
    if (buffer->empty()) return;
    if (cursor.getOffset() < buffer->getTotalLength()) {
	cursor.advanceOffset();
    }
    // scrollToCursor();
}

void Window::moveCursorUp()
{
    // if (getBufferText().empty()) return;
    // if (getCursorLine() > 0) {
    // 	std::size_t prev_line_size = getBufferText()[getCursorLine() - 1].first;
    // 	std::size_t new_col = std::ranges::clamp(getCursorCol(), std::size_t{0}, prev_line_size);
    // 	getCursor().setPosition(getCursorLine() - 1, new_col);
    // }
    // scrollToCursor();
}

void Window::moveCursorDown()
{    
    // if (getBufferText().empty()) return;
    // if (getCursorLine() + 1 < getBufferText().size()) {
    // 	std::size_t next_line_size = getBufferText()[getCursorLine() + 1].first;
    // 	std::size_t new_col = std::ranges::clamp(getCursorCol(), std::size_t{0}, next_line_size);
    // 	getCursor().setPosition(getCursorLine() + 1, new_col);
    // }
    // scrollToCursor();
}

void Window::backspaceOnCursor(LayoutTree& root_tree)
{
    // if (getBufferText().empty()) return;
    // if (getCursorCol() > 0) {
    // 	getBuffer().eraseCharAt(getCursorLine(), getCursorCol() - 1);
    // 	getCursor().retreatCol();
    // } else if (getCursorLine() > 0) {
    //     std::size_t prev_line_size = getBufferText()[getCursorLine() - 1].first;
    // 	getBuffer().appendLineTo(getCursorLine() - 1, getCursorLine());
    // 	getBuffer().removeLine(getCursorLine());
    // 	getCursor().setPosition(getCursorLine() - 1, prev_line_size);
    // }
    // recalculateCursor(root_tree, getCursorOffset(), getCursorOffset() - 1);
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
    }
    
    else if (cursor_lc.line < view_port.first_visible_line) {
        view_port.first_visible_line = cursor_lc.line;
    }
}

void Window::recalcViewPort(int char_width, int char_height)
{
    view_port.visible_cols = static_cast<std::size_t>(rect.width / char_width);
    view_port.visible_lines = static_cast<std::size_t>(rect.height / char_height);
}

void Window::attachBufferView()
{
    auto view = std::make_shared<BufferView>(&rect, &view_port, &cursor, &overlay_buf, buffer.get());
    add(view);
}
