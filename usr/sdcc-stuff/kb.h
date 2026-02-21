#include "stuff.h"

extern u8 old_kb_state[KB_MAX_ROW+1];
extern u8 kb_state[KB_MAX_ROW+1];
extern u8 kb_pressed[KB_MAX_ROW+1];

// rows go right to left. maybe ill change it later
extern char scancode2char[];
extern char scancode2char_shift[];

u8 get_kb_row(u8 row);
u8 is_key_down(u8 scancode);


#define KEY_SHIFT		SCN(4,4)
#define KEY_CTRL 		SCN(5,4)
#define KEY_ALT 		SCN(6,4)
#define KEY_ESC 		SCN(7,4)

#define KEY_BACKSPACE	SCN(0,5)
#define KEY_DEL 		SCN(1,5)
#define KEY_ENTER		SCN(2,5)
#define KEY_TAB 		SCN(3,5)
#define KEY_UP 			SCN(4,5)
#define KEY_DOWN 		SCN(5,5)
#define KEY_LEFT 		SCN(6,5)
#define KEY_RIGHT 		SCN(7,5)

#define KEY_SPACE 		SCN(3,7)
