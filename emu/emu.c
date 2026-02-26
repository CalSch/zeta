#include "libz80/z80.h"
#include "log.h"
#include "emu.h"
#include "video.h"
#include "front_rl.h"

void debug_dump();

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
	} else if (addr == 4) {
		debug_dump();
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



char debug_str[800];
char dump_str[100];
char decode_str[100];
char stack_str[200];

void print_cpu_state_inline() {
	// Z80Context ctx = prev_ctx;
	memset(decode_str,0,sizeof(decode_str));
	memset(dump_str,0,sizeof(dump_str));

	dont_log_memreads = true;
	Z80Debug(&ctx,dump_str,decode_str);
	dont_log_memreads = false ;

	printf(
		"PC=%s dump='%s' decode='%s' "
		"AF=%04x BC=%04x DE=%04x HL=%04x SP=%04x"
		"\n"
		,
        addr2str(ctx.PC),
		/* ctx.PC, */
		dump_str,
		decode_str,

		ctx.R1.wr.AF,
		ctx.R1.wr.BC,
		ctx.R1.wr.DE,
		ctx.R1.wr.HL,
		ctx.R1.wr.SP
	);
}


// Use `debug_str` to store a string of the cpu state
char* get_cpu_state() {
	dont_log_memreads = true;

    for (int i=0;i<100;i++) {
        dump_str[i]=0;
        decode_str[i]=0;
        stack_str[i]=0;
    }
    memset(debug_str,0,sizeof(debug_str));
    Z80Debug(&ctx,dump_str,decode_str);
    char* ptr = stack_str;
    if (ctx.R1.wr.SP != 0) {
        // printf("making stack str\n");
        for (int i=ctx.R1.wr.SP;i<ctx.R1.wr.SP+64;i++) {
			if (i%0x10 == 0) {
				ptr += sprintf(ptr,"\n%04X | ",i);
			}
            // printf("i=%04X\n",i);
            ptr += sprintf(ptr,"%02X ",memread(0,i));
            if ((ptr-stack_str)+3 >= 300) {
                break;
            }
        }
    }

	dont_log_memreads = false;

    sprintf(debug_str,
        "PC=%s dump='%6s' decode='%s'             \n"
        "AF =%04X BC =%04X DE =%04X HL =%04X IX =%04X IY =%04X SP =%04X\n"
        "AF'=%04X BC'=%04X'DE =%04X HL'=%04X IX'=%04X IY'=%04X SP'=%04X\n"
        "Stack: %s\n"
        ,
        addr2str(ctx.PC),
		/* ctx.PC, */
        dump_str,
        decode_str,

        ctx.R1.wr.AF,
        ctx.R1.wr.BC,
        ctx.R1.wr.DE,
        ctx.R1.wr.HL,
        ctx.R1.wr.IX,
        ctx.R1.wr.IY,
        ctx.R1.wr.SP,

        ctx.R2.wr.AF,
        ctx.R2.wr.BC,
        ctx.R2.wr.DE,
        ctx.R2.wr.HL,
        ctx.R2.wr.IX,
        ctx.R2.wr.IY,
        ctx.R2.wr.SP,

        stack_str
    );

    return debug_str;
}


void debug_dump() {
	printf("\n\n========== DEBUG DUMP ===========\n");
	get_cpu_state();
	puts(debug_str);
	printf("\n\n");
}


Label debug_labels[1024];
int debug_labels_count = 0;
char label_rel_buf[64];

char* addr2str(u16 addr) {
    if (debug_labels_count == 0) {
        snprintf(label_rel_buf, sizeof(label_rel_buf), "$%04X", addr);
        return label_rel_buf;
    }
    u16 last_one = 0;
    char* last_name = debug_labels[0].name;
    for (int i = 0; i < debug_labels_count; i++) {
        if (debug_labels[i].addr > addr)
            break;
        last_one = debug_labels[i].addr;
        last_name = debug_labels[i].name;
    }
    snprintf(label_rel_buf, sizeof(label_rel_buf), "%.20s+$%X ($%04X)", last_name, addr - last_one, addr);
    return label_rel_buf;
}


