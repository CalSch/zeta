#include "video.h"

void setup_video() {
	memset(vram.state.txt.text,' ',sizeof(vram.state.txt.text));
}

vram_t vram;
