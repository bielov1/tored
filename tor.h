#pragma once

#include <print>
#include <cstdio>
#include <fstream>
#include <list>

#include <string_view>

#define GRAPHICS_API_OPENGL_33
#include "GLFW/glfw3.h"

#include "graphic.h"

extern "C" {
    extern const unsigned char _binary_charmap_oldschool_white_png_start[];
    extern const unsigned char _binary_charmap_oldschool_white_png_end[];
}

static const int DEFAULT_SCREEN_WIDTH = 800;
static const int DEFAULT_SCREEN_HEIGHT = 600;

static const int ASCII_DISPLAY_LOW = 32;
static const int ASCII_DISPLAY_HIGH = 127;

static const int FONT_WIDTH = 128;
static const int FONT_HEIGHT = 64;
static const int FONT_COLS = 18;
static const int FONT_ROWS = 7;
static const int  FONT_SCALE = 2;
static const int FONT_CHAR_WIDTH = (FONT_WIDTH / FONT_COLS);
static const int FONT_CHAR_HEIGHT = (FONT_HEIGHT / FONT_ROWS);
static constexpr Color FONT_COLOR = WHITE;
static const size_t BUFFER_CAP = 1024;

constexpr bool operator==(KeyInputTag kit, int i) {
    return static_cast<std::underlying_type_t<KeyInputTag>>(kit) == i;
}

enum class SplitType { Horizontal, Vertical };
struct Leaf
{
    Leaf(int cw, int ch, std::shared_ptr<Window> win)
	: char_width{ cw }
	, char_height{ ch }
	, window( win )
    {}
    int char_width;
    int char_height;
    std::shared_ptr<Window> window;
};

struct Node
{
    Node(SplitType st, float rat, LayoutTree l, LayoutTree r)
	: split_type{st}
	, ratio{rat}
	, left(std::move(l))
	, right(std::move(r))
    {}
    
    SplitType split_type;
    float ratio;
    LayoutTree left;
    LayoutTree right;
};

struct RenderVisitor
{
    Font font;

    void operator()(const Leaf& leaf) const {
	
        if (leaf.window) {
            leaf.window->draw(font, CharParams{
		    .width  = static_cast<float>(leaf.char_width),
		    .height = static_cast<float>(leaf.char_height)
		});
        }
    }

    void operator()(const std::unique_ptr<Node>& node) const {
        if (node) {
            std::visit(*this, node->left);
            std::visit(*this, node->right);
        }
    }
};

struct LayoutVisitor
{
    Rectangle current_bounds;
    
    void operator()(Leaf& leaf)
    {
	leaf.window->setRect(current_bounds);
	leaf.window->recalcViewPort(leaf.char_width, leaf.char_height);
	leaf.window->scrollToCursor();
    }

    void operator()(std::unique_ptr<Node>& node)
    {
	if (!node) return;

	if (node->split_type == SplitType::Horizontal) {
	    float top_height = current_bounds.height * node->ratio;
	    float bottom_height = current_bounds.height - top_height;

	    Rectangle top_bounds = {
		.x = current_bounds.x,
		.y = current_bounds.y,
		.width = current_bounds.width,
		.height = top_height
	    };

	    Rectangle bottom_bounds = {
		.x = current_bounds.x,
		.y = current_bounds.y + top_height,
		.width = current_bounds.width,
		.height = bottom_height
	    };

	    std::visit(LayoutVisitor{ top_bounds }, node->left);
	    std::visit(LayoutVisitor{ bottom_bounds} , node->right);
	} else if (node->split_type == SplitType::Vertical) {
	    float left_width = current_bounds.width * node->ratio;
	    float right_width = current_bounds.width - left_width;

	    Rectangle left_bounds = {
		.x = current_bounds.x,
		.y = current_bounds.y,
		.width = left_width,
		.height = current_bounds.height
	    };

	    Rectangle right_bounds = {
		.x = current_bounds.x + left_width,
		.y = current_bounds.y,
		.width = right_width,
		.height = current_bounds.height
	    };

	    std::visit(LayoutVisitor{ left_bounds }, node->left);
	    std::visit(LayoutVisitor{ right_bounds}, node->right);
	}
    }
};

class Editor
{
public:
    static Editor& getInstance()
    {
	static Editor editor{};
	return editor;
    }    

    void handleKeyAction(KeyInputTag key);
    void onResize(int new_window_width, int new_window_height);
    void refreshScreen();
    void saveToFile(const std::string& file_path);
    void loadFromFile(const std::string& file_path);
    Font loadPNGDataAsFont(std::span<const unsigned char> data, int cols, int rows);

    void recalculateLayout(int new_screen_width, int new_screen_height);
    
    // void cycleNextWindow();
    // void cyclePreviousWindow();
    const std::shared_ptr<Window>& getActiveWindow() const { return active_window; }
    // void setActiveWindow(std::shared_ptr<Window> new_active_window) { active_window = new_active_window; }
    
    Editor(const Editor&) = delete;
    Editor& operator=(const Editor&) = delete;
private:
    Editor();
    ~Editor();

    // switch to linked list or deque
    std::list<std::shared_ptr<Window>> window_list;
    std::shared_ptr<Window> active_window;
    LayoutTree windows_layout;
    std::size_t max_scroll_line;
    std::size_t max_scroll_col;
    int screen_width;
    int screen_height;
    Font font;
};
