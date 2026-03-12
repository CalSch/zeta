

typedef unsigned char u8;
typedef unsigned int u16;

#define NULL 0

#define countof(a) (sizeof(a)/sizeof(a[0]))

#define TXT_COLS 80
#define TXT_ROWS 60

#define KB_MAX_ROW 7

#define SCN(col,row) (row*8+(7-col))

#define PRINTd(x) puts(#x "=");putdec((int)x);newline();

__at(0xe000) char screen_buf[TXT_COLS*TXT_ROWS];
__sfr __at(0) io_stdio;
__sfr __at(1) io_nums;
__sfr __at(2) io_mapper;

extern int cur;


void clear(void);
void newline(void);
void putc(char c);
void puts(char* str);
void putbin(u8 x);
void puthex8(u8 x);
void puthex16(u16 x);
void putdec(int n_);

void memcpy_asm(void* src, void* dest, u16 n);
void memcpy(void* dest, void* src, u16 n);
//#define memcpy(dst,src,n) (memcpy_asm(src,dst,n), dst)

u16 strlen(char* str);
int atoi(char* str);
