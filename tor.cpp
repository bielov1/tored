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

void renderCursor(Color color)
{
    int x = buffer_cursor_col * FONT_CHAR_WIDTH * FONT_SCALE;
    int y = buffer_cursor_row * FONT_CHAR_HEIGHT * FONT_SCALE;
    DrawRectangle(x, y, FONT_CHAR_WIDTH * FONT_SCALE, FONT_CHAR_HEIGHT * FONT_SCALE, color);
}

int main()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "");
    Image font_image = LoadImageFromMemory(".png", font_image_data, sizeof(font_image_data));
    if (!IsImageValid(font_image)) {
        fprintf(stderr, "[ERROR] image is invalid.\n");
        exit(1);
    }
    
    Font font = LoadFontFromImage(font_image, MAGENTA, 32);

    auto getCursorX = [&, buffer_view = std::span{buffer}](){
	int c = 0;
	while (buffer_size - c > 0 && buffer_view[buffer_size - c - 1] != '\n') c++;
	return c;
    };

    UnloadImage(font_image);
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
 
        int key = GetCharPressed();

        while (key > 0) {
            if ((key >= 32) && (key <= 125) && (buffer_size < BUFFER_CAP)) {
                buffer[buffer_size++] = key;
		buffer_cursor_col = getCursorX();
                buffer[buffer_size] = '\0';
            }
            
            key = GetCharPressed();
        }
	
	if (IsKeyPressed(KEY_ENTER)) {
	    buffer[buffer_size++] = '\n';
	    buffer_cursor_row++;
	    buffer_cursor_col = 0;
	}
	
        if (IsKeyPressed(KEY_BACKSPACE)) {
            buffer_size--;
            if (buffer_size < 0) buffer_size = 0;
	    if (buffer[buffer_size] == '\n') {
		buffer_cursor_row--;
		buffer_cursor_col = getCursorX();
	    } else {
		buffer_cursor_col--;
	    }
	    
            buffer[buffer_size] = '\0';
        }

	
        
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
