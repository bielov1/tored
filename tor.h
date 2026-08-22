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
    int line_idx = 0;
    int col_idx  = 0;
};

using TextLine = std::pair<size_t, std::string>;
using Buffer = std::vector<TextLine>;
class Editor
{
public:
    static Editor& getInstance()
    {
	static Editor editor;
	return editor;
    }
    
    void insertTextOnCursor(std::string_view text);
    void backspace();
    void createNewline();
    void handleKeyAction(int key);
    void cursorMoveLeft();
    void cursorMoveRight();
    
    const Buffer& getBuffer() const { return buffer; }
    const Cursor& getCursor() const { return cursor; }
    
    Editor(const Editor&) = delete;
    Editor& operator=(const Editor&) = delete;
private:
    Editor();
    ~Editor() = default;

    static const size_t BUFFER_CAP = 1024;    
    Buffer buffer;
    Cursor cursor;
};
