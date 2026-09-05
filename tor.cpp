#include <iostream>
#include "tor.h"

Editor::Editor()
    : active_window( nullptr )
    , root_tree{ Leaf{ 0, 0, nullptr } }
    , max_scroll_line{ 1024 }
    , max_scroll_col{ 256 }
    , screen_width{ DEFAULT_SCREEN_WIDTH }
    , screen_height{ DEFAULT_SCREEN_HEIGHT }
    , font{}
{
    size_t font_size = _binary_charmap_oldschool_white_png_end - _binary_charmap_oldschool_white_png_start;
    auto font_data = reinterpret_cast<const unsigned char*>(_binary_charmap_oldschool_white_png_start);
    font = loadPNGDataAsFont({font_data, font_size}, FONT_COLS, FONT_ROWS);
}

Editor::~Editor()
{
    UnloadFont(font);
}

void Editor::handleKeyAction(KeyInputTag key)
{
    static_assert(KeyInputTag::__static_key_input_tag_count == 9);
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
	splitScreen(SplitType::Horizontal);
	break;
    case KeyInputTag::KIT_F3:
	splitScreen(SplitType::Vertical);
	break;
    // case GLFW_KEY_F5:
    // 	std::fprintf(stdout, "F5 was pressed\n");
    // 	saveToFile(std::string{"output"});
    // 	break;
    default:
	std::fprintf(stderr, "[WARNING] uknown key input\n");
    }
}

void Editor::moveCursorLeft()
{
    if (!active_window) throw "moveCursorLeft() always assumes active_window is valid\n";
    auto& buf = active_window->getBuffer();
    auto& cur = active_window->getCursor();

    const auto& text = buf.getText();
    if (text.empty()) return;

    std::size_t current_line = cur.getLine();
    
    if (cur.getCol() > 0) {
	cur.retreatCol();
    } else if (current_line > 0) {
	std::size_t prev_line_size = text[current_line - 1].first;
	cur.setPosition(current_line - 1, prev_line_size);
    }
    scrollToCursor(cur, active_window->getViewPort());
}

void Editor::moveCursorRight()
{
    if (!active_window) assert(false && "moveCursorRight() always assumes active_window is valid\n");
    auto& buf = active_window->getBuffer();
    auto& cur = active_window->getCursor();

    const auto& text = buf.getText();
    if (text.empty()) return;

    std::size_t current_line = cur.getLine();
    std::size_t line_size = text[current_line].first;

    if (cur.getCol() < line_size) {
        cur.advanceCol();
    } else if (current_line + 1 < text.size()) {
        cur.setPosition(current_line + 1, 0);
    }
    scrollToCursor(cur, active_window->getViewPort());
}

void Editor::moveCursorUp()
{
    if (!active_window) throw "moveCursorUp() always assumes active_window is valid\n";
    auto& buf = active_window->getBuffer();
    auto& cur = active_window->getCursor();

    const auto& text = buf.getText();
    if (text.empty()) return;

    std::size_t current_line = cur.getLine();
	
    if (current_line > 0) {
	std::size_t prev_line_size = text[current_line - 1].first;
	std::size_t new_col = std::ranges::clamp(cur.getCol(), std::size_t{0}, prev_line_size);
	cur.setPosition(current_line - 1, new_col);
    }
    scrollToCursor(cur, active_window->getViewPort());
}

void Editor::moveCursorDown()
{
    if (!active_window) throw "moveCursorDown() always assumes active_window is valid\n";
    auto& buf = active_window->getBuffer();
    auto& cur = active_window->getCursor();
    
    const auto& text = buf.getText();
    if (text.empty()) return;

    std::size_t current_line = cur.getLine();
	
    if (current_line + 1 < text.size()) {
	std::size_t next_line_size = text[current_line + 1].first;
	std::size_t new_col = std::ranges::clamp(cur.getCol(), std::size_t{0}, next_line_size);
	cur.setPosition(current_line + 1, new_col);
    }
    scrollToCursor(cur, active_window->getViewPort());
}

void Editor::backspaceOnCursor()
{
    if (!active_window) throw "backspaceOnCursor() always assumes active_window is valid\n";
    auto& buf = active_window->getBuffer();
    auto& cur = active_window->getCursor();
    
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
    scrollToCursor(cur, active_window->getViewPort());    
}

void Editor::newlineOnCursor()
{
    if (!active_window) throw "newlineOnCursor() always assumes active_window is valid\n";
    auto& buf = active_window->getBuffer();
    auto& cur = active_window->getCursor();

    buf.splitLineAt(cur.getLine(), cur.getCol());
    cur.setPosition(cur.getLine() + 1, 0);

    scrollToCursor(cur, active_window->getViewPort());
}

void Editor::scrollToCursor(const Cursor& cur, ViewPort& vp)
{
    std::size_t cur_line = cur.getLine();
    std::size_t cur_col = cur.getCol();

    std::size_t padding = 5;
    
    if (cur_line < vp.first_visible_line)
	vp.first_visible_line = cur_line < padding ? 0 : cur_line - padding;

    // fix in scrollToCursor bug when padding not being adaptive to vp.visible_lines, causing first_visible_line/col jump over its visible lines
    if (cur_line >= vp.first_visible_line + vp.visible_lines) {
	if (padding > vp.visible_lines) padding = vp.visible_lines - 1;
	vp.first_visible_line = cur_line - vp.visible_lines + padding;
    }

    if (cur_col < vp.first_visible_col)
	vp.first_visible_col = cur_col < padding ? 0 : cur_col - padding;

    if (cur_col >= vp.first_visible_col + vp.visible_cols) {
	if (padding > vp.visible_cols) padding = vp.visible_cols - 1;
	vp.first_visible_col = cur_col - vp.visible_cols + padding;
    }
}    

void Editor::insertCharOnActiveWindow(char c)
{
    if (!active_window) throw "insertCharOnActiveWindow() always assumes active_window is valid\n";

    auto& buf = active_window->getBuffer();
    auto& cur = active_window->getCursor();
    auto& view_port = active_window->getViewPort();

    buf.insertCharAt(cur.getLine(), cur.getCol(), c);
    cur.advanceCol();
    scrollToCursor(cur, view_port);
}

void Editor::onResize(int new_screen_width, int new_screen_height)
{
    if (!active_window) throw "onResize() always assumes active_window is valid\n";
    screen_width = new_screen_width;
    screen_height = new_screen_height;
    recalculateLayout(root_tree, new_screen_width, new_screen_height);
    scrollToCursor(active_window->getCursor(), active_window->getViewPort());
}

void Editor::refreshScreen()
{
    if (!active_window) {
	active_window = createNewWindow(screen_width, screen_height);
	root_tree = Leaf{
	    FONT_CHAR_WIDTH * FONT_SCALE,
	    FONT_CHAR_HEIGHT * FONT_SCALE,
	    active_window
	};
    }
    std::visit(RenderVisitor{font}, root_tree);
}

void Editor::closeActiveWindow()
{
    assert(false && "closeActiveWindow() is not implemented yet\n");
}

void Editor::splitScreen(SplitType sp)
{
    if (!active_window) throw "splitScreen() always assumes active_window is valid\n";

    root_tree = splitWindow(std::move(root_tree), active_window, sp);
    // Gotta check this on horizontal split
    recalculateLayout(root_tree, screen_width, screen_height);
    scrollToCursor(active_window->getCursor(), active_window->getViewPort());
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
    static_assert(KeyInputTag::__static_key_input_tag_count == 9);
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
    if (key == GLFW_KEY_F3 && action == GLFW_PRESS) {
        Editor::getInstance().handleKeyAction(KeyInputTag::KIT_F3);
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

void customWindowSizeCallback(GLFWwindow* window, int new_width, int new_height)
{
    (void)window;
    rlViewport(0, 0, new_width, new_height);
    Editor::getInstance().onResize(new_width, new_height);
}

// TODO: adjust screen on run time
// TODO: render visible line to distinguish window bounds on splitted screen
// TODO: implement close active window
// TODO: switch between opened windows

int main(int argc, char *argv[])
{
    InitWindow(DEFAULT_SCREEN_WIDTH, DEFAULT_SCREEN_HEIGHT, "");
    
    GLFWwindow *ctx = glfwGetCurrentContext();
    glfwSetKeyCallback(ctx, keyCallback);
    glfwSetCharCallback(ctx, charCallback);
    glfwSetWindowSizeCallback(ctx, customWindowSizeCallback);
    
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
