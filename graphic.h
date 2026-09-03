#pragma once

#include <memory>
#include <cassert>
#include <vector>

#include "raylib.h"
#include "buffer.h"
#include "cursor.h"
#include "la.h"

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
    
    virtual void draw(Font font, int scale) = 0;
    
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
    
    void drawChar(Font font, char c, Vec2f pos, int scale, Color color);    
    void draw(Font font, int scale) override;

private:
    Rectangle *window_rect;
    ViewPort *view_port;
    Cursor *cursor;
    Buffer *buffer;
};

class Window : public Graphic
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

    void draw(Font font, int scale) override {
        for (auto& child : graphics) {
            child->draw(font, scale);
        }
    }
    void add(std::shared_ptr<Graphic> component) override {
	graphics.push_back(component);
    }
    //remove(std::shared_ptr<Graphic> component) override {}

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
