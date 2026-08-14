#pragma once

#include <iostream>
#include <cstddef>

#include "raylib.h"
#include "./la.h"

constexpr unsigned char font_image_data[] {
#embed "charmap-oldschool_white.png"
};

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
