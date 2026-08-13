#include "tor.h"

void renderChar(Font font, char c, Vec2f pos, float scale, Color color)
{
    DrawTextCodepoint(font, (int)c, Vector2{ pos.x, pos.y }, (float)font.baseSize * scale, color);
}

void renderText(Font font, const std::string &text, Vec2f pos, float scale, Color color)
{
    for (char c : text) {
        renderChar(font, c, pos, scale, color);
        pos.x += FONT_CHAR_WIDTH * scale;
    }
}

char buffer[BUFFER_CAP + 1];
int buffer_size = 0;

int main()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "");

    Image font_image = LoadImage("./charmap-oldschool_white.png");
    if (!IsImageValid(font_image)) {
        fprintf(stderr, "image is invalid.\n");
        exit(1);
    }
    
    Font font = LoadFontFromImage(font_image, MAGENTA, 32);

    UnloadImage(font_image);
    SetTargetFPS(60);

    while (!WindowShouldClose()) {

        int key = GetCharPressed();

        while (key > 0) {
            if ((key >= 32) && (key <= 125) && (buffer_size < BUFFER_CAP)) {
                buffer[buffer_size++] = key;
                buffer[buffer_size] = '\0';
            }
            
            key = GetCharPressed();
        }

        if (IsKeyPressed(KEY_BACKSPACE)) {
            buffer_size--;
            if (buffer_size < 0) buffer_size = 0;
            buffer[buffer_size] = '\0';
        }
        
        BeginDrawing();
        ClearBackground(Color{ 0x18, 0x18, 0x18, 0x0 });
        renderText(font, std::string{buffer}, Vec2f{ 0.0f, 0.0f }, 2.0f, WHITE);
        EndDrawing();
    }
    
    UnloadFont(font);
    CloseWindow();

	return 0;
}
