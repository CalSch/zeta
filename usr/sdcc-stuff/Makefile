CC = sdcc
AS = sdasz80
CFLAGS = -mz80 --no-std-crt0 --nostdinc
LDFLAGS = -mz80 --no-std-crt0 --code-loc 0x0000 --data-loc 0x2000

SRCS := $(wildcard *.c)
OBJS = init.rel $(patsubst %.c,%.rel,$(filter %.c,$(SRCS)))

all: out.bin

# compile
%.rel: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# assemble
%.rel: %.s
	$(AS) -l -o $@ $<

# link
# we build it twice. once to get the sizes of each area, and another to put _INITIALIZER and _int_table in the right place
# start of _INITIALIZER = end of _CODE (l__CODE)
# start of _int_table = end of _INITIALIZER, aligned to the start of a page (((l__CODE + l__INITIALIZER)/256 + 1)*256)
# _int_table is figured out with `dc` the RPN calculator
#	`16 o 16 i` tells it to output and input in hex instead of dec
#	`p` at the end prints the result
# 
# WARNING: this could be a big place for bugs to spawn, and it gets hard to debug since `make` doesn't actually print the result of the `$()` expansion
# TODO: get `make` to print the `$()` expansion, or extract this into a script where we have more control
#
out.ihx: $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@
	$(CC) $(LDFLAGS) -Wl"-b _INITIALIZER=0x$$(grep 'l__CODE' out.map | head -1 | awk '{print $$1}')" -Wl"-b _int_table=0x$$(echo 16 o 16 i $$(grep -E 'l__INITIALIZER|l__CODE' out.map | awk '{print $$1}') + 100 / 1 + 100 \* p | dc)" $(OBJS) -o $@

# ihx -> bin
out.bin: out.ihx
	makebin -p $< $@

clean:
	rm -f *.rel *.ihx *.bin *.map *.noi *.lk *.lst *.sym *.asm
