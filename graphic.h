#pragma once

#include <variant>
#include <memory>
#include <cassert>
#include <vector>
#include <ranges>
#include <algorithm>

#include "raylib.h"
#include "rlgl.h"
#include "buffer.h"
#include "cursor.h"
#include "la.h"

// ============================================================================

struct ViewPort
{
    std::size_t first_visible_line;
    std::size_t first_visible_col;
    std::size_t visible_lines;
    std::size_t visible_cols;
};

class Graphic
{
public:
    virtual ~Graphic() = default;
    
    virtual void draw(const Font& font, int char_width, int char_height) = 0;    
    virtual void add(std::shared_ptr<Graphic> component) {}
    //virtual remove(std::shared_ptr<Graphic> component) {}
};

class BufferView : public Graphic
{
public:
    BufferView(Rectangle *r,
	       ViewPort *vp,
	       Cursor *c,
	       Buffer *b)
        : window_rect( r )
	, view_port( vp )
	, cursor( c )
	, buffer( b )
    {}
    
    void drawChar(const Font& font, char c, int char_width, int char_height, Vec2f pos, Color color);    
    void draw(const Font& font, int char_width, int char_height) override;

private:
    Rectangle *window_rect;
    ViewPort *view_port;
    Cursor *cursor;
    Buffer *buffer;
};

class Window : public Graphic // add Observer that changes font on update()
{
public:
    Window(Rectangle r,
	   ViewPort vp,
	   Cursor c,
	   std::shared_ptr<Buffer> b)
        : rect{ r }
	, view_port{ vp }
        , cursor{ c }
	, buffer{ std::move(b) }
    {}
    
    ~Window() override = default;

    void draw(const Font& font, int char_width, int char_height) override {
        for (auto& child : graphics) {
            child->draw(font, char_width, char_height);
        }
    }
    
    void add(std::shared_ptr<Graphic> component) override {
	graphics.push_back(component);
    }
    //remove(std::shared_ptr<Graphic> component) override {}

    void moveCursorLeft();
    void moveCursorRight();
    void moveCursorUp();
    void moveCursorDown();
    void newlineOnCursor();
    void insertChar(char c);
    void scrollToCursor();
    
    void recalcViewPort(int char_width, int char_heigth);
    void attachBufferView();
    void setRect(const Rectangle& new_rect) { rect = new_rect; }

    Rectangle& getRect()    { return rect; }
    ViewPort& getViewPort() { return view_port; }
    Cursor& getCursor()     { return cursor; }
    
    Buffer& getBuffer() const { return *buffer; }
    std::shared_ptr<Buffer> getBufferShared() const { return buffer; }

 private:
    std::vector<std::shared_ptr<Graphic>> graphics;

    Rectangle rect;
    ViewPort view_port;
    Cursor cursor;
    std::shared_ptr<Buffer> buffer;
};

// ============================================================================

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
struct Node;
using LayoutTree = std::variant<
    Leaf,
    std::unique_ptr<Node>
    >;
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
struct SplitVisitor
{
    std::shared_ptr<Window> target_window;
    SplitType split_type;
    float ratio;

    LayoutTree operator()(Leaf& leaf) {
	if (leaf.window == target_window) {
	    auto rec = leaf.window->getRect();
	    if (split_type == SplitType::Horizontal) {
		Rectangle left_rect = {
		    .x = rec.x,
		    .y = rec.y,
		    .width = rec.width * ratio,
		    .height = rec.height
		};

		leaf.window->setRect(left_rect);
		leaf.window->recalcViewPort(leaf.char_width, leaf.char_height);
		leaf.window->scrollToCursor();
		    
		Rectangle right_rect = {
		    .x = rec.x + rec.width * ratio,
		    .y = rec.y,
		    .width = rec.width * (1.0f - ratio),
		    .height = rec.height
		};

		auto new_window = std::make_shared<Window>(
		    right_rect,
		    leaf.window->getViewPort(),
		    leaf.window->getCursor(),
		    leaf.window->getBufferShared()
                );

		new_window->recalcViewPort(leaf.char_width, leaf.char_height);
		new_window->scrollToCursor();
		new_window->attachBufferView();
	        return std::make_unique<Node>(
		    split_type,
		    ratio,
		    Leaf{ leaf.char_width, leaf.char_height, leaf.window },
		    Leaf{ leaf.char_width, leaf.char_height, new_window }
		);
	    } else if (split_type == SplitType::Vertical) {
		Rectangle top_rect = {
		    .x = rec.x,
		    .y = rec.y,
		    .width = rec.width,
		    .height = rec.height * ratio
		};

		leaf.window->setRect(top_rect);
		leaf.window->recalcViewPort(leaf.char_width, leaf.char_height);
		leaf.window->scrollToCursor();

		Rectangle bottom_rect = {
		    .x = rec.x,
		    .y = rec.y + rec.height * ratio,
		    .width = rec.width,
		    .height = rec.height * (1.0f - ratio)
		};

		auto new_window = std::make_shared<Window>(
		    bottom_rect,
		    leaf.window->getViewPort(),
		    leaf.window->getCursor(),
		    leaf.window->getBufferShared()
		);

		new_window->recalcViewPort(leaf.char_width, leaf.char_height);
		new_window->scrollToCursor();
		new_window->attachBufferView();
		return std::make_unique<Node>(
		    split_type,
		    ratio,
		    Leaf{ leaf.char_width, leaf.char_height, leaf.window },
		    Leaf{ leaf.char_width, leaf.char_height, new_window }
	        );
	    }
	}

	return Leaf{ leaf.char_width,
		     leaf.char_height,
		     leaf.window };
    }

    LayoutTree operator()(std::unique_ptr<Node>& node) {
	node->left = std::visit(*this, node->left);
        node->right = std::visit(*this, node->right);
        return std::move(node);
    }
};

inline LayoutTree splitWindow(LayoutTree tree, std::shared_ptr<Window> target, SplitType sp, float ratio = 0.5f)
{
    SplitVisitor visitor{target, sp, ratio};
    return std::visit(visitor, tree);
}

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
	} else if (node->split_type == SplitType::Vertical) {
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
	}
    }
};

struct CursorVisitor
{
    std::size_t active_window_cursor_line;
    std::size_t active_window_cursor_col;

    void operator()(Leaf& leaf)
    {
	if (leaf.window->getCursor().getLine() > active_window_cursor_line) {
	    leaf.window->getCursor().setPosition(active_window_cursor_line,
						 active_window_cursor_col);
	} else if (leaf.window->getCursor().getLine() == active_window_cursor_line &&
		   leaf.window->getCursor().getCol() > active_window_cursor_col) {
	    leaf.window->getCursor().setCol(active_window_cursor_col);
	}
	
	leaf.window->scrollToCursor();
    }

    void operator()(std::unique_ptr<Node>& node)
    {
	if (!node) return;

	std::visit(*this, node->left);
	std::visit(*this, node->right);
    }
};

inline void recalculateLayout(LayoutTree& tree, int new_screen_width, int new_screen_height)
{
    Rectangle screen_bounds = {
        .x = 0.0f,
        .y = 0.0f,
        .width = static_cast<float>(new_screen_width),
        .height = static_cast<float>(new_screen_height)
    };
    std::visit(LayoutVisitor{ screen_bounds }, tree);
}

inline void recalculateCursor(LayoutTree& tree, std::size_t active_window_cursor_line, std::size_t active_window_cursor_col)
{
    std::visit(CursorVisitor{ active_window_cursor_line, active_window_cursor_col }, tree);
}
// ============================================================================
