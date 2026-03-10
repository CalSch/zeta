#include "disks.h"

struct diskctx_t diskctx;

#define CURRENT_DISK diskctx.disks[diskctx.disk_idx]

const static int command_lengths[] = {
 [DISK_CMD_SET_DI           ] = 2,
 [DISK_CMD_SET_SI           ] = 3,
 [DISK_CMD_INC_SI           ] = 1,
 [DISK_CMD_DEC_SI           ] = 1,
 [DISK_CMD_GET_DI           ] = 1,
 [DISK_CMD_GET_SI           ] = 1,
 [DISK_CMD_GET_DISK_ATTR    ] = 1,
 [DISK_CMD_GET_DISK_SIZE_LO ] = 1,
 [DISK_CMD_GET_DISK_SIZE_HI ] = 1,
 [DISK_CMD_READ             ] = 1,
 [DISK_CMD_WRITE            ] = 1,
};

static void clear_cmdbuf() {
	for (int i=0;i<sizeof(diskctx.cmd_buf);i++)
		diskctx.cmd_buf[i] = 0;
}

//TODO: make errors better
void disk_do_cmd() {
	printf("doing command: %02x %02x %02x %02x\n",diskctx.cmd_buf[0],diskctx.cmd_buf[1],diskctx.cmd_buf[2],diskctx.cmd_buf[3]);
	switch (diskctx.cmd_buf[0]) {
		case DISK_CMD_SET_DI: diskctx.disk_idx = diskctx.cmd_buf[1]; break;
		case DISK_CMD_SET_SI: diskctx.sector_idx = (diskctx.cmd_buf[2] << 8) + diskctx.cmd_buf[1]; break;

		case DISK_CMD_INC_SI: diskctx.sector_idx++; break;
		case DISK_CMD_DEC_SI: diskctx.sector_idx--; break;

		case DISK_CMD_GET_DI: diskctx.readback = diskctx.disk_idx; break;
		case DISK_CMD_GET_SI: diskctx.readback = diskctx.sector_idx; break;
		case DISK_CMD_GET_DISK_ATTR: diskctx.readback = CURRENT_DISK.attrs; break;
		case DISK_CMD_GET_DISK_SIZE_LO: diskctx.readback = CURRENT_DISK.size & 0xff; break;
		case DISK_CMD_GET_DISK_SIZE_HI: diskctx.readback = CURRENT_DISK.size >> 8; break;
		
		case DISK_CMD_READ:
			if (CURRENT_DISK.attrs & 1) // is the disk there? TODO: set error
				if (CURRENT_DISK.size > diskctx.sector_idx) // is there space? TODO: set error
					memcpy(diskctx.buffer, CURRENT_DISK.data + (diskctx.sector_idx * SECT_SIZE), SECT_SIZE);
				else
					printf("to smol, size %d <= si %d\n",CURRENT_DISK.size, diskctx.sector_idx);
			else
				printf("aint there\n");
			break;

		case DISK_CMD_WRITE:
			if (CURRENT_DISK.attrs & 1) // is the disk there? TODO: set error
				if (CURRENT_DISK.size > diskctx.sector_idx) {// is there space? TODO: set error
					memcpy(CURRENT_DISK.data + (diskctx.sector_idx * SECT_SIZE), diskctx.buffer, SECT_SIZE);
					if (CURRENT_DISK.on_write != NULL) {
						CURRENT_DISK.on_write(CURRENT_DISK);
					}
				} else
					printf("no space\n");
				else
					printf("aint there\n");
			break;
	}
}

void disk_iowrite(u8 data) {
	if (diskctx.cmd_buf_idx == 4) {
		// TODO: error
		return;
	}
	diskctx.cmd_buf[diskctx.cmd_buf_idx] = data;
	diskctx.cmd_buf_idx++;
	
	/* printf("disk io write. cmdbuf  = %02x %02x %02x %02x\n",diskctx.cmd_buf[0],diskctx.cmd_buf[1],diskctx.cmd_buf[2],diskctx.cmd_buf[3]); */
	int cmdlen = command_lengths[diskctx.cmd_buf[0]];
	/* printf("cmdlen=%d\n",cmdlen); */
	if (cmdlen == diskctx.cmd_buf_idx) {
		disk_do_cmd();	
		diskctx.cmd_buf_idx = 0;
		clear_cmdbuf();
	}
}

u8 disk_ioread() {
	return diskctx.readback;
}

void disk_bufwrite(u16 addr, u8 data) {
	diskctx.buffer[addr] = data;
}
u8 disk_bufread(u16 addr) {
	return diskctx.buffer[addr];
}
