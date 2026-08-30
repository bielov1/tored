#pragma once

#include <memory>
#include <cassert>

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
    BufferView(ViewPort *vp,
	       Buffer *b,
	       Cursor *c)
        : view_port(vp)
	, buffer(b)
	, cursor(c)
    {}
    
    void drawChar(Font font, char c, Vec2f pos, int scale, Color color);    
    void draw(Font font, int scale) override;
private:    
    ViewPort *view_port;
    Buffer *buffer;
    Cursor *cursor;
};

class Window : public Graphic
{
public:
    Window(std::unique_ptr<ViewPort> vp,
	   std::unique_ptr<Buffer> b,
	   std::unique_ptr<Cursor> c)
        : view_port(std::move(vp))
	, buffer(std::move(b))
        , cursor(std::move(c))
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

    ViewPort& getViewPort() const { return *view_port; }
    Buffer& getBuffer() const { return *buffer; }
    Cursor& getCursor() const { return *cursor; }    
 private:
    std::vector<std::shared_ptr<Graphic>> graphics;

    std::unique_ptr<ViewPort> view_port;
    std::unique_ptr<Buffer> buffer;
    std::unique_ptr<Cursor> cursor;
};
