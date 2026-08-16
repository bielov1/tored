#include "tor.h"

char buffer[BUFFER_CAP + 1];
int buffer_size = 0;
int buffer_cursor_row = 0;
int buffer_cursor_col = 0;

auto getCursorX = [buffer_view = std::span{buffer}](){
    int c = 0;
    while (buffer_size - c > 0 && buffer_view[buffer_size - c - 1] != '\n') c++;
    return c;
 };

void handleKeyAction(int key)
{
    switch (key) {
    case GLFW_KEY_BACKSPACE:
	buffer_size--;
	if (buffer_size < 0) buffer_size = 0;
	if (buffer[buffer_size] == '\n') {
	    buffer_cursor_row--;
	    buffer_cursor_col = getCursorX();
	} else {
	    buffer_cursor_col--;
	    if (buffer_cursor_col < 0) buffer_cursor_col = 0;
	}
	    
	buffer[buffer_size] = '\0';
	break;
    case GLFW_KEY_ENTER:
	if (buffer_size < BUFFER_CAP) {
	    buffer[buffer_size++] = '\n';
	    buffer_cursor_row++;
	    buffer_cursor_col = 0;
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
}

void charCallback(GLFWwindow* window, unsigned int codepoint)
{
    (void)window;
    if ((codepoint >= 32) && (codepoint <= 127) && (buffer_size < BUFFER_CAP)) {
	buffer[buffer_size++] = codepoint;
	buffer_cursor_col++;
	buffer[buffer_size] = '\0';
    }
}

void renderChar(Font font, char c, Vec2f pos, int scale, Color color)
{
    DrawTextCodepoint(font, (int)c, Vector2{pos.x, pos.y}, (float)font.baseSize * scale, color);
}

void renderText(Font font, const std::string &text, Vec2f pos, int scale, Color color)
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

void renderCursor(Color color, int scale)
{
    int x = buffer_cursor_col * FONT_CHAR_WIDTH * scale;
    int y = buffer_cursor_row * FONT_CHAR_HEIGHT * scale;
    DrawRectangle(x, y, FONT_CHAR_WIDTH * FONT_SCALE, FONT_CHAR_HEIGHT * FONT_SCALE, color);
}

Font loadPNGDataAsFont(std::span<const unsigned char> data, int cols, int rows, int start_codepoint = 32)
{
    Font font{};
    Image image = LoadImageFromMemory(".png", data.data(), data.size());
    if (!IsImageValid(image)) {
	std::fprintf(stderr, "[ERROR] could not load image from memory\n");
	exit(1);
    }
    
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
	    renderCursor(WHITE, FONT_SCALE);
        EndDrawing();
    }
    
    UnloadFont(font);
    CloseWindow();

    return 0;
}
