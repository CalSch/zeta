.globl _setup
.globl _end_frame
.globl s__INITIALIZER
.globl s__INITIALIZED
.globl l__INITIALIZER
.globl s__CODE
.globl l__CODE

.area _DATA

in_video_int:
    .db 0

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

.macro PUSH_ALL
    push AF
    push BC
    push DE
    push HL
.endm
.macro POP_ALL
    pop HL
    pop DE
    pop BC
    pop AF
.endm

start:
    MAP_SECTOR #6, #2, #1
    ld SP, #0xdfff ; stack at top of last sector
    
    ; setup ints
    ld HL, #int_table
    ld A, H
    ld I, A
    im 2
    ei


    ld hl, #s__INITIALIZER
    ld de, #s__INITIALIZED
    ld bc, #l__INITIALIZER
    ldir

    call _setup

    jp .

unknown_int:
    ei
    reti
end_frame_int:
    PUSH_ALL
    
    ld A, (in_video_int)
    cp A, #0
    jp NZ, end_interrupt_pop
    
    inc A
    ld (in_video_int), A

    ei
    call _end_frame

    xor A, A
    ld (in_video_int), A

    POP_ALL
    jp end_interrupt

end_interrupt_pop:
    POP_ALL
    out (4), A
end_interrupt:
    ei
    reti

.area _int_table (ABS)

int_table:
    .dw unknown_int
    .dw end_frame_int

