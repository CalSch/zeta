#pragma once
#include "emu.h"

// disk buffer HW sector index
#define DISK_BUF_HWS 1

#define DISK_CMD_SET_DI           0x01
#define DISK_CMD_SET_SI           0x02
#define DISK_CMD_INC_SI           0x03
#define DISK_CMD_DEC_SI           0x04
#define DISK_CMD_GET_DI           0x05
#define DISK_CMD_GET_SI           0x06
#define DISK_CMD_GET_DISK_ATTR    0x07
#define DISK_CMD_GET_DISK_SIZE_LO 0x08
#define DISK_CMD_GET_DISK_SIZE_HI 0x09
#define DISK_CMD_READ             0x0a
#define DISK_CMD_WRITE            0x0b

#define MAX_DISKS 256

typedef struct disk_t {
    u8 attrs;
    u16 size; // 8k sector count
    u8 *data;

    char *fname; // file name of the file corresponding to the disk if there is one

    void (*on_write)(struct disk_t disk);
} disk_t;

struct diskctx_t{
    disk_t disks[MAX_DISKS];

    u8 readback;
    u8 disk_idx;
    u16 sector_idx;

    u8 cmd_buf[4];
    u8 cmd_buf_idx;
    
    u8 buffer[SECT_SIZE];
};
// TODO: rename to something shorter, like dc or dctx?
extern struct diskctx_t diskctx;


/* static void clear_cmdbuf(); */

// TODO: probably rename to do_disk_cmd()
void disk_do_cmd();

void disk_iowrite(u8 data);
u8 disk_ioread();
void disk_bufwrite(u16 addr, u8 data);
u8 disk_bufread(u16 addr);
