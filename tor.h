#pragma once

#include <print>
#include <cstddef>
#include <cassert>
#include <vector>
#include <string_view>

#include "raylib.h"
#define GRAPHICS_API_OPENGL_33
#include "GLFW/glfw3.h"
#include "rlgl.h"

#include "./la.h"

static const int SCREEN_WIDTH = 800;
static const int SCREEN_HEIGHT = 600;

struct Cursor
{
    std::size_t line_idx = 0;
    std::size_t col_idx  = 0;
};

using TextLine = std::pair<std::size_t, std::string>;
using Buffer = std::vector<TextLine>;
class Editor
{
public:
    static Editor& getInstance()
    {
	static Editor editor;
	return editor;
    }
    
    void insertTextOnCursor(const std::string& text, std::size_t text_size, int col_idx, int line_idx);
    void backspaceOnCursor();
    void newlineOnCursor();
    void handleKeyAction(int key);
    void moveCursorLeft();
    void moveCursorRight();
    void moveCursorUp();
    void moveCursorDown();
    
    const Buffer& getBuffer() const { return buffer; }
    Cursor& getCursor() { return cursor; }
    
    Editor(const Editor&) = delete;
    Editor& operator=(const Editor&) = delete;
private:
    Editor();
    ~Editor() = default;

    static const size_t BUFFER_CAP = 1024;    
    Buffer buffer;
    Cursor cursor;
};
