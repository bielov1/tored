#pragma once

#include <iostream>
#include <cstddef>
#include <cassert>
#include <span>
#include <print>

#include "raylib.h"
#define GRAPHICS_API_OPENGL_33 
#include "rlgl.h"
#include "GLFW/glfw3.h"
//#define STB_IMAGE_IMPLEMENTATION
//#include "stb_image.h"
#include "./la.h"

// constexpr unsigned char file_data[] {
// #embed "charmap-oldschool_white.png"
// };

extern "C" {
    extern const unsigned char _binary_charmap_oldschool_white_png_start[];
    extern const unsigned char _binary_charmap_oldschool_white_png_end[];
}

static const int SCREEN_WIDTH = 800;
static const int SCREEN_HEIGHT = 600;

static const int FONT_WIDTH = 128;
static const int FONT_HEIGHT = 64;
static const int FONT_COLS = 18;
static const int FONT_ROWS = 7;
static const int FONT_CHAR_WIDTH = (FONT_WIDTH / FONT_COLS);
static const int FONT_CHAR_HEIGHT = (FONT_HEIGHT / FONT_ROWS);
static const int FONT_SCALE = 2;

static const int BUFFER_CAP = 1024;
