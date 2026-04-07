#pragma once
#include "emu.h"

#define MAX_WATCHVARS 256
#define WATCHVAR_NAME_LEN 32

enum watchvar_type {
    WV_U8,
    WV_U16,
    WV_CHAR,
    WV_STR,
};
extern const char* WATCHVAR_TYPE_STRINGS[];
extern const char* WATCHVAR_TYPE_FORMATS[];
extern int WATCHVAR_TYPE_SIZES[];

typedef struct {
    u16 addr;
    enum watchvar_type type;
    u16 count;
    char name[WATCHVAR_NAME_LEN];
} watchvar_t;

extern watchvar_t dbg_watchvars[MAX_WATCHVARS];
extern u16 dbg_watchvar_count;
extern u8 dbg_mem_snapshot[ADDR_SPACE];

extern char watchvar_str_buf[1024];
extern char watchvar_temp_buf[256];

void add_watchvar(watchvar_t var);

void update_mem_snapshot();

// converts a variable to a string and puts it in watchvar_temp_buf
void watchvar_to_string(enum watchvar_type type, u16 addr);

void update_watchvar_str();

watchvar_t parse_watchvar_str(char* s);
