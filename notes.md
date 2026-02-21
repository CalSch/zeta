
resolution: 640x480
256 colors

each process has some memory banks. at least one. no shared memory

- needs ipc
- filesystem

memory!!
- 256 bytes = 1 page
- 32 pages / 8k = 1 sector
- 8 sectors = 1 book / 64k
- each of the 8 sectors can be swapped out for any other
- at boot, sector 0 is mapped to ROM
- how many total sectors of RAM?
	- how bout 512
	- thats 4m of RAM or 2^22 bytes
- sw sector: 8k in addr space (ex. 0x2000-0x4000)
- hw sector: 8k on a physical chip (ex. RAM, ROM, VRAM, etc.)
	- indexed with 2 bytes
	- need to figure out what the indicies are
		- idea: ROM=0 MISC=1-511 RAM=512-1023
			- yeah sure
			- what is misc?
				- 1=disk buffer
				- 8-15=vram
- in the future, userspace programs will allocate sectors and the kernel will manage swapping, so hopefully there's not much manual swapping in the c code
- at boot, the mapper starts like this:
	- 0 = ROM
	- 1 = RAM[0]
	- 2-6 = ROM (maybe smth else later)
	- 7 = VRAM[0]  (which contains the text buffer so you can print without swapping stuff)



io!!!
- devices:
	- 0: stdio, getc & putc
	- 1: numbers, printf & scanf %d
	- 2: mapper
		- z80 sends: xx yy yy ff ff ff
			- xx = sw sector (0-7)
			- yy = hw sector (0-65535) (low byte first)
			- FF's are to tell the mapper that the command is done. this prevents synchronization issues i think
	- 3: keyboard
		- see keyboard section


keyboard!!!!!
- key matrix
- when running `in A, (C)`, set C to 3 and B to the keyboard row to look at
- this is how the c64 and zx spectrum work.
matrix:
   0  1  2  3  4  5  6  7
00 A  B  C  D  E  F  G  H
01 I  J  K  L  M  N  O  P
02 Q  R  S  T  U  V  W  X
03 Y  Z  0  1  2  3  4  5
04 6  7  8  9  sf ct al es
05 bk de en ta up dn le ri
06 -  =  [  ]  \  ;  '  ,
07 .  /  `  sp ## ## ## ##

video!!!!
- vram
	- yes
- and colors
	- oh yeah
- and like shapes and stuff
	- you betcha
- oh i forgot text
	- oopsies
- in vram, is the text screen seperate from the other mode, or is it unionized
- vram sectors:
	- 1th one: video card state
		```c
		struct gfx_state {
			u8 mode;
			// more to come
		};
		```
	- 2rd and 3st one: text mode state
		```c
		struct gfx_text_attr {
			u8 fg;
			u8 bg;
		};
		struct gfx_text {
			char text[ROWS*COLS];
			struct gfx_text_attr attrs[ROWS*COLS];
		};
		```
	- more: yes

interrupts!!!!!!
- mode 2 i think
	- the one where the device gives it a nuber and it jumps table
- nubers:
	- 1=vsync / end of frame drawing
