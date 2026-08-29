#include <iostream>
#include "tor.h"


Editor::Editor()
    : graphic{}
    , font{}
{
    size_t font_size = _binary_charmap_oldschool_white_png_end - _binary_charmap_oldschool_white_png_start;
    unsigned char *font_data = (unsigned char *)_binary_charmap_oldschool_white_png_start;
    std::span<const unsigned char> data_view{font_data, font_size};
    font = loadPNGDataAsFont(data_view, FONT_COLS, FONT_ROWS);

    createNewWindow(SCREEN_WIDTH, SCREEN_HEIGHT);
}

Editor::~Editor()
{
    UnloadFont(font);
}

void Editor::insertTextOnCursor(const std::string& text, std::size_t text_size, int col_idx, int line_idx)
{
    buffer[line_idx].second.insert(col_idx, text, 0, text_size);
    buffer[line_idx].first += text_size;
}

void Editor::handleKeyAction(KeyInputTag key)
{
    static_assert(KeyInputTag::__static_key_input_tag_count == 7);
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
    case KeyInputTag::KIT_F5:
	saveToFile(std::string{"output"});
	break;
    default:
	std::fprintf(stderr, "[WARNING] uknown key input\n");
    }
}
// abcde
// |fghijklmnop
// ^ cursor
// after backspace()
//
// abcdefghijklmop
//      ^ cursor
void Editor::backspaceOnCursor()jj
{
    if (cursor.col_idx > 0) {
	cursor.col_idx -= 1;
	buffer[cursor.line_idx].second.erase(cursor.col_idx, 1);
	buffer[cursor.line_idx].first -= 1;
    } else {
	if (cursor.line_idx > 0) {
	    auto& [line_size, line] = buffer[cursor.line_idx];
	    std::size_t prev_line_size = buffer[cursor.line_idx - 1].first;
	    insertTextOnCursor(std::move(line), line_size, prev_line_size, cursor.line_idx - 1);
	    buffer.erase(buffer.begin() + cursor.line_idx);
	    cursor.line_idx -= 1;
	    cursor.col_idx = prev_line_size;
	}
    }
}

// abcdefghijklmnop
//      ^ cursor
// after createNewline()
// abcde
// fghijklmnop

// a|abc
void Editor::newlineOnCursor()
{
    auto& [line_size, line] = buffer[cursor.line_idx];

    std::string rest = line.substr(cursor.col_idx);

    line.erase(cursor.col_idx);
    line_size = cursor.col_idx;

    buffer.emplace(buffer.begin() + cursor.line_idx + 1, rest.size(), std::move(rest)); 

    cursor.line_idx += 1;
    cursor.col_idx = 0;
}

void Editor::moveCursorLeft()
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

void Editor::moveCursorRight()
{
    if (cursor.col_idx < buffer[cursor.line_idx].first) {
        cursor.col_idx += 1;
    } else {
	if (cursor.line_idx < buffer.size() - 1) {
            cursor.line_idx += 1;
            cursor.col_idx  = 0;
        }
    }
}

void Editor::moveCursorUp()
{
    if (cursor.line_idx > 0) {
	cursor.line_idx -= 1;
	if (buffer[cursor.line_idx].first < cursor.col_idx) {
	    cursor.col_idx = buffer[cursor.line_idx].first;
	}   
    }
}

void Editor::moveCursorDown()
{
    if (cursor.line_idx < buffer.size() - 1) {
	cursor.line_idx += 1;
	if (buffer[cursor.line_idx].first < cursor.col_idx) {
	    cursor.col_idx = buffer[cursor.line_idx].first;	    
	}
    }
}

void Editor::createNewWindow(int window_width, int window_height)
{
    auto new_window = std::make_shared<Window>();
    
    auto new_buffer = std::make_shared<Buffer>("Hello World!");
    
    auto new_cursor = std::make_shared<Cursor>();
    
    auto buffer_view = std::make_shared<BufferView>(
	ViewPort{
	    .first_visible_line = 0,
	    .first_visible_col  = 0,
	    .visible_lines = window_height / FONT_HEIGHT,
	    .visible_cols  = window_width / FONT_WIDTH
	},
	new_buffer, new_cursor);
    
    new_window->add(buffer_view);
    
    open_windows.push_back(new_window);
    active_window = new_window;
}

inline std::shared_ptr<Window> Editor::getActiveWindow()
{
    return active_window;
}

void Editor::refreshScreen()
{
    if (active_window) {
	active_window->draw(font);
    }
}

void Editor::saveToFile(const std::string& file_path)
{
    std::ofstream ofs{file_path, std::ios_base::binary};
    if (!ofs) {
        throw std::runtime_error("Failed to open file: " + file_path);
    }
    
    for (std::size_t i = 0; i < buffer.size(); ++i) {
	const auto& [size, line] = buffer[i];
	ofs.write(line.data(), size);
	ofs.put('\n');
    }
    std::print(stdout, "[INFO] Successfully save buffer content to file: {}\n", file_path);
}

void Editor::loadFromFile(const std::string& file_path)
{
    assert(buffer.size() == 0 && "Buffer should be empty.");
    std::ifstream ifs{file_path, std::ios_base::binary | std::ios_base::ate};
    if (!ifs) throw std::runtime_error("Failed to open file: " + file_path);

    std::string content(ifs.tellg(), '\0');
    ifs.seekg(0, std::ios::beg);
    ifs.read(content.data(), content.size());

    buffer = content 
           | std::views::split('\n')
           | std::views::transform([](auto&& range) {
                 std::string_view sv{range.begin(), range.end()};
                 return TextLine{ sv.size(), std::string(sv) };
             })
           | std::ranges::to<Buffer>();
}

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
    static_assert(KeyInputTag::__static_key_input_tag_count == 7);
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
    if (key == GLFW_KEY_F5 && action == GLFW_PRESS) {
        Editor::getInstance().handleKeyAction(KeyInputTag::KIT_F5);
    }    
}

void charCallback(GLFWwindow* window, unsigned int codepoint)
{
    (void)window;
    Editor& editor = Editor::getInstance();
    Cursor& cursor = editor.getCursor();
    editor.insertTextOnCursor(std::string{(char)codepoint}, 1, cursor.col_idx, cursor.line_idx);
    cursor.col_idx += 1;
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
	//renderer.renderScene(editor.getBuffer(), editor.getCursor(), Vec2f{ 0.0f, 0.0f });
	editor.refreshScreen();
        EndDrawing();
    }
    
    CloseWindow();

    return 0;
}
