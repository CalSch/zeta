#include "kb.h"

u8 old_kb_state[KB_MAX_ROW+1];
u8 kb_state[KB_MAX_ROW+1];
u8 kb_pressed[KB_MAX_ROW+1];

// rows go right to left. maybe ill change it later
char scancode2char[]       = "hgfedcbaponmlkjixwvutsrq543210zy    9876        ,';\\][=-     `/.";
char scancode2char_shift[] = "HGFEDCBAPONMLKJIXWVUTSRQ%$#@!)ZY    (*&^        <\":|}{+_     ~?>";

u8 get_kb_row(u8 row) __naked {
	(void)row; // get rid of the unused variable warning
	__asm
		; row is in A and the return value should be in A

		ld B, A ; A=row and is the 'argument' for the IN
		ld C, #3 ; 3 is the io address of the keyboard
		in A, (C)
		ret
	__endasm;
}

u8 is_key_down(u8 scancode) {
	return (kb_state[scancode/8]>>(scancode%8))&1;
}

