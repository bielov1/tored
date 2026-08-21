#include "renderer.h"

Renderer::Renderer()
    : font{}
{
    size_t font_size = _binary_charmap_oldschool_white_png_end - _binary_charmap_oldschool_white_png_start;
    unsigned char *font_data = (unsigned char *)_binary_charmap_oldschool_white_png_start;
    std::span<const unsigned char> data_view{font_data, font_size};
    font = loadPNGDataAsFont(data_view, FONT_COLS, FONT_ROWS);
}

Renderer::~Renderer()
{
    UnloadFont(font);
}

void Renderer::renderChar(char c, Vec2f pos, Color color = FONT_COLOR)
{
    int idx = GetGlyphIndex(font, (int)c);
    if (idx >= 0 && idx < font.glyphCount) {
	Rectangle src = font.recs[idx];
	Rectangle dst = {
	    pos.x,
	    pos.y,
	    src.width  * FONT_SCALE,
	    src.height * FONT_SCALE
	};
	DrawTexturePro(font.texture, src, dst, Vector2{ 0.0f, 0.0 }, 0.0f, color);
    }    
}

void Renderer::renderText(const Buffer& buffer, Vec2f start_pos, float char_w, float char_h, Color color = FONT_COLOR)
{    
    Vec2f curr_pos = start_pos;
    int buffer_size = buffer.size();
    for (int i = 0; i < buffer_size; ++i) {
	const auto& [line_size, line_text] = buffer[i];
	for (char c : line_text) {
	    renderChar(c, curr_pos, color);
	    curr_pos.x += char_w;
	}
	
	if (i + 1 < buffer_size) {
	    curr_pos.y += char_h;
	    curr_pos.x = start_pos.x;
	}
    }
}

void Renderer::renderCursor(const Cursor& cursor, const std::string& line_text, Vec2f start_pos, float char_w, float char_h, Color color = FONT_COLOR)
{
    Vec2f cursor_pixel_pos{
	start_pos.x + (static_cast<float>(cursor.col_idx)  * char_w),
	start_pos.y + (static_cast<float>(cursor.line_idx) * char_h)
    };
    
    Rectangle rec = {
	.x      = cursor_pixel_pos.x,
	.y      = cursor_pixel_pos.y,
	.width  = char_w,
	.height = char_h
    };
    
    DrawRectangleRec(rec, color);

    // render character on top of cursor if cursor covers any
    if (cursor.col_idx < line_text.size()) {
        char c = line_text[cursor.col_idx];
        if (c != '\n') {
            renderChar(c, cursor_pixel_pos, BLACK);
        }
    }
}

void Renderer::renderScene(const Buffer& buffer, const Cursor& cursor, Vec2f start_pos)
{
    const float char_height = FONT_CHAR_HEIGHT * FONT_SCALE;
    const float char_width = FONT_CHAR_WIDTH * FONT_SCALE;

    //render text in buffer
    if (!buffer.empty()) 
	renderText(buffer, start_pos, char_width, char_height);

    std::string current_line = "";
    if (cursor.line_idx < buffer.size()) {
        current_line = buffer[cursor.line_idx].second;
    }
    renderCursor(cursor, current_line, start_pos, char_width, char_height);
}

Font Renderer::loadPNGDataAsFont(std::span<const unsigned char> data, int cols, int rows)
{
    Font font{};
    Image image = LoadImageFromMemory(".png", data.data(), data.size());
    if (!IsImageValid(image)) {
	std::fprintf(stderr, "[ERROR] could not load image from memory\n");
	exit(1);
    }
    
    ImageFormat(&image, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    ImageColorReplace(&image, BLACK, BLANK);
    
    font.baseSize = image.height / rows;
    font.glyphCount = cols * rows;
    font.glyphPadding = 0;
    font.texture = LoadTextureFromImage(image);
    
    font.recs = (Rectangle*)RL_MALLOC(font.glyphCount * sizeof(Rectangle));
    font.glyphs = (GlyphInfo*)RL_MALLOC(font.glyphCount * sizeof(GlyphInfo));
    
    for (int i = 0; i < font.glyphCount; ++i) {
	int col = i % cols;
	int row = i / cols;

	Rectangle rec = {
	    static_cast<float>(col * FONT_CHAR_WIDTH),
	    static_cast<float>(row * FONT_CHAR_HEIGHT),
	    static_cast<float>(FONT_CHAR_WIDTH),
	    static_cast<float>(FONT_CHAR_HEIGHT)
	};
	
	font.glyphs[i].value = ASCII_DISPLAY_LOW + i;
	font.glyphs[i].offsetX = 0;
	font.glyphs[i].offsetY = 0;
	font.glyphs[i].advanceX = FONT_CHAR_WIDTH;
	font.glyphs[i].image = ImageFromImage(image, rec);
	font.recs[i] = rec;
    }

    if (!IsFontValid(font)) {
	std::fprintf(stderr, "font is invalid.\n");
	exit(1);
    }

    
    UnloadImage(image);
    return font;
}
