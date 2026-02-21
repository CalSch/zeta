#pragma once
#include "emu.h"

#define TXT_COLS 80
#define TXT_ROWS 60

#define VRAM_HWS_FIRST 8 // "first vram HardWare Sector"
#define VRAM_HWS_LAST 15

struct vid_txt_state {
    char text[TXT_COLS*TXT_ROWS];
    u8 fgs[TXT_COLS*TXT_ROWS];
    u8 bgs[TXT_COLS*TXT_ROWS];
};

struct vid_state {
    struct vid_txt_state txt;
};

typedef union vram_t {
    struct vid_state state;
    u8 data[sizeof(struct vid_state)];
} vram_t;

void setup_video();

extern vram_t vram;


