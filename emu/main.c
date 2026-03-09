#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "emu.h"
#include "disks.h"
#include "front_rl.h"
#include <signal.h>
#include <unistd.h>
#include <time.h>

/* #define BATCH_CYCLES 1000*10 */
/* #define TARGET_HZ (1000*1000*10) // 10mhz */

void load_debug_labels(const char* filename) {
    FILE* f = fopen(filename, "r");
    if (!f) return;
    debug_labels_count = 0;
    while (debug_labels_count < 1024 &&
           fscanf(f, "%hx %63s", &debug_labels[debug_labels_count].addr,
                                  debug_labels[debug_labels_count].name) == 2) {
        debug_labels_count++;
    }
    fclose(f);
    printf("loaded %d debug labels\n",debug_labels_count);
    printf("%d '%s'\n",debug_labels[0].addr,debug_labels[0].name);
}

void sigint_handle(int sig) {
    running = false;
}

void load_rom(char* filename) {
    FILE* f = fopen(filename,"r");

    if (f == NULL) {
        perror("open(rom_file)");
        exit(1);
    }

    int i=0;
    for (;i<ROM_SIZE;i++) {
        int c = fgetc(f);
        if (c != EOF) {
            rom[i]=c;
        } else {
            break;
        }
    }
    LOG_INFO("read %d bytes of rom",i);

    fclose(f);
}

u8 get_free_disk_idx() {
    // TODO: error if no disk found, currently will loop forever
    u8 idx = 0;
    while (diskctx.disks[idx].attrs & 1)
        idx++;
    return idx;
}

void right_to_dih(disk_t disk) {
    printf("writing to dih '%s'...\n", disk.fname);
    FILE *f = fopen(disk.fname, "w");
    if (f == NULL) {
        perror("open(dih)");
        exit(1);
    }

    /* printf("disk: %.3s\n", disk.data); */
    /* printf("buf : %.3s\n", diskctx.buffer); */
    fwrite(disk.data, disk.size * SECT_SIZE, 1,  f);

    fclose(f);
}

void add_disk(char* name) {
    printf("opening file '%s'...\n",name);
    FILE* f = fopen(name,"r");
    if (f == NULL) {
        perror("open(disk_file)");
        exit(1);
    }

    disk_t d = {0};
    d.fname = name;
    d.on_write = right_to_dih;
    d.attrs = 1; // yeah its there
    
    // figure out dat size
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);

    d.size = (size + SECT_SIZE - 1) / SECT_SIZE;

    d.data = malloc(d.size * SECT_SIZE);
    memset(d.data, 0, d.size * SECT_SIZE);

    fread(d.data, size, 1, f);

    u8 disk_idx = get_free_disk_idx();

    diskctx.disks[disk_idx] = d;

    printf("added disk %d: %ld bytes (%d sectors)\n", disk_idx, size, d.size);
}

double get_current_time() {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9; // Convert nanoseconds to seconds
}

int main(int argc, char** argv) {
    double start_time = get_current_time();
    signal(SIGINT,sigint_handle);
    float target_mhz = 10;
    unsigned int batch_cycles = 1000*10;
    for (int i=0;i<argc;i++) {
        if (!strcmp(argv[i],"-rom")) {
            load_rom(argv[++i]);
        }
        else if (!strcmp(argv[i],"-labels")) {
            load_debug_labels(argv[++i]);
        }
        else if (!strcmp(argv[i],"-addr2str")) {
            int inp = atoi(argv[++i]);
            u16 addr = (u16)inp;
            printf("%d -> $%04X -> %s\n",inp,addr,addr2str(addr));
            exit(0);
        }
        else if (!strcmp(argv[i],"-mhz")) target_mhz = atof(argv[++i]);
        else if (!strcmp(argv[i],"-bc")) batch_cycles = atoi(argv[++i]);
        else if (!strcmp(argv[i],"-dstate")) dbg_state = true;
        else if (!strcmp(argv[i],"-dmr")) dbg_memread = true;
        else if (!strcmp(argv[i],"-dmw")) dbg_memwrite = true;
        else if (!strcmp(argv[i],"-din")) dbg_ioread = true;
        else if (!strcmp(argv[i],"-dout")) dbg_iowrite = true;
        else if (!strcmp(argv[i],"-dcs")) dbg_callstack = true;
        else if (!strcmp(argv[i],"-fps")) draw_fps = true;
        else if (!strcmp(argv[i],"-disk")) {
            add_disk(argv[++i]);
        }
    }



    printf("target_mhz=%f\n",target_mhz);
    printf("batch_cycles=%d\n",batch_cycles);
    start_raylib();
    setup_sector_table();
    setup_video();
    emu_init();

    struct timespec batch_start, now;
    clock_gettime(CLOCK_MONOTONIC, &batch_start);

    while (running) {
        unsigned long cycles_before = ctx.tstates;
        while (ctx.tstates < cycles_before+batch_cycles) {
            emu_tick();
        }

        // do sleepy time (thanks claude)
        long expected_ns = (long)((double)batch_cycles / (target_mhz*1000*1000) * 1e9);

        clock_gettime(CLOCK_MONOTONIC, &now);
        long elapsed_ns = (now.tv_sec - batch_start.tv_sec) * 1000000000L
                        + (now.tv_nsec - batch_start.tv_nsec);

        long sleep_ns = expected_ns - elapsed_ns;
        if (sleep_ns > 0) {
            struct timespec ts = { 0, sleep_ns };
            nanosleep(&ts, NULL);
        }
        /* printf("sleep_ns=%ld\n",sleep_ns); */

        clock_gettime(CLOCK_MONOTONIC, &batch_start);
    }

    double end_time = get_current_time();

    printf("\ncycles=%ld\n",ctx.tstates);
    printf("seconds=%lf\n",end_time-start_time);
    printf("mhz=%lf\n",(double)ctx.tstates/(end_time-start_time)/1000000);
    return 0;
}
