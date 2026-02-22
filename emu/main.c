#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "emu.h"
#include "front_rl.h"
#include <signal.h>
#include <unistd.h>
#include <time.h>

/* #define BATCH_CYCLES 1000*10 */
/* #define TARGET_HZ (1000*1000*10) // 10mhz */

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
    float target_mhz = 10;
    unsigned int batch_cycles = 1000*10;
    for (int i=0;i<argc;i++) {
        if (!strcmp(argv[i],"-rom")) {
            load_rom(argv[++i]);
        }
        else if (!strcmp(argv[i],"-mhz")) target_mhz = atof(argv[++i]);
        else if (!strcmp(argv[i],"-bc")) batch_cycles = atoi(argv[++i]);
        else if (!strcmp(argv[i],"-dstate")) dbg_state = true;
        else if (!strcmp(argv[i],"-dmr")) dbg_memread = true;
        else if (!strcmp(argv[i],"-dmw")) dbg_memwrite = true;
        else if (!strcmp(argv[i],"-din")) dbg_ioread = true;
        else if (!strcmp(argv[i],"-dout")) dbg_iowrite = true;
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
