#pragma once

#include <variant>
#include <memory>
#include <cassert>
#include <vector>

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
    int width;
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

struct LayoutVisitor
{
    void operator()(Leaf& leaf)
    {	
	leaf.window->recalcViewPort(leaf.char_width, leaf.char_height);
    }

    void operator()(std::unique_ptr<Node>& node)
    {
	if (node) {
	    std::visit(*this, node->left);
	    std::visit(*this, node->right);
	}
    }
};

inline LayoutTree splitWindow(LayoutTree tree, std::shared_ptr<Window> target, SplitType sp, float ratio = 0.5f)
{
    SplitVisitor visitor{target, sp, ratio};
    return std::visit(visitor, tree);
}

inline void recalculateLayout(LayoutTree& tree)
{
    std::visit(LayoutVisitor{}, tree);
}
// ============================================================================
