#pragma once
#include "emu.h"
#include "video.h"
#include <raylib.h>

extern bool draw_fps;

u8 get_kb_row(u8 row);

void start_raylib();
void end_raylib();
