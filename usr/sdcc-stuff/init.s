.globl start
.globl _setup
.globl _end_frame
.globl _counter
.globl s__INITIALIZER
.globl s__INITIALIZED
.globl l__INITIALIZER
.globl s__CODE
.globl l__CODE

.area _DATA

in_video_int:
    .db 0


.area _NMI (ABS)
.org 0x0000
    jp start

.org 0x0066
nmi_start:
    out (4), A
    retn
nmi_end:

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
    ; starting map
    ; sw0 -> 0   rom
    ; sw1 -> 512 ram0
    ; sw7 -> 8   vram0
    MAP_SECTOR #5, #0, #1 ; sw5 -> hw1 (disk)

    MAP_SECTOR #3, #2, #2 ; sw3 -> hw514 (ram2)
    MAP_SECTOR #6, #2, #1 ; sw6 -> hw513 (ram1)
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
    push AF
    push HL
    ld HL, #_counter
    inc (HL)
    pop HL
    pop AF
    ei
    reti

    PUSH_ALL
    
    ld A, (in_video_int)
    cp A, #0
    jp NZ, end_interrupt_pop
    
    ;inc A
    ld A, #1
    ld (in_video_int), A

    ei
    call _end_frame

    ;xor A, A
    ld A, #0
    ld (in_video_int), A

    POP_ALL
    jp end_interrupt


special_key_int:
    out (4), A
    ei
    reti



end_interrupt_pop:
    POP_ALL
end_interrupt:
    ei
    reti

.area _int_table (ABS)

int_table:
    .dw unknown_int
    .dw end_frame_int
    .dw special_key_int

