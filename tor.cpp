#include <iostream>
#include "tor.h"

Editor::Editor()
    : max_scroll_line{1024}
    , max_scroll_col{256}
    , font{}
{
    size_t font_size = _binary_charmap_oldschool_white_png_end - _binary_charmap_oldschool_white_png_start;
    unsigned char *font_data = (unsigned char *)_binary_charmap_oldschool_white_png_start;
    std::span<const unsigned char> data_view{font_data, font_size};
    font = loadPNGDataAsFont(data_view, FONT_COLS, FONT_ROWS);
    
    setActiveWindow(createNewWindow(SCREEN_WIDTH, SCREEN_HEIGHT));
}

Editor::~Editor()
{
    UnloadFont(font);
}

void Editor::handleKeyAction(KeyInputTag key)
{
    static_assert(KeyInputTag::__static_key_input_tag_count == 8);
    switch (key) {
    case KeyInputTag::KIT_BACKSPACE:
	backspaceOnCursor();
	break;
    case KeyInputTag::KIT_ENTER:
	newlineOnCursor();
	break;
    case KeyInputTag::KIT_LEFT:
	moveCursorLeft();
	break;
    case KeyInputTag::KIT_RIGHT:
	moveCursorRight();
	break;
    case KeyInputTag::KIT_UP:
	moveCursorUp();
	break;
    case KeyInputTag::KIT_DOWN:
	moveCursorDown();
	break;
    case KeyInputTag::KIT_F2:
	horizontalSplitScreen();
	break;
    // case GLFW_KEY_F5:
    // 	std::fprintf(stdout, "F5 was pressed\n");
    // 	saveToFile(std::string{"output"});
    // 	break;
    default:
	std::fprintf(stderr, "[WARNING] uknown key input\n");
    }
}

// IDEA: editor should have main window from which
// peer windows constantly retrives position of main window cursor
// to open the exact file name the cursor hovers. Just like threads.
// should all peer windows do that?

std::shared_ptr<Window> Editor::createNewWindow(int window_width, int window_height)
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
    
    addGraphicsToWindow(new_window);
    
    open_windows.push_back(new_window);
    return new_window;
}

std::shared_ptr<Window> Editor::createNewWindow(std::shared_ptr<Window> other_window)
{
    if (!other_window) return nullptr;

    auto new_window = std::make_shared<Window>(
        other_window->getRect(),
        other_window->getViewPort(),
        other_window->getCursor(),
	other_window->getBufferShared()
    );
    
    addGraphicsToWindow(new_window);
    
    open_windows.push_back(new_window);
    return new_window; 
}

void Editor::addGraphicsToWindow(std::shared_ptr<Window> window)
{
    auto buffer_view = std::make_shared<BufferView>(
	&window->getRect(),
	&window->getViewPort(),
	&window->getCursor(),
	&window->getBuffer()
    );
    
    window->add(buffer_view);
}

void Editor::moveCursorLeft()
{
    if (!active_window) return;

    auto& buf = active_window->getBuffer();
    auto& cur = active_window->getCursor();
    auto& view_port = active_window->getViewPort();

    const auto& text = buf.getText();
    if (text.empty()) return;

    std::size_t current_line = cur.getLine();
    
    if (cur.getCol() > 0) {
	cur.retreatCol();
    } else if (current_line > 0) {
	std::size_t prev_line_size = text[current_line - 1].first;
	cur.setPosition(current_line - 1, prev_line_size);
    }
    scrollToCursor(cur, view_port);
}

void Editor::moveCursorRight()
{
    if (!active_window) return;

    auto& buf = active_window->getBuffer();
    auto& cur = active_window->getCursor();
    auto& view_port = active_window->getViewPort();

    const auto& text = buf.getText();
    if (text.empty()) return;

    std::size_t current_line = cur.getLine();
    std::size_t line_size = text[current_line].first;

    if (cur.getCol() < line_size) {
        cur.advanceCol();
    } else if (current_line + 1 < text.size()) {
        cur.setPosition(current_line + 1, 0);
    }
    scrollToCursor(cur, view_port);
}

void Editor::moveCursorUp()
{
    if (!active_window) return;

    auto& buf = active_window->getBuffer();
    auto& cur = active_window->getCursor();
    auto& view_port = active_window->getViewPort();  

    const auto& text = buf.getText();
    if (text.empty()) return;

    std::size_t current_line = cur.getLine();
	
    if (current_line > 0) {
	std::size_t prev_line_size = text[current_line - 1].first;
	std::size_t new_col = std::ranges::clamp(cur.getCol(), std::size_t{0}, prev_line_size);
	cur.setPosition(current_line - 1, new_col);
    }
    
    scrollToCursor(cur, view_port);
}

void Editor::moveCursorDown()
{
    if (!active_window) return;

    auto& buf = active_window->getBuffer();
    auto& cur = active_window->getCursor();
    auto& view_port = active_window->getViewPort();
    
    const auto& text = buf.getText();
    if (text.empty()) return;

    std::size_t current_line = cur.getLine();
	
    if (current_line + 1 < text.size()) {
	std::size_t next_line_size = text[current_line + 1].first;
	std::size_t new_col = std::ranges::clamp(cur.getCol(), std::size_t{0}, next_line_size);
	cur.setPosition(current_line + 1, new_col);
    }
    
    scrollToCursor(cur, view_port);
}

void Editor::backspaceOnCursor()
{
    if (!active_window) return;

    auto& buf = active_window->getBuffer();
    auto& cur = active_window->getCursor();
    auto& view_port = active_window->getViewPort();
    
    const auto& text = buf.getText();
    if (text.empty()) return;

    std::size_t current_line = cur.getLine();
    std::size_t current_col = cur.getCol();

    if (cur.getCol() > 0) {
	buf.eraseCharAt(current_line, current_col - 1);
	cur.retreatCol();
    } else if (current_line > 0) {
	std::size_t prev_line = current_line - 1;
        std::size_t prev_line_size = text[prev_line].first;
	
	buf.appendLineTo(prev_line, current_line);
	buf.removeLine(current_line);
	cur.setPosition(prev_line, prev_line_size);
    }

    scrollToCursor(cur, view_port);    
}

void Editor::newlineOnCursor()
{
    if (!active_window) return;

    auto& buf = active_window->getBuffer();
    auto& cur = active_window->getCursor();
    auto& view_port = active_window->getViewPort();

    buf.splitLineAt(cur.getLine(), cur.getCol());
    cur.setPosition(cur.getLine() + 1, 0);
    scrollToCursor(cur, view_port);
}

void Editor::scrollToCursor(const Cursor& cur, ViewPort& vp)
{
    auto cur_line = cur.getLine();
    auto cur_col = cur.getCol();

    int padding = 3;
    
    if (cur_line < vp.first_visible_line)
	vp.first_visible_line = cur_line;

    if (cur_line >= vp.first_visible_line + vp.visible_lines)
	vp.first_visible_line = cur_line - vp.visible_lines + padding;

    if (cur_col < vp.first_visible_col)
	vp.first_visible_col = cur_col;

    if (cur_col >= vp.first_visible_col + vp.visible_cols)
	vp.first_visible_col = cur_col - vp.visible_cols + padding;

    vp.first_visible_line = std::ranges::clamp(vp.first_visible_line, std::size_t{0}, max_scroll_line);
    vp.first_visible_col = std::ranges::clamp(vp.first_visible_col, std::size_t{0}, max_scroll_col);
}    

void Editor::insertCharOnActiveWindow(char c)
{
    if (!active_window) return;

    auto& buf = active_window->getBuffer();
    auto& cur = active_window->getCursor();
    auto& view_port = active_window->getViewPort();

    buf.insertCharAt(cur.getLine(), cur.getCol(), c);
    cur.advanceCol();
    scrollToCursor(cur, view_port);
}

void Editor::refreshScreen()
{
    for (const auto& window : open_windows) {
	if (window) {
	    window->draw(font, FONT_SCALE);
	}
    }
}

void Editor::closeActiveWindow()
{
    assert(false && "closeActiveWindow() is not implemented yet\n");
}

void Editor::horizontalSplitScreen()
{
    if (!active_window) return;
    
    std::shared_ptr<Window> new_window = createNewWindow(active_window);
    
    auto& active_window_rect = active_window->getRect();
    auto& active_window_view_port = active_window->getViewPort();
    auto& new_window_rect = new_window->getRect();
    auto& new_window_view_port = new_window->getViewPort();

    active_window_rect.width /= 2.f;
    active_window_view_port.visible_cols = static_cast<std::size_t>(active_window_rect.width / (FONT_CHAR_WIDTH * FONT_SCALE));

    new_window_rect.width /= 2.f;
    new_window_rect.x = active_window_rect.x + active_window_rect.width;
    new_window_view_port.visible_cols = static_cast<std::size_t>(new_window_rect.width / (FONT_CHAR_WIDTH * FONT_SCALE));
    
    scrollToCursor(active_window->getCursor(), active_window_view_port);
    scrollToCursor(new_window->getCursor(), new_window_view_port);
}

void Editor::saveToFile(const std::string& file_path)
{
    (void)file_path;
    assert(false && "saveToFile() is not implemented yet\n");
}

// void Editor::loadFromFile(const std::string& file_path)
// {
//     assert(buffer.size() == 0 && "Buffer should be empty.");
//     std::ifstream ifs{file_path, std::ios_base::binary | std::ios_base::ate};
//     if (!ifs) throw std::runtime_error("Failed to open file: " + file_path);

//     std::string content(ifs.tellg(), '\0');
//     ifs.seekg(0, std::ios::beg);
//     ifs.read(content.data(), content.size());

//     buffer = content 
//            | std::views::split('\n')
//            | std::views::transform([](auto&& range) {
//                  std::string_view sv{range.begin(), range.end()};
//                  return TextLine{ sv.size(), std::string(sv) };
//              })
//            | std::ranges::to<Buffer>();
// }

Font Editor::loadPNGDataAsFont(std::span<const unsigned char> data, int cols, int rows)
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

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) 
{
    (void)window;
    (void)scancode;
    (void)mods;
    static_assert(KeyInputTag::__static_key_input_tag_count == 8);
    if (key == GLFW_KEY_BACKSPACE && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
	Editor::getInstance().handleKeyAction(KeyInputTag::KIT_BACKSPACE);
    }
    if (key == GLFW_KEY_ENTER && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        Editor::getInstance().handleKeyAction(KeyInputTag::KIT_ENTER);
    }
    if (key == GLFW_KEY_LEFT && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        Editor::getInstance().handleKeyAction(KeyInputTag::KIT_LEFT);
    }
    if (key == GLFW_KEY_RIGHT && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        Editor::getInstance().handleKeyAction(KeyInputTag::KIT_RIGHT);
    }
    if (key == GLFW_KEY_UP && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        Editor::getInstance().handleKeyAction(KeyInputTag::KIT_UP);
    }
    if (key == GLFW_KEY_DOWN && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        Editor::getInstance().handleKeyAction(KeyInputTag::KIT_DOWN);
    }
    if (key == GLFW_KEY_F2 && action == GLFW_PRESS) {
        Editor::getInstance().handleKeyAction(KeyInputTag::KIT_F2);
    }
    if (key == GLFW_KEY_F5 && action == GLFW_PRESS) {
        Editor::getInstance().handleKeyAction(KeyInputTag::KIT_F5);
    }
}

void charCallback(GLFWwindow* window, unsigned int codepoint)
{
    (void)window;
    if (codepoint >= 32 && codepoint <= 126) {
        Editor::getInstance().insertCharOnActiveWindow(static_cast<char>(codepoint));
    }
}

int main(int argc, char *argv[])
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "");
    
    GLFWwindow *ctx = glfwGetCurrentContext(); 
    glfwSetKeyCallback(ctx, keyCallback);
    glfwSetCharCallback(ctx, charCallback);
    
    SetTargetFPS(60);
    
    Editor& editor = Editor::getInstance();

    // std::string load_file_name = "la.cpp";
    // editor.loadFromFile(load_file_name);

    while (!WindowShouldClose()) {
	BeginDrawing();
	ClearBackground(Color{ 0x18, 0x18, 0x18, 0x0 });
	editor.refreshScreen();
        EndDrawing();
    }
    
    CloseWindow();

    return 0;
}
