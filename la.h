#pragma once

struct Vec2f
{
    Vec2f(float x, float y);
    Vec2f(float x);
    
    Vec2f add(Vec2f a, Vec2f b);
    Vec2f sub(Vec2f a, Vec2f b);
    Vec2f mul(Vec2f a, Vec2f b);
    Vec2f div(Vec2f a, Vec2f b);
    
    float x, y;
};

