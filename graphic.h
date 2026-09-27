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
    virtual void add(std::shared_ptr<Graphic> component)
    {
	(void)component;
	throw std::logic_error("Leaf nodes do not support add()");
    }
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
 private:
    std::vector<std::shared_ptr<Graphic>> graphics;

    Rectangle rect;
    ViewPort view_port;
    Cursor cursor;
    OverlayBuffer overlay_buf;
    std::shared_ptr<IBuffer> buffer;
};

// ============================================================================
