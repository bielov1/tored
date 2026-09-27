#include <iostream>
#include "tor.h"

static std::shared_ptr<Window> createNewWindow(int window_width, int window_height,
					       int char_height, int char_width,
					       int scale)
{
    Rectangle window_rect = {
	.x = 0.0f,
	.y = 0.0f,
	.width = static_cast<float>(window_width),
	.height = static_cast<float>(window_height)
    };
    
    ViewPort view_port = {
	.first_visible_line = 0,
	.first_visible_col  = 0,
	.visible_lines = static_cast<std::size_t>(window_height / (char_height * scale)),
	.visible_cols  = static_cast<std::size_t>(window_width / (char_width * scale))
    };
    
    Cursor cursor{
	CursorDrawType::Filled,
	BufferOffset {
	    .byte_offset = 0
	}
    };

    auto new_window = std::make_shared<Window>(
        window_rect,
        view_port,
        cursor,
	OverlayBuffer{cursor.getOffset()},
	std::make_shared<PieceTable>()
    );
    
    new_window->attachBufferView();
    return new_window;
}

Editor::Editor()
    : active_window( nullptr )
    , windows_layout{ Leaf{ 0, 0, nullptr } }
    , max_scroll_line{ 1024 }
    , max_scroll_col{ 256 }
    , screen_width{ DEFAULT_SCREEN_WIDTH }
    , screen_height{ DEFAULT_SCREEN_HEIGHT }
    , font{}
{
    size_t font_size = _binary_charmap_oldschool_white_png_end - _binary_charmap_oldschool_white_png_start;
    auto font_data = reinterpret_cast<const unsigned char*>(_binary_charmap_oldschool_white_png_start);
    font = loadPNGDataAsFont({font_data, font_size}, FONT_COLS, FONT_ROWS);
    
    active_window = createNewWindow(screen_width, screen_height,
				    FONT_CHAR_HEIGHT, FONT_CHAR_WIDTH,
				    FONT_SCALE);
    window_list.push_back(active_window);
    windows_layout = Leaf{
	FONT_CHAR_WIDTH * FONT_SCALE,
	FONT_CHAR_HEIGHT * FONT_SCALE,
	active_window
    };
}

Editor::~Editor()
{
    UnloadFont(font);
}

void Editor::handleKeyAction(KeyInputTag key)
{
    static_assert(KeyInputTag::__static_key_input_tag_count == 6);
    if (!active_window) throw "active_window always assumed to be valid\n";

    active_window->handleNavigationOrActionKey(key);
}

void Editor::onResize(int new_screen_width, int new_screen_height)
{
    if (!active_window) throw "onResize() always assumes active_window is valid\n";
    screen_width = new_screen_width;
    screen_height = new_screen_height;
    recalculateLayout(new_screen_width, new_screen_height);
}

void Editor::refreshScreen()
{
    std::visit(RenderVisitor{font}, windows_layout);
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

void Editor::recalculateLayout(int new_screen_width, int new_screen_height)
{
    Rectangle screen_bounds = {
        .x = 0.0f,
        .y = 0.0f,
        .width = static_cast<float>(new_screen_width),
        .height = static_cast<float>(new_screen_height)
    };
    std::visit(LayoutVisitor{ screen_bounds }, windows_layout);
}

// void Editor::cycleNextWindow() 
// {
//     if (window_list.size() <= 1) return;
//     window_list.splice(window_list.end(), window_list, window_list.begin());
// }

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) 
{
    (void)window;
    (void)scancode;
    (void)mods;
    static_assert(KeyInputTag::__static_key_input_tag_count == 6);
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
}

void charCallback(GLFWwindow* window, unsigned int codepoint)
{
    (void)window;
    if (codepoint >= 32 && codepoint <= 126) {
	char c = static_cast<char>(codepoint);
        Editor::getInstance().getActiveWindow()->handleCharInput(c);
    }
}

void customWindowSizeCallback(GLFWwindow* window, int new_width, int new_height)
{
    (void)window;
    rlViewport(0, 0, new_width, new_height);
    Editor::getInstance().onResize(new_width, new_height);
}

int main()
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
