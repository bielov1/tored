#pragma once

#include <string>
#include <span>

#include "tor.h"

extern "C" {
    extern const unsigned char _binary_charmap_oldschool_white_png_start[];
    extern const unsigned char _binary_charmap_oldschool_white_png_end[];
}

struct Cursor;
struct Vec2f;
struct Color;

// Could use separate renderer for each buffer in further development
class Renderer
{
public:
    // May change in future
    static Renderer& getInstance()
    {
	static Renderer renderer;
	return renderer;
    }
    
    void renderChar(char c, Vec2f pos, Color color);
    void renderText(const Buffer& buffer, Vec2f start_pos, float char_w, float char_h, Color color);
    void renderCursor(const Cursor& cursor, const std::string& line_text, Vec2f start_pos, float char_w, float char_h, Color color);
    void renderScene(const Buffer& buffer, const Cursor& cursor, Vec2f start_pos);
    Font loadPNGDataAsFont(std::span<const unsigned char> data, int cols, int rows);

private:
    Renderer();
    ~Renderer();
    
    static const int ASCII_DISPLAY_LOW = 32;
    static const int ASCII_DISPLAY_HIGH = 127;

    static const int FONT_WIDTH = 128;
    static const int FONT_HEIGHT = 64;
    static const int FONT_COLS = 18;
    static const int FONT_ROWS = 7;
    static const int FONT_CHAR_WIDTH = (FONT_WIDTH / FONT_COLS);
    static const int FONT_CHAR_HEIGHT = (FONT_HEIGHT / FONT_ROWS);
    static constexpr float FONT_SCALE = 2.0f;
    static constexpr Color FONT_COLOR = WHITE;
    
    Font font;
};
