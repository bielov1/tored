#include "tor.h"

char buffer[BUFFER_CAP + 1];
int buffer_size = 0;
int buffer_cursor = 0;

int cursor_offset = 0;
int cursor_last_line = 0;
int cursor_line = 0;

auto calcCursorOffsetForPrevLine = [buffer_view = std::span{buffer}](int cursor){
    int k = 0;
    while (cursor - k > 0 && buffer_view[cursor - k - 1] != '\n') k++;
    return k;
 };

void handleKeyAction(int key)
{
    switch (key) {
    case GLFW_KEY_BACKSPACE:
	if (buffer_size > 0) {
	    buffer_size--;
	    buffer_cursor = buffer_size;
	    if (buffer[buffer_size] == '\n') {
		cursor_line--;
		cursor_last_line = cursor_line;
	    } else {
		cursor_line = cursor_last_line;
	    }
	    
	    cursor_offset = calcCursorOffsetForPrevLine(buffer_cursor);
	    buffer[buffer_size] = '\0';
	}
	break;
    case GLFW_KEY_ENTER:
	if (buffer_size < BUFFER_CAP) {
	    buffer[buffer_size++] = '\n';
	    buffer_cursor = buffer_size;
	    cursor_line = cursor_last_line + 1;
	    if (cursor_last_line < cursor_line) cursor_last_line = cursor_line;
	    
	    cursor_offset = 0;
	    buffer[buffer_size] = '\0';
	}
	break;
    case GLFW_KEY_LEFT:
	if (buffer_cursor > 0) {
	    buffer_cursor--;

	    if (buffer[buffer_cursor] == '\n') {
		cursor_line--;
		cursor_offset = calcCursorOffsetForPrevLine(buffer_cursor);
	    } else {
		cursor_offset--;
	    }

	}
	break;
    case GLFW_KEY_RIGHT:
	if (buffer_cursor < buffer_size) {
	    if (buffer[buffer_cursor] == '\n') {
		cursor_line++;
		cursor_offset = 0;
	    } else {
		cursor_offset++;
	    }
	    buffer_cursor++;
	}
	break;
    default:
	std::fprintf(stderr, "[WARNING] uknown key input\n");
    }
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    (void)window;
    (void)scancode;
    (void)mods;
    if (key == GLFW_KEY_BACKSPACE && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
	handleKeyAction(GLFW_KEY_BACKSPACE);
    }
    if (key == GLFW_KEY_ENTER && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        handleKeyAction(GLFW_KEY_ENTER);
    }
    if (key == GLFW_KEY_LEFT && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        handleKeyAction(GLFW_KEY_LEFT);
    }
    if (key == GLFW_KEY_RIGHT && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        handleKeyAction(GLFW_KEY_RIGHT);
    }
    
}

void charCallback(GLFWwindow* window, unsigned int codepoint)
{
    (void)window;
    if ((codepoint >= 32) && (codepoint <= 127) && (buffer_size < BUFFER_CAP)) {
	buffer[buffer_size++] = codepoint;
	
	if (buffer_cursor + 1 < buffer_size) {
	    buffer_cursor = buffer_size;
	    cursor_line = cursor_last_line;
	    cursor_offset = calcCursorOffsetForPrevLine(buffer_cursor);
	} else {
	    buffer_cursor++;
	    cursor_offset++;
	}
	buffer[buffer_size] = '\0';
    }
}

void renderChar(const Font& font, char c, Vec2f pos, int scale, Color color)
{
    int idx = GetGlyphIndex(font, (int)c);
    if (idx > 0) {
	Rectangle src = font.recs[idx];
	Rectangle dst = { pos.x, pos.y, src.width * scale, src.height * scale };
	DrawTexturePro(font.texture, src, dst, Vector2{0.0f, 0.0}, 0.0f, color);
    }
}

void renderText(const Font& font, const std::string &text, Vec2f pos, int scale, Color color)
{
    int start_x = pos.x;
    for (char c : text) {
	if (c == '\n') {
	    pos.y += FONT_CHAR_HEIGHT * scale;
	    pos.x = start_x;
	    continue;
	}
	
        renderChar(font, c, pos, scale, color);
        pos.x += FONT_CHAR_WIDTH * scale;
    }
}

void renderCursor(const Font& font, int scale)
{
    const Vec2f pos {(float)(cursor_offset * FONT_CHAR_WIDTH * scale),
		     (float)(cursor_line * FONT_CHAR_HEIGHT * scale)};
    Rectangle rec = {
	.x = pos.x,
	.y = pos.y,
	.width = (float)(FONT_CHAR_WIDTH * scale),
	.height = (float)(FONT_CHAR_HEIGHT * scale)
    };
    DrawRectangleRec(rec, WHITE);

    if (buffer_cursor < buffer_size && buffer[buffer_cursor] != '\n') {
        renderChar(font, buffer[buffer_cursor], pos, scale, BLACK);
    }
}

Font loadPNGDataAsFont(std::span<const unsigned char> data, int cols, int rows, int start_codepoint = 32)
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

    auto genRectangle = [](int row, int col, int char_width, int char_height) {
	return Rectangle{ (float)(col * char_width), (float)(row * char_height), (float)char_width, (float)char_height };
    };
    
    for (int i = 0; i < font.glyphCount; ++i) {
	int col = i % cols;
	int row = i / cols;

	Rectangle rec = genRectangle(row, col, FONT_CHAR_WIDTH, FONT_CHAR_HEIGHT);
	
	font.glyphs[i].value = start_codepoint + i;
	font.glyphs[i].offsetX = 0;
	font.glyphs[i].offsetY = 0;
	font.glyphs[i].advanceX = FONT_CHAR_WIDTH;
	font.glyphs[i].image = ImageFromImage(image, rec);
	font.recs[i] = rec;
    }

    UnloadImage(image);
    return font;
}

int main()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "");
   
    GLFWwindow *ctx = glfwGetCurrentContext(); 
    glfwSetKeyCallback(ctx, keyCallback);
    glfwSetCharCallback(ctx, charCallback);

    size_t font_size = _binary_charmap_oldschool_white_png_end - _binary_charmap_oldschool_white_png_start;
    unsigned char *font_data = (unsigned char *)_binary_charmap_oldschool_white_png_start;
    
    std::span<const unsigned char> data_view{font_data, font_size};
    Font font = loadPNGDataAsFont(data_view, FONT_COLS, FONT_ROWS, 32);
   
    SetTargetFPS(60);
    
    while (!WindowShouldClose()) {
	BeginDrawing();
	ClearBackground(Color{ 0x18, 0x18, 0x18, 0x0 });
	renderText(font, std::string{buffer}, Vec2f{ 0.0f, 0.0f }, FONT_SCALE, WHITE);
	renderCursor(font, FONT_SCALE);
        EndDrawing();
    }
    
    UnloadFont(font);
    CloseWindow();

    return 0;
}
