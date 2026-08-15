#include "tor.h"

void renderChar(Font font, char c, Vec2f pos, int scale, Color color)
{
    DrawTextCodepoint(font, (int)c, Vector2{ pos.x, pos.y }, (float)font.baseSize * scale, color);
}

void renderText(Font font, const std::string &text, Vec2f pos, int scale, Color color)
{
    for (char c : text) {
	if (c == '\n') {
	    pos.y += FONT_CHAR_HEIGHT * scale;
	    pos.x = 0;
	    continue;
	}
	
        renderChar(font, c, pos, scale, color);
        pos.x += FONT_CHAR_WIDTH * scale;
    }
}

char buffer[BUFFER_CAP + 1];
int buffer_size = 0;
int buffer_cursor_row = 0;
int buffer_cursor_col = 0;

auto getCursorX = [buffer_view = std::span{buffer}](){
    int c = 0;
    while (buffer_size - c > 0 && buffer_view[buffer_size - c - 1] != '\n') c++;
    return c;
 };

void renderCursor(Color color)
{
    int x = buffer_cursor_col * FONT_CHAR_WIDTH * FONT_SCALE;
    int y = buffer_cursor_row * FONT_CHAR_HEIGHT * FONT_SCALE;
    DrawRectangle(x, y, FONT_CHAR_WIDTH * FONT_SCALE, FONT_CHAR_HEIGHT * FONT_SCALE, color);
}

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
    if ((codepoint >= 32) && (codepoint <= 125) && (buffer_size < BUFFER_CAP)) {
	buffer[buffer_size++] = codepoint;
	buffer_cursor_col++;
	buffer[buffer_size] = '\0';
    }
}

int main()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "");
    
    Image font_image = LoadImageFromMemory(".png", font_image_data, sizeof(font_image_data));
    if (!IsImageValid(font_image)) {
	std::fprintf(stderr, "[ERROR] failed to load image from memory\n");
        exit(1);
    } 
    
    Font font = LoadFontFromImage(font_image, MAGENTA, 32);

    UnloadImage(font_image);
    SetTargetFPS(60);

    GLFWwindow* ctx = glfwGetCurrentContext(); 
    glfwSetKeyCallback(ctx, keyCallback);
    glfwSetCharCallback(ctx, charCallback);
    
    while (!WindowShouldClose()) {
	BeginDrawing();
        ClearBackground(Color{ 0x18, 0x18, 0x18, 0x0 });
        renderText(font, std::string{buffer}, Vec2f{ 0.0f, 0.0f }, FONT_SCALE, WHITE);
	renderCursor(WHITE);
	
        EndDrawing();
    }
    
    UnloadFont(font);
    CloseWindow();

    return 0;
}
