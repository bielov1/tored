#include "graphic.h"

void BufferView::draw(const Font& font, int char_width, int char_height)
{
    if (buffer->empty()) return;
        
    // draw text
    Vec2f start_pos = { window_rect->x, window_rect->y };
    Vec2f draw_char_pos = start_pos;

    auto drawChar = [&](char c, Vec2f& pos, Color color) -> void {
	int idx = GetGlyphIndex(font, c);
	if (idx >= 0 && idx < font.glyphCount) {
	    Rectangle src = font.recs[idx];
	    Rectangle dst = {
		pos.x,
		pos.y,
		static_cast<float>(char_width),
		static_cast<float>(char_height)
	    };
		
	    DrawTexturePro(font.texture, src, dst, Vector2{ 0.0f, 0.0f }, 0.0f, color);
	}
    };
    
    std::size_t i;
    std::size_t line_count = buffer->getLineCount();
    std::size_t end_line = view_port->first_visible_line + view_port->visible_lines;
    for (i = view_port->first_visible_line; i < line_count && i < end_line; ++i) {
	auto line_slices = buffer->getLine(i, *overlay_buffer);	
	for (const auto& slice : line_slices) {
	    for (const auto& c : slice) {
		if (c == '\n') continue;
		drawChar(c, draw_char_pos, WHITE);		
		if (draw_char_pos.x + char_width >= view_port->visible_cols * char_width) {
		    draw_char_pos.x = start_pos.x;
		    draw_char_pos.y += char_height;
		} else {
		    draw_char_pos.x += char_width;
		}
	    }
	}
	
	draw_char_pos.x  = start_pos.x;
	draw_char_pos.y += char_height;
    }
   
    // draw cursor
    if (view_port->visible_cols > 0) {
	LineCol base = buffer->offsetToLineCol(cursor->getOffset());
	
	std::size_t total_cols = base.col + overlay_buffer->text.size();
	std::size_t wrapped_rows = total_cols / view_port->visible_cols;
	std::size_t final_col = total_cols % view_port->visible_cols;
	std::size_t absolute_visual_line = base.line + wrapped_rows;

	Vec2f cursor_draw_pos = {
	    start_pos.x + static_cast<float>(final_col * char_width),
	    start_pos.y + static_cast<float>(absolute_visual_line * char_height)
	};
    
	Rectangle cursor_rec = {
	    .x      = cursor_draw_pos.x,
	    .y      = cursor_draw_pos.y,
	    .width  = static_cast<float>(char_width),
	    .height = static_cast<float>(char_height)
	};

	DrawRectangleRec(cursor_rec, WHITE);
    
	char c = buffer->getCharAt(cursor->getOffset());
	if (c != '\n' && c != '\0') {
	    drawChar(c, cursor_draw_pos, BLACK);
	}
    }
}

void Window::dumpOverlayBuffer()
{
    if (overlay_buffer.empty()) return;

    buffer->insert(overlay_buffer.start_offset, overlay_buffer.text);
    cursor.setOffset(overlay_buffer.start_offset + overlay_buffer.text.size());
    overlay_buffer.clear();
}

void Window::handleCharInput(char c)
{
    if (overlay_buffer.empty()) {
	overlay_buffer.start_offset = cursor.getOffset();
    }

    overlay_buffer.append(c);
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
    // if (getBufferText().empty()) return;    
    // if (getCursorCol() > 0) {
    // 	getCursor().retreatCol();
    // } else if (getCursorLine() > 0) {
    // 	std::size_t prev_line_size = getBufferText()[getCursorLine() - 1].first;
    // 	getCursor().setPosition(getCursorLine() - 1, prev_line_size);
    // }
    // scrollToCursor();
}

void Window::moveCursorRight()
{
    // if (getBufferText().empty()) return;
    // std::size_t line_size = getBufferText()[getCursorLine()].first;
    // if (getCursorCol() < line_size) {
    //     getCursor().advanceCol();
    // } else if (getCursorLine() + 1 < getBufferText().size()) {
    //     getCursor().setPosition(getCursorLine() + 1, 0);
    // }
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
    // getBuffer().splitLineAt(getCursorLine(), getCursorCol());
    // getCursor().setPosition(getCursorLine() + 1, 0);
    // scrollToCursor();
    // recalculateCursor(root_tree, getCursorLine(), getCursorCol());
}

void Window::scrollToCursor()
{
    // std::size_t padding = 5;   
    // if (getCursorLine() < view_port.first_visible_line)
    // 	view_port.first_visible_line = getCursorLine() < padding ? 0 : getCursorLine() - padding;

    // if (getCursorLine() >= view_port.first_visible_line + view_port.visible_lines) {
    // 	if (padding > view_port.visible_lines) padding = view_port.visible_lines - 1;
    // 	view_port.first_visible_line = getCursorLine() - view_port.visible_lines + padding;
    // }

    // if (getCursorCol() < view_port.first_visible_col)
    // 	view_port.first_visible_col = getCursorCol() < padding ? 0 : getCursorCol() - padding;

    // if (getCursorCol() >= view_port.first_visible_col + view_port.visible_cols) {
    // 	if (padding > view_port.visible_cols) padding = view_port.visible_cols - 1;
    // 	view_port.first_visible_col = getCursorCol() - view_port.visible_cols + padding;
    // }
}

void Window::recalcViewPort(int char_width, int char_height)
{
    view_port.visible_cols = static_cast<std::size_t>(rect.width / char_width);
    view_port.visible_lines = static_cast<std::size_t>(rect.height / char_height);
}

void Window::attachBufferView()
{
    auto view = std::make_shared<BufferView>(&rect, &view_port, &cursor, &overlay_buffer, buffer.get());
    add(view);
}
