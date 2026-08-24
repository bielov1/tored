#pragma once

#include <print>
#include <cstdio>
#include <cstddef>
#include <cassert>
#include <vector>
#include <fstream>
#include <ios>
#include <string_view>
#include <ranges>
#include <algorithm>

#include "raylib.h"
#define GRAPHICS_API_OPENGL_33
#include "GLFW/glfw3.h"
#include "rlgl.h"

#include "./la.h"

static const int SCREEN_WIDTH = 800;
static const int SCREEN_HEIGHT = 600;

enum class KeyInputTag : int
{
    KIT_BACKSPACE,
    KIT_ENTER,
    KIT_LEFT,
    KIT_RIGHT,
    KIT_UP,
    KIT_DOWN,
    KIT_F5,
    __static_key_input_tag_count
};

constexpr bool operator==(KeyInputTag kit, int i) {
    return static_cast<std::underlying_type_t<KeyInputTag>>(kit) == i;
}

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
    void handleKeyAction(KeyInputTag key);
    void moveCursorLeft();
    void moveCursorRight();
    void moveCursorUp();
    void moveCursorDown();

    void saveToFile(const std::string& file_path);
    void loadFromFile(const std::string& file_path);
    
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
