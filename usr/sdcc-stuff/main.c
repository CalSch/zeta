
#include "stuff.h"
#include "kb.h"

int x,y,z;

char input_str[100];

void run_input(void) {
	switch (input_str[0]) {
	case 'P':
		z = y;
		y = x;
		x = atoi(input_str+1);
		break;
	case 'p':
		x = y;
		y = z;
		z = 0;
		break;
	case '+':
		x = y + x;
		y = z;
		z = 0;
		break;
	case '-':
		x = y - x;
		y = z;
		z = 0;
		break;
	case '*':
		x = y * x;
		y = z;
		z = 0;
		break;
	case '/':
		x = y / x;
		y = z;
		z = 0;
		break;
	} 
	clear();
}

void special_keypress(u8 scancode) {
	switch (scancode) {
	case KEY_ENTER: // enter
		run_input();
		input_str[0]=0;
		break;
	case KEY_BACKSPACE:
		input_str[strlen(input_str)-1]=0;
		break;
	}
}

void keypress(u8 scancode) {
	clear();
	char c;
	if (is_key_down(SCN(4,4)))
		c = scancode2char_shift[scancode];
	else
		c = scancode2char[scancode];
	
	if (c == ' ' && scancode != SCN(3,7)) {
		special_keypress(scancode);
		return;
	}

	u16 len = strlen(input_str);
	input_str[len]=c;
	input_str[len+1]=0;
}

void end_frame(void) {
	cur = TXT_COLS;

	/* putbin(t); */
	putc(' ');
	newline();
	newline();
	
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


	newline();
	newline();

	putc('Z');
	putc('=');
	putdec(z);
	newline();

	putc('Y');
	putc('=');
	putdec(y);
	newline();

	putc('X');
	putc('=');
	putdec(x);
	newline();

	newline();
	putdec(strlen(input_str));
	putc(':');
	putc('<');
	puts(input_str);
	putc('>');


	// END OF FRAME

	// save kb state
	for (int i=0;i<=KB_MAX_ROW;i++)
		old_kb_state[i]=kb_state[i];

	// wait lol
	/* for (unsigned int i=0;i<1<<15;i++) */
	/* for (unsigned int j=0;j<5;j++) {} */
	/* t++; */
}

void setup(void) {
	for (int i=0;i<KB_MAX_ROW;i++)
		old_kb_state[i]=0;



	int t=0;
	while (1) {
	}
}
