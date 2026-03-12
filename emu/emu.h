#pragma once
#include "libz80/z80.h"
#include "log.h"
#include <stdbool.h>

#define ADDR_SPACE 65536 // 2^16
#define SECT_SIZE 8192 // 2^13
#define SECTORS (ADDR_SPACE/SECT_SIZE)

#define ROM_SIZE SECT_SIZE
#define RAM_SIZE (SECT_SIZE*512)

#define CALLSTACK_SIZE 256

#define ARRAY_LEN(a) (sizeof(a)/sizeof(a[0]))

typedef unsigned char u8;
typedef unsigned short u16;

extern bool running;

extern Z80Context ctx;

extern u8 rom[ROM_SIZE];
extern u8 ram[RAM_SIZE];

extern u16 sector_table[SECTORS]; // maps software sector index (0-7) to hardware sector index (0-65535)


extern bool dbg_state;
extern bool dbg_memread;
extern bool dbg_memwrite;
extern bool dbg_ioread;
extern bool dbg_iowrite;
extern bool dbg_callstack;


void setup_sector_table();

u8 memread(int param, u16 addr);
void memwrite(int param, u16 addr, u8 val);

u8 ioread(int param, u16 addr);
void iowrite(int param, u16 addr, u8 val);

void emu_init();
void emu_tick();

void print_cpu_state_inline();

typedef struct {
    bool is_int;
    u16 from;
    u16 to;
} call_t;
extern call_t call_stack[CALLSTACK_SIZE];
extern u16 call_stack_size;

typedef struct {
    u16 addr;
    char name[64];
} debug_label_t;

extern debug_label_t debug_labels[1024];
extern int debug_labels_count;
char* addr2str(u16 addr);
