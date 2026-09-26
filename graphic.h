#pragma once

#include <variant>
#include <memory>
#include <cassert>
#include <vector>
#include <ranges>
#include <algorithm>

#include "raylib.h"
#include "rlgl.h"
#include "piece_table.h"
#include "cursor.h"
#include "la.h"

// ============================================================================
enum class KeyInputTag : int
{
    KIT_BACKSPACE,
    KIT_ENTER,
    KIT_LEFT,
    KIT_RIGHT,
    KIT_UP,
    KIT_DOWN,
    KIT_F1,
    KIT_F2,
    KIT_F3,
    KIT_F4,
    KIT_F5,
    __static_key_input_tag_count
};

struct CharParams
{
    float width;
    float height;
};

enum class SplitType;
struct OverlayBuffer;

class Graphic
{
public:
    virtual ~Graphic() = default;
    
    virtual void draw(const Font& font, const CharParams params) = 0;    
    virtual void add(std::shared_ptr<Graphic> component) {}
    //virtual remove(std::shared_ptr<Graphic> component) {}
};

struct ViewPort
{
    std::size_t first_visible_line;
    std::size_t first_visible_col;
    std::size_t visible_lines;
    std::size_t visible_cols;
};

class BufferView : public Graphic
{
public:
    BufferView(Rectangle *r,
	       ViewPort *vp,
	       Cursor *c,
	       OverlayBuffer *ob,
	       IBuffer *b)
        : window_rect( r )
	, view_port( vp )
	, cursor( c )
	, overlay_buf( ob )
	, buffer( b )
    {}
    template<typename DrawCharFunc>
    void drawCursor(const Vec2f& cursor_draw_pos, DrawCharFunc&& drawChar, const CharParams params);
    template<typename DrawCharFunc, typename WrapFunc>
    void drawOverlay(Vec2f& pos, Vec2f& cursor_draw_pos,
		     std::size_t offset_counter,
		     DrawCharFunc&& drawChar,
		     WrapFunc&& getWrappedPos);
    void draw(const Font& font, const CharParams params) override;
    // CursorDrawType getCursorDrawType() { return cursor->getDrawType(); }
    // std::size_t getCursorLine() { return cursor->getLine(); }
    // std::size_t getCursorCol() { return cursor->getCol(); }

private:
    Rectangle *window_rect;
    ViewPort *view_port;
    Cursor *cursor;
    OverlayBuffer *overlay_buf;
    IBuffer *buffer;
};

struct Leaf;
struct Node;
using LayoutTree = std::variant<
    Leaf,
    std::unique_ptr<Node>
    >;

class Window : public Graphic // add Observer that changes font on update()
{
public:
    Window(Rectangle r,
	   ViewPort vp,
	   Cursor c,
	   OverlayBuffer ob,
	   std::shared_ptr<IBuffer> b)
        : rect{ r }
	, view_port{ vp }
        , cursor{ c }
	, overlay_buf{ ob.start_offset }
	, buffer{ std::move(b) }
    {}
    
    ~Window() override = default;

    void draw(const Font& font, const CharParams params) override {	
        for (auto& child : graphics) {
	    // lazy dump overlay buffer
            child->draw(font, params);
        }
    }
    
    void add(std::shared_ptr<Graphic> component) override {
	graphics.push_back(component);
    }
    //remove(std::shared_ptr<Graphic> component) override {}

    void dumpOverlayBuffer();
    void handleCharInput(char c);
    void handleNavigationOrActionKey(KeyInputTag key);
    
    void moveCursorLeft();
    void moveCursorRight();
    void moveCursorUp();
    void moveCursorDown();
    void backspaceOnCursor();
    void newlineOnCursor();
    void scrollToCursor();
    
    void recalcViewPort(int char_width, int char_heigth);
    void attachBufferView();
    void setRect(const Rectangle& new_rect) { rect = new_rect; }

    Rectangle& getRect() { return rect; }
    ViewPort& getViewPort() { return view_port; }
    Cursor& getCursor() { return cursor; }
    IBuffer& getBuffer() { return *(buffer.get()); }
    std::shared_ptr<IBuffer> getBufferShared() const { return buffer; }
    CursorDrawType getCursorDrawType() { return cursor.getDrawType(); }
    std::size_t getCursorOffset() { return cursor.getOffset(); }
    
 private:
    std::vector<std::shared_ptr<Graphic>> graphics;

    Rectangle rect;
    ViewPort view_port;
    Cursor cursor;
    OverlayBuffer overlay_buf;
    std::shared_ptr<IBuffer> buffer;
};

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
	std::make_shared<PieceTable>("Hello World!")
    );
    
    new_window->attachBufferView();
    return new_window;
}

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
    
    std::shared_ptr<Window> created_window{};
    
    LayoutTree operator()(Leaf& leaf) {
	if (leaf.window == target_window) {
	    auto rec = leaf.window->getRect();
	    
	    if (split_type == SplitType::Horizontal) {
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

		created_window = std::make_shared<Window>(
		    bottom_rect,
		    leaf.window->getViewPort(),
		    Cursor{
			CursorDrawType::Hollow,
			BufferOffset {
			    leaf.window->getCursorOffset()
			}
		    },
		    OverlayBuffer{leaf.window->getCursorOffset()},
		    leaf.window->getBufferShared()
		);
		
	    } else if (split_type == SplitType::Vertical) {
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

		created_window = std::make_shared<Window>(
		    right_rect,
		    leaf.window->getViewPort(),
		    Cursor{
			CursorDrawType::Hollow,
			BufferOffset {
			    leaf.window->getCursorOffset()
			}
		    },
		    OverlayBuffer{leaf.window->getCursorOffset()},
		    leaf.window->getBufferShared()
                );
	    }
	    created_window->recalcViewPort(leaf.char_width, leaf.char_height);
	    created_window->scrollToCursor();
	    created_window->attachBufferView();

	    return std::make_unique<Node>(
                split_type,
                ratio,
                Leaf{ leaf.char_width, leaf.char_height, leaf.window },
                Leaf{ leaf.char_width, leaf.char_height, created_window }
            );
	}

	return leaf;
    }

    LayoutTree operator()(std::unique_ptr<Node>& node) {
	if (!node) return nullptr;

	node->left = std::visit(*this, node->left);

	if (!created_window) {
	    node->right = std::visit(*this, node->right);

	}
        return std::move(node);
    }
};

inline LayoutTree splitWindow(LayoutTree tree,
			      std::shared_ptr<Window> target,
			      SplitType sp,
			      std::shared_ptr<Window>& out_created_window,
			      float ratio = 0.5f)
{
    SplitVisitor visitor{ std::move(target), sp, ratio };
    LayoutTree result_tree = std::visit(visitor, tree);
    out_created_window = std::move(visitor.created_window);
    
    return result_tree;
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

// struct CursorVisitor
// {
//     std::size_t active_window_offset;
//     std::size_t active_window_old_offset;

//     void operator()(Leaf& leaf)
//     {
// 	if (leaf.window->getBufferText().empty()) return;
	
// 	auto& peer_window = leaf.window;
// 	if (active_window_offset < peer_window->getCursorOffset()) {
// 	    if (active_window_old_offset < active_window_offset) {
// 		// char was added
// 		peer_window->getCursor().advanceOffset();
// 	    } else {
// 		// char was removed
// 		peer_window->getCursor().retreatOffset();
// 	    }
	    
// 	    // const Text& buffer = peer_window->getBufferText();
// 	    // auto it = std::ranges::upper_bound(
// 	    //     buffer.begin(),
// 	    //     buffer.end(),
// 	    //     target_offset,
// 	    //     [](std::size_t offset_val, const Line& line) {
// 	    //         return offset_val < line.first;
// 	    //     }
// 	    // );
// 	    // std::size_t line = 0;
// 	    // if (it != buffer.begin()) {
// 	    //     line = std::distance(buffer.begin(), it) - 1;
// 	    // }
    
// 	    // std::size_t line_start_offset = buffer[line_idx].first;
// 	    // std::size_t col_idx = target_offset - line_start_offset;
// 	}
// 	peer_window->scrollToCursor();
//     }

//     void operator()(std::unique_ptr<Node>& node)
//     {
// 	if (!node) return;

// 	std::visit(*this, node->left);
// 	std::visit(*this, node->right);
//     }
// };

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

// inline void recalculateCursor(LayoutTree& tree, std::size_t active_window_offset, std::size_t active_window_old_offset)
// {
//     std::visit(CursorVisitor{ active_window_offset, active_window_old_offset }, tree);
// }
// ============================================================================
