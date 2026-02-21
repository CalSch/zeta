#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "emu.h"
#include "front_rl.h"
#include <signal.h>

unsigned int total_cycles = 0;

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

double get_current_time() {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9; // Convert nanoseconds to seconds
}

int main(int argc, char** argv) {
    double start_time = get_current_time();
    signal(SIGINT,sigint_handle);
    for (int i=0;i<argc;i++) {
        if (!strcmp(argv[i],"-rom")) {
            load_rom(argv[++i]);
        }
        else if (!strcmp(argv[i],"-dstate")) dbg_state = true;
        else if (!strcmp(argv[i],"-dmr")) dbg_memread = true;
        else if (!strcmp(argv[i],"-dmw")) dbg_memwrite = true;
        else if (!strcmp(argv[i],"-din")) dbg_ioread = true;
        else if (!strcmp(argv[i],"-dout")) dbg_iowrite = true;
    }
    start_raylib();
    setup_sector_table();
    setup_video();
    emu_init();

    while (running) {
        emu_tick();
    }

    double end_time = get_current_time();

    printf("\ncycles=%ld\n",ctx.tstates);
    printf("seconds=%lf\n",end_time-start_time);
    printf("mhz=%lf\n",(double)ctx.tstates/(end_time-start_time)/1000000);
    return 0;
}
