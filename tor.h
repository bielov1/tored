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
    KIT_F2, // split_screen: show same buffer on two separate windows
    KIT_F5,
    __static_key_input_tag_count
};

constexpr bool operator==(KeyInputTag kit, int i) {
    return static_cast<std::underlying_type_t<KeyInputTag>>(kit) == i;
}

class Editor
{
public:
    static Editor& getInstance()
    {
	static Editor editor;
	return editor;
    }    

    void handleKeyAction(KeyInputTag key);
    std::shared_ptr<Window> createNewWindow(int window_width, int window_height);
    std::shared_ptr<Window> createNewWindow(std::shared_ptr<Window> other_window);
    void addGraphicsToWindow(std::shared_ptr<Window> window);
    void insertCharOnActiveWindow(char c);
    void moveCursorLeft();
    void moveCursorRight();
    void moveCursorUp();
    void moveCursorDown();

    void backspaceOnCursor();
    void newlineOnCursor();
    void scrollToCursor(const Cursor& cur, ViewPort& vp);
    void saveToFile(const std::string& file_path);
    
    void refreshScreen();
    void closeActiveWindow();
    void horizontalSplitScreen();
    // void saveToFile(const std::string& file_path);
    // void loadFromFile(const std::string& file_path);
    Font loadPNGDataAsFont(std::span<const unsigned char> data, int cols, int rows);
    
    std::shared_ptr<Window> getActiveWindow() { return active_window; }
    void setActiveWindow(std::shared_ptr<Window> new_active_window) { active_window = new_active_window; }
    
    Editor(const Editor&) = delete;
    Editor& operator=(const Editor&) = delete;
private:
    Editor();
    ~Editor();
    
    static const int ASCII_DISPLAY_LOW = 32;
    static const int ASCII_DISPLAY_HIGH = 127;

    static const int FONT_WIDTH = 128;
    static const int FONT_HEIGHT = 64;
    static const int FONT_COLS = 18;
    static const int FONT_ROWS = 7;
    static constexpr float FONT_SCALE = 2.0f;
    static const int FONT_CHAR_WIDTH = (FONT_WIDTH / FONT_COLS);
    static const int FONT_CHAR_HEIGHT = (FONT_HEIGHT / FONT_ROWS);
    static constexpr Color FONT_COLOR = WHITE;
    static const size_t BUFFER_CAP = 1024;
    
    std::vector<std::shared_ptr<Window>> open_windows;
    std::shared_ptr<Window> active_window;
    std::size_t max_scroll_line;
    std::size_t max_scroll_col;
    Font font;
};
