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
	$(AS) -o $@ $<

# link
out.ihx: $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@
	@# sleep 1
	@# $(eval INIT_LOC := $(shell grep "l__CODE" out.map | head -1 | awk '{print $$1}'))
	@# @echo -e "\n\n$(INIT_LOC)\n\n"
	@# @read a
	@#$(CC) $(LDFLAGS) -Wl"-b _INITIALIZER=0x$$(grep "l__CODE" out.map | head -1 | awk '{print $$1}')" $(OBJS) -o $@
	$(CC) $(LDFLAGS) -Wl"-b _INITIALIZER=0x$$(grep 'l__CODE' out.map | head -1 | awk '{print $$1}')" $(OBJS) -o $@

# ihx -> bin
out.bin: out.ihx
	makebin -p $< $@

clean:
	rm -f *.rel *.ihx *.bin *.map *.noi *.lk *.lst *.sym *.asm
