#pragma once
#include <vector>
#include <memory>

class Graphic
{
public:
    virtual ~Graphic();
    
    virtual void draw(Font font) = 0;
    
    virtual add(std::shared_ptr<Graphic> component) {}
    virtual remove(std::shared_ptr<Graphic> component) {}
};

using Line = std::pair<std::size_t, std::string>;
using Text = std::vector<Line>;

struct ViewPort
{
    int first_visible_line;
    int first_visible_col;
    int visible_lines;
    int visible_cols;
};


class BufferView : public Graphic
{
public:
    BufferView(ViewPort vp, std::shared_ptr<Buffer> b, std::shared_ptr<Cursor> c) 
        : view_port{vp}
	, buffer(b)
	, cursor(c)
    {}
    
    void draw(Font font) override;
    ViewPort& getViewPort() { return view_port; }
private:    
    ViewPort view_port;
    std::shared_ptr<Buffer> buffer;
    std::shared_ptr<Curosr> cursor;
};

class Buffer// , public Observer
{
public:
    Buffer(std::string text)
	: text{}
    {
	text.emplace_back(text.size(), std::move(text));
    }

    Text& getText() { return text; }
    //void update() override;
private:
    Text text;
};

class Cursor//, public Observer
{
public:
    std::size_t getLine() const { return line_idx; }
    std::size_t getCol() const { return col_idx; }
    //void move(std::size_t line, std::size_t col) { line_idx = line; col_idx = col; }
    //void update() override;

private:
    std::size_t line_idx = 0;
    std::size_t col_idx  = 0;
}

// TODO: potential classes to add
// class ScrollBar : public Graphic
// {
// };

// class InfoLine : public Graphic
// {

// };

class Window : public Graphic
{
public:
    ~Window() override = default;

    void draw(Font font) override {
        for (auto& child : graphics) {
            child->draw(font);
        }
    }
    add(std::shared_ptr<Graphic> component) override {
	graphics.push_back(component);
    }
    //remove(std::shared_ptr<Graphic> component) override {}
 private:
    std::vector<std::shared_ptr<Graphic>> graphics;
};
