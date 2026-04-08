
#include "stuff.h"
#include "kb.h"
#include "malloc.h"

/* u16 counter = 0; */
int count_a = 0;
int count_b = 0;

void special_keypress(u8 scancode) {
	switch (scancode) {
	case KEY_ENTER: // enter
		break;
	case KEY_BACKSPACE:
		break;
	}
}

void keypress(u8 scancode) {
	char c;
	if (is_key_down(SCN(4,4)))
		c = scancode2char_shift[scancode];
	else
		c = scancode2char[scancode];
	
	if (c == ' ' && scancode != SCN(3,7)) {
		special_keypress(scancode);
		return;
	}

}

void redraw(void) {
	/* clear(); */
	cur = 0;
	PRINTd(count_a);
	PRINTd(count_b);
}

// called by init.s on the end frame interrupt
void end_frame(void) {
	/* io_debug=0; */
	io_stdio='.';
	cur = 0;

	// get kb state
	for (int i=0;i<=KB_MAX_ROW;i++) {
		u8 d = get_kb_row(i);
		kb_state[i] = d;
		kb_pressed[i] = ~old_kb_state[i] & d;
		/* putbin(d); */
		/* newline(); */
	}


	for (int i=0;i<=KB_MAX_ROW;i++) {
		if (kb_pressed[i]) {
			for (int j=0;j<8;j++) {
				if ((kb_pressed[i]>>j)&1) {
					keypress(j+i*8);
				}
			}
		}
	}

	redraw();

	// END OF FRAME

	// save kb state
	for (int i=0;i<=KB_MAX_ROW;i++)
		old_kb_state[i]=kb_state[i];


}

void setup(void) {
	cur = TXT_COLS;

	/* setup_malloc(); */

	// reads sector from disk
	/* __asm */
	/* 	ld A, #0xA */
	/* 	out (5), A */
	/* 	ld A, #0 */
	/* 	ld (0xbfff), A */
	/* __endasm; */

}

void thingy(void) {
	while (1) {
		/* for (u16 i=0;i<0x00ff;i++) {} */
		/* count_b*=count_a; */
		count_b++;
	}
}
