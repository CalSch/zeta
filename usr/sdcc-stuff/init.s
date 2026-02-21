.globl _setup
.globl s__INITIALIZER
.globl s__INITIALIZED
.globl l__INITIALIZER


.area _CODE

.macro MAP_SECTOR sw, hw_hi, hw_lo
    ld a, sw
    out (2), a
    ld a, hw_lo
    out (2), a
    ld a, hw_hi
    out (2), a
    ld a, #0xFF
    out (2), a
    out (2), a
    out (2), a
.endm

start:
    MAP_SECTOR #6, #2, #1
    ld sp, #0xdfff ; stack at top of last sector

    ld hl, #s__INITIALIZER
    ld de, #s__INITIALIZED
    ld bc, #l__INITIALIZER
    ldir

    call _setup

    jp .

