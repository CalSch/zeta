#include "libz80/z80.h"
#include "log.h"
#include "emu.h"
#include "video.h"
#include "front_rl.h"

bool running = true;

Z80Context ctx;

u8 rom[ROM_SIZE];
u8 ram[RAM_SIZE];

u16 sector_table[SECTORS] = {0}; // maps software sector index (0-7) to hardware sector index (0-65535)
u8 mapper_state[6] = {0};

bool dbg_state    = false;
bool dbg_memread  = false;
bool dbg_memwrite = false;
bool dbg_ioread   = false;
bool dbg_iowrite  = false;

bool dont_log_memreads = false; // overrides dbg_memread

void setup_sector_table() {
	sector_table[0] = 0;
	sector_table[1] = 512;
	sector_table[SECTORS-1] = VRAM_HWS_FIRST;
}

u8 memread(int param, u16 addr) {
	u8 val = 0;
	char dev[10] = "idk";

	int sect = addr / SECT_SIZE;
	int rel_addr = addr % SECT_SIZE;
	u16 hw_sect = sector_table[sect];

	if (dbg_memread && !dont_log_memreads)
		printf(" read($%04x / %5d) -> ", addr,addr);

	if (hw_sect == 0) {
		val = rom[rel_addr];
		strcpy(dev,"rom");
	} else if (512 <= hw_sect && hw_sect <= 1023) {
		val = ram[rel_addr-((hw_sect-512)*SECT_SIZE)];
		strcpy(dev,"ram");
	} else if (VRAM_HWS_FIRST <= hw_sect && hw_sect <= VRAM_HWS_LAST) {
		val = vram.data[rel_addr-((hw_sect-VRAM_HWS_FIRST)*SECT_SIZE)];
		strcpy(dev,"vram");
	}

	if (dbg_memread && !dont_log_memreads)
		printf("%s s:%d h:%d ra:%04x -> $%02x / %3d\n", dev,sect,hw_sect,rel_addr, val,val);
	return val;
}

void memwrite(int param, u16 addr, u8 val) {
	char dev[10] = "idk";

	int sect = addr / SECT_SIZE;
	int rel_addr = addr % SECT_SIZE;
	u16 hw_sect = sector_table[sect];

	if (dbg_memwrite)
		printf("write($%04x / %5d, $%02x / %3d) -> ", addr,addr, val,val);
	fflush(stdout);

	if (hw_sect == 0) {
		strcpy(dev,"rom");
	} else if (512 <= hw_sect && hw_sect <= 1023) {
		ram[rel_addr-((hw_sect-512)*SECT_SIZE)] = val;
		strcpy(dev,"ram");
	} else if (VRAM_HWS_FIRST <= hw_sect && hw_sect <= VRAM_HWS_LAST) {
		vram.data[rel_addr-((hw_sect-VRAM_HWS_FIRST)*SECT_SIZE)] = val;
		strcpy(dev,"vram");
	}

	if (dbg_memwrite)
		printf("%s s:%d h:%d ra:%04x\n", dev,sect,hw_sect,rel_addr);
}


u8 ioread(int param, u16 addr) {
	u8 val = 0;
	char dev[10] = "idk";

	u8 breg = addr >> 8;
	addr = addr & 0xff;

	if (addr == 0) {
		strcpy(dev,"stdio");
		val = getc(stdin);
	} else if (addr == 1) {
		strcpy(dev,"num");
		printf("num in: ");
		int m = scanf("%hhd",&val);
		// TODO: make this work
		/* while (m==0) { */
		/* 	printf("invalid. num: "); */
		/* 	m = scanf("%hhd",&val); */
		/* } */
	} else if (addr == 3) {
		u8 row = ctx.R1.br.B;
		strcpy(dev,"keyboard");
		val = get_kb_row(row);
	}

	if (dbg_ioread)
		LOG_INFO(" in($%02x%02x) -> %s -> $%02x / %3d", breg,addr, dev, val,val);

	return val;
}

void iowrite(int param, u16 addr, u8 val) {
	char dev[10] = "idk";

	u8 breg = addr >> 8;
	addr = addr & 0xff;

	if (addr == 0) {
		strcpy(dev,"stdio");
		putc(val,stdout);
	} else if (addr == 1) {
		strcpy(dev,"num");
		printf("num out: %d\n",val);
	} else if (addr == 2) {
		strcpy(dev,"mapper");
		// shift state left and add value to the end
		for (int i=0;i<5;i++)
			mapper_state[i]=mapper_state[i+1];
		mapper_state[5]=val;

		// now check for command
		if (mapper_state[3]==0xff && mapper_state[4]==0xff && mapper_state[5]==0xff) {
			// do mapper command
			if (mapper_state[0] >= SECTORS) {
				LOG_INFO("invalid mapper command! %02x %02x %02x",mapper_state[0],mapper_state[1],mapper_state[2]);
			} else {
				/* printf("map %d -> %d\n",mapper_state[0],mapper_state[1] + (mapper_state[2]<<8)); */
				sector_table[mapper_state[0]] = mapper_state[1] + (mapper_state[2]<<8);
			}
		}
	}

	if (dbg_iowrite)
		LOG_INFO("out($%02x%02x, $%02x / %3d) -> %s", breg,addr, val,val, dev);
}


void emu_init() {
	ctx.memRead = memread;
	ctx.memWrite = memwrite;
	ctx.ioRead = ioread;
	ctx.ioWrite = iowrite;
	Z80RESET(&ctx);
	LOG_INFO("setup");
}
void emu_tick() {
	if (dbg_state)
		print_cpu_state_inline();
	Z80Execute(&ctx);
}



char debug_str[300];
char dump_str[100];
char decode_str[100];
char stack_str[100];

void print_cpu_state_inline() {
	// Z80Context ctx = prev_ctx;
	memset(decode_str,0,sizeof(decode_str));
	memset(dump_str,0,sizeof(dump_str));

	dont_log_memreads = true;
	Z80Debug(&ctx,dump_str,decode_str);
	dont_log_memreads = false ;

	printf(
		"PC=%04x dump='%s' decode='%s' "
		"AF=%04x BC=%04x DE=%04x HL=%04x SP=%04x"
		"\n"
		,
		ctx.PC,
		dump_str,
		decode_str,

		ctx.R1.wr.AF,
		ctx.R1.wr.BC,
		ctx.R1.wr.DE,
		ctx.R1.wr.HL,
		ctx.R1.wr.SP
	);
}
