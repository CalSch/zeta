#include "stuff.h"

int cur=0;

void clear(void) __naked {
	__asm
		ld HL, #0xe000
		ld DE, #0xe001
		ld BC, #4800
		;ld BC, (_cur)
		;out (4), A
		ld (HL), #' '
		ldir
		ret
	__endasm;
}
void newline(void) {
	cur = cur - (cur%TXT_COLS) + TXT_COLS;
}
void putc(char c) {
	if (c=='\n')
		newline();
	else
		if (cur<sizeof(screen_buf))
			screen_buf[cur++] = c;
}
void puts(char* str) {
	while (*str)
		putc(*(str++));
}
void putbin(u8 x) {
	for (int i=7;i>=0;i--) {
		putc((x>>i)&1 ? '1' : '0');
	}
}
const char hextable[] = "0123456789ABCDEF";
void puthex8(u8 x) {
	putc(hextable[x>>4]);
	putc(hextable[x&0xf]);
}
void puthex16(u16 x) {
	puthex8(x>>8);
	puthex8(x&0xff);
}
void putdec(int n_) {
	int n=n_; // sdcc sucks
	if (n<0) {
		putc('-');
		putdec(-n);
		return;
	}
	static const u16 powers[] = {10000, 1000, 100, 10, 1};
	u8 i, digit, leading = 1;
	for (i = 0; i < 5; i++) {
		digit = 0;
		while (n >= powers[i]) {
			n -= powers[i];
			digit++;
		}
		if (digit || !leading || i == 4) {
			putc('0' + digit);
			leading = 0;
		}
	}
}


void memcpy(void* dest, void* src, u16 n) {
	while (n--)
		((u8*)dest)[n]=((u8*)src)[n];
}
void memcpy_asm(void* src, void* dest, u16 n) __naked {
	(void)src;
	(void)dest;
	(void)n;
	__asm
		push IX
		push BC
		
		ld IX, #6 ; n is pushed to the stack, offset by 6 to get past BC, IX, and PC
		add IX,sp
		ld C, (IX)
		ld B, 1(IX)

		ldir

		pop BC
		pop IX
		ret
	__endasm;
}

u16 strlen(char* str) {
	char* s;
	for (s=str;*s;s++)
		;
	return s-str;
}

int atoi(char* str) {
	int v=0;
	char* s = str;
	while (*s) {
		if (*s >= '0' && *s <= '9') {
			v *= 10;
			v += *s - '0';
		}
		s++;
	}
	if (str[0]=='-')
		v *= -1;
	return v;
}
