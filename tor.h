#pragma once

#include <print>
#include <cstdio>
#include <fstream>
#include <iostream>

#include <string_view>
#include <ranges>
#include <algorithm>

#define GRAPHICS_API_OPENGL_33
#include "GLFW/glfw3.h"

#include "graphic.h"

extern "C" {
    extern const unsigned char _binary_charmap_oldschool_white_png_start[];
    extern const unsigned char _binary_charmap_oldschool_white_png_end[];
}

static const int DEFAULT_SCREEN_WIDTH = 800;
static const int DEFAULT_SCREEN_HEIGHT = 600;

static const int ASCII_DISPLAY_LOW = 32;
static const int ASCII_DISPLAY_HIGH = 127;

static const int FONT_WIDTH = 128;
static const int FONT_HEIGHT = 64;
static const int FONT_COLS = 18;
static const int FONT_ROWS = 7;
static const int  FONT_SCALE = 2;
static const int FONT_CHAR_WIDTH = (FONT_WIDTH / FONT_COLS);
static const int FONT_CHAR_HEIGHT = (FONT_HEIGHT / FONT_ROWS);
static constexpr Color FONT_COLOR = WHITE;
static const size_t BUFFER_CAP = 1024;

enum class KeyInputTag : int
{
    KIT_BACKSPACE,
    KIT_ENTER,
    KIT_LEFT,
    KIT_RIGHT,
    KIT_UP,
    KIT_DOWN,
    KIT_F2,
    KIT_F3,
    KIT_F5,
    __static_key_input_tag_count
};

constexpr bool operator==(KeyInputTag kit, int i) {
    return static_cast<std::underlying_type_t<KeyInputTag>>(kit) == i;
}

struct RenderVisitor
{
    Font font;

    void operator()(const Leaf& leaf) const {
        if (leaf.window) {
            leaf.window->draw(font, leaf.char_width, leaf.char_height);
        }
    }

    void operator()(const std::unique_ptr<Node>& node) const {
        if (node) {
            std::visit(*this, node->left);
            std::visit(*this, node->right);
        }
    }
};

class Editor
{
public:
    static Editor& getInstance()
    {
	static Editor editor;
	return editor;
    }    

    void handleKeyAction(KeyInputTag key);
    void insertCharOnActiveWindow(char c);
    void moveCursorLeft();
    void moveCursorRight();
    void moveCursorUp();
    void moveCursorDown();

    void backspaceOnCursor();
    void newlineOnCursor();
    void scrollToCursor(const Cursor& cur, ViewPort& vp);
   
    void splitActiveWindow(SplitType sp);
    void onResize(int new_window_width, int new_window_height);
    void refreshScreen();
    void closeActiveWindow();
    void saveToFile(const std::string& file_path);
    // void loadFromFile(const std::string& file_path);
    Font loadPNGDataAsFont(std::span<const unsigned char> data, int cols, int rows);
    
    std::shared_ptr<Window> getActiveWindow() { return active_window; }
    void setActiveWindow(std::shared_ptr<Window> new_active_window) { active_window = new_active_window; }
    
    Editor(const Editor&) = delete;
    Editor& operator=(const Editor&) = delete;
private:
    Editor();
    ~Editor();

    std::shared_ptr<Window> active_window;
    LayoutTree root_tree;
    std::size_t max_scroll_line;
    std::size_t max_scroll_col;
    int screen_width;
    int screen_height;
    Font font;
};

static std::shared_ptr<Window> createNewWindow(int window_width, int window_height)
{
    Rectangle new_rect = {
	.x = 0.0f,
	.y = 0.0f,
	.width = static_cast<float>(window_width),
	.height = static_cast<float>(window_height)
    };
    
    ViewPort new_view_port = {
	.first_visible_line = 0,
	.first_visible_col  = 0,
	.visible_lines = static_cast<std::size_t>(window_height / (FONT_CHAR_HEIGHT * FONT_SCALE)),
	.visible_cols  = static_cast<std::size_t>(window_width / (FONT_CHAR_WIDTH * FONT_SCALE))
    };
    
    Cursor new_cursor{};
    auto new_buffer = std::make_shared<Buffer>();
    
    auto new_window = std::make_shared<Window>(
        new_rect,
        new_view_port,
        new_cursor,
	std::move(new_buffer)
    );

    new_window->attachBufferView();
    return new_window;
}
