
#include "stuff.h"
#include "kb.h"
#include "malloc.h"

int line_count = 0;
char** lines = NULL;

u16 counter = 0;

void load_string(char* str) {
	// step 1: find line count
	u16 len = strlen(str);
	PRINTd(len);

	line_count = 1;
	for (u16 i=0;i<len;i++)
		if (str[i] == '\n')
			line_count++;

	PRINTd(line_count);

	// step 2: allocate lines
	lines = malloc(sizeof(char*) * line_count);

	// step 3: read each line
	u16 line_len = 0;
	u16 current_line = 0;
	char* current_line_start = str;
	for (u16 i=0;i<len;i++) {
		if (str[i] == '\n') {
			lines[current_line] = malloc(line_len+1); // allocate line
			if (lines[current_line] == NULL) {
				line_count = current_line-1;
				return;
			}
			memcpy(lines[current_line], current_line_start, line_len); // copy line
			lines[current_line][line_len] = 0; // null terminator

			// reset state and advance line
			line_len = 0;
			current_line++;
			current_line_start = str+i+1;
			putc('.');
		} else {
			line_len++;
		}
	}

	// step 3.5: save the last line
	lines[current_line] = malloc(line_len+1); // allocate line
	memcpy(lines[current_line], current_line_start, line_len); // copy line
	lines[current_line][line_len] = 0; // null terminator

	// step 4: nothing, ur done
}

void special_keypress(u8 scancode) {
	switch (scancode) {
	case KEY_ENTER: // enter
		/* run_input(); */
		/* input_str[0]=0; */
		break;
	case KEY_BACKSPACE:
		/* input_str[strlen(input_str)-1]=0; */
		break;
	}
}

void keypress(u8 scancode) {
	/* clear(); */
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
	clear();
	cur = 0;
}

// called by init.s on the end frame interrupt
void end_frame(void) {
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

	__asm
		;di
	__endasm;

	setup_malloc();

	/* load_string("hello\nwold!\n line 3"); */
	__asm
		ld A, #0xA
		out (5), A
		ld A, #0
		ld (0xbfff), A
	__endasm;

	load_string((char*)0xa000);

	newline();
	newline();

	for (int i=0;i<line_count;i++) {
		if (!(i<line_count))
			break;
		putdec(i);
		puts(" at ");puthex16((u16)lines[i]);
		puts(" [");putdec(strlen(lines[i]));
		puts("] : ");
		puts(lines[i]);
		newline();
	}

	PRINTd(counter);


	int t=0;
	while (1) {
	}
}
