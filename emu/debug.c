#include "debug.h"
#include "emu.h"
#include <string.h>

const char* WATCHVAR_TYPE_STRINGS[] = {
    "u8","u16","char","str"
};
const char* WATCHVAR_TYPE_FORMATS[] = {
	// the `1$` means "use the 1st argument" (instead of the 2nd, which might not exist)
    "%02x / %1$3d","%04x / %1$5d","'%c'","\"%s\""
};
int WATCHVAR_TYPE_SIZES[] = {
    1,2,1,2
};

watchvar_t dbg_watchvars[MAX_WATCHVARS];
u16 dbg_watchvar_count;
u8 dbg_mem_snapshot[ADDR_SPACE];

char watchvar_str_buf[1024];
char watchvar_temp_buf[256];

void add_watchvar(watchvar_t var) {
    if (dbg_watchvar_count >= MAX_WATCHVARS) {
        printf("gah! too many watchvars!");
        /* exit(1); */
        // TODO: real error?
        return;
    }
    dbg_watchvars[dbg_watchvar_count] = var;
    dbg_watchvar_count++;
}

void update_mem_snapshot() {
    dont_log_memreads = true;
    for (int i=0;i<ADDR_SPACE;i++)
        dbg_mem_snapshot[i] = memread(0, i);
    dont_log_memreads = false;
}

// converts a variable to a string and puts it in watchvar_temp_buf
void watchvar_to_string(enum watchvar_type type, u16 addr) {
    char* ptr = watchvar_temp_buf;
    *ptr = 0; // empty the string

    u8 lo = dbg_mem_snapshot[addr];
    u8 hi = dbg_mem_snapshot[addr+1];

    int size = WATCHVAR_TYPE_SIZES[type];
    const char* fmt = WATCHVAR_TYPE_FORMATS[type];

    u16 val = (size == 2) ? ((hi<<8) + lo) : lo;

    if (type == WV_STR) {
        // strings need char pointers, so take one from the snapshot
        ptr += sprintf(ptr, fmt, dbg_mem_snapshot+val);
    } else {
        ptr += sprintf(ptr, fmt, val);
    }
}

void update_watchvar_str() {
    char* ptr = watchvar_str_buf;
    *ptr = 0; // empty the string
    for (int i=0;i<dbg_watchvar_count;i++) {
        watchvar_t v = dbg_watchvars[i];
        // TODO: convert to snprintf
        ptr += sprintf(ptr, "%s = ", v.name);
        if (v.count > 1) {
            ptr += sprintf(ptr, "%s[%d] :\n", WATCHVAR_TYPE_STRINGS[v.type], v.count);
            for (int i=0;i<v.count;i++) {
                u16 addr = v.addr + i*WATCHVAR_TYPE_SIZES[v.type];
                
                watchvar_to_string(v.type, addr); // converts a variable to a string and puts it in watchvar_temp_buf
                ptr += sprintf(ptr, "  [%3d:%04x]=%s\n", i, addr, watchvar_temp_buf);
            }
        } else {
            watchvar_to_string(v.type, v.addr);
            ptr += sprintf(ptr, "(%s) %s\n", WATCHVAR_TYPE_STRINGS[v.type], watchvar_temp_buf);
        }
    }
}

watchvar_t parse_watchvar_str(char* s) {
	watchvar_t v;
	char* name_s = s;
	char* type_s = strchr(s,':')+1;
	char* count_s = strchr(type_s,':')+1;
	char* addr_s = strchr(count_s,':')+1;
	// separate into different strings (turn ':'s into '\0's)
	*(type_s-1) = 0;
	*(count_s-1) = 0;
	*(addr_s-1) = 0;

	sscanf(name_s,"%31s", v.name);
	sscanf(count_s,"%hd", &v.count);
	sscanf(addr_s,"%hx", &v.addr);

	v.type = -1;
	for (int i=0;i<sizeof(WATCHVAR_TYPE_STRINGS)/sizeof(WATCHVAR_TYPE_STRINGS[0]);i++) {
		if (!strcmp(WATCHVAR_TYPE_STRINGS[i], type_s)) {
			v.type = i;
			break;
		}
	}

	if (v.type == -1) {
		printf("WARNING: watchvar type '%s' is unknown\n",type_s);
	}

	return v;
}
