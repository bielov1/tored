#include "la.h"

Vec2f::Vec2f(float x, float y)
    : x{x}
    , y{y}
{}

Vec2f::Vec2f(float x)
    : x{x}
    , y{x}
{}

inline Vec2f Vec2f::add(Vec2f a, Vec2f b)
{
    return Vec2f{ a.x + b.x, a.y + b.y };
}

inline Vec2f Vec2f::sub(Vec2f a, Vec2f b)
{
    return Vec2f{ a.x - b.x, a.y - b.y };
}

inline Vec2f Vec2f::mul(Vec2f a, Vec2f b)
{
    return Vec2f{ a.x * b.x, a.y * b.y};
}

inline Vec2f Vec2f::div(Vec2f a, Vec2f b)
{
    return Vec2f{ a.x / b.x, a.y / b.y };
}
