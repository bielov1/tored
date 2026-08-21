#include <iostream>
#include "tor.h"
#include "renderer.h"

Editor::Editor()
    : buffer{}
    , cursor{}
{
    buffer.reserve(BUFFER_CAP);
    buffer.emplace_back(0, "");
}

void Editor::insertTextOnCursor(const std::string& text)
{    
    const std::size_t text_size = text.size();
    buffer[cursor.line_idx].second.insert(cursor.col_idx, text, 0, text_size);
    buffer[cursor.line_idx].first += text_size;
    cursor.col_idx += text_size;    
}

void Editor::handleKeyAction(int key)
{
    switch (key) {
    case GLFW_KEY_BACKSPACE:
	backspace();
	break;
    case GLFW_KEY_ENTER:
	createNewline();
	break;
    case GLFW_KEY_LEFT:
	cursorMoveLeft();
	break;
    case GLFW_KEY_RIGHT:
	cursorMoveRight();
	break;
    default:
	std::fprintf(stderr, "[WARNING] uknown key input\n");
    }
}

void Editor::backspace()
{
    // TODO: all characters on cursor.line_idx should be moved to upper line if cursor.col_idx == 0
    // free line or shift cursor.line_idx < upper by one line.
    if (cursor.col_idx > 0) {
	buffer[cursor.line_idx].second.erase(cursor.col_idx - 1, 1);
	buffer[cursor.line_idx].first -= 1;
	cursor.col_idx -= 1;
    }
}

void Editor::createNewline()
{
    // TODO: at cursor pos move to next line characters after it
    buffer.emplace(buffer.begin() + cursor.line_idx + 1, 0, "");
    cursor.line_idx += 1;
    cursor.col_idx = 0;
}

void Editor::cursorMoveLeft()
{
    if (cursor.col_idx > 0) {
        cursor.col_idx -= 1;
    } else {
	if (cursor.line_idx > 0) {
            cursor.line_idx -= 1;
            cursor.col_idx = buffer[cursor.line_idx].first;
        }
    }
}

void Editor::cursorMoveRight()
{
    if (cursor.col_idx < static_cast<int>(buffer[cursor.line_idx].first)) {
        cursor.col_idx += 1;
    } else {
	if (cursor.line_idx < buffer.size() - 1) {
            cursor.line_idx += 1;
            cursor.col_idx  = 0;
        }
    }
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) 
{
    (void)window;
    (void)scancode;
    (void)mods;
    if (key == GLFW_KEY_BACKSPACE && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
	Editor::getInstance().handleKeyAction(GLFW_KEY_BACKSPACE);
    }
    if (key == GLFW_KEY_ENTER && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        Editor::getInstance().handleKeyAction(GLFW_KEY_ENTER);
    }
    if (key == GLFW_KEY_LEFT && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        Editor::getInstance().handleKeyAction(GLFW_KEY_LEFT);
    }
    if (key == GLFW_KEY_RIGHT && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        Editor::getInstance().handleKeyAction(GLFW_KEY_RIGHT);
    }
}

void charCallback(GLFWwindow* window, unsigned int codepoint)
{
    (void)window;
    Editor::getInstance().insertTextOnCursor(std::string{(char)codepoint});
}

int main()
{       
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "");
    Editor& editor = Editor::getInstance();
    Renderer& renderer = Renderer::getInstance();

    GLFWwindow *ctx = glfwGetCurrentContext(); 
    glfwSetKeyCallback(ctx, keyCallback);
    glfwSetCharCallback(ctx, charCallback);
    
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
	BeginDrawing();
	ClearBackground(Color{ 0x18, 0x18, 0x18, 0x0 });
	renderer.renderScene(editor.getBuffer(), editor.getCursor(), Vec2f{ 0.0f, 0.0f });
        EndDrawing();
    }
    
    CloseWindow();

    return 0;
}
