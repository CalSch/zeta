.globl start
.globl _setup
.globl _count_a
.globl _count_b
.globl _end_frame
.globl _thingy
.globl s__INITIALIZER
.globl s__INITIALIZED
.globl l__INITIALIZER
.globl s__CODE
.globl l__CODE
.globl data_start
.globl nmi_start

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

.area _DATA
data_start:

in_video_int: 
    .db 0

; stores the location of the current process's state
current_proc:
    .dw 0
; stores the index into proc_list of the current process
current_pid:
    .db 0

; list of pointers to process states
proc_list:
    .dw 0, 0

; HL AF PC BC DE
; remember that pairs are stored in the opposite order (eg. LH vs HL, to align with ld (nn), rr)
proc_a_state: 
    .dw 0,0,0,0,0
proc_b_state: 
    .dw 0,0,0,0,0
       
.area _NMI (ABS)
.org 0x0000
    jp start
        
.org 0x0066
nmi_start: 
    call swap_tasks
    ei 
    retn 
       
.area _CODE
       
;nmi_end: 
    ;out (4), A
    ;retn 
swap_tasks: 
    ; skip if in video interrupt
    ; TODO; pretty bad code here
    PUSH_ALL
        ld A, (in_video_int)
        cp A, #0
        
        jp NZ, pop_and_ret
    POP_ALL

    pop IY ; now that we wont return, we don't need the return addr (its always nmi_start+3) (also we use IY bc we cant touch HL yet)
    ld IY, (current_proc) ; load the address of the current process's state

    ;out (4), A
  
; save HL
    ld 0 (IY), L
    ld 1 (IY), H
; save AF
    ; set HL to AF (bc you cant do ld (IY+d), F)
    push AF
    pop HL
    ld 2 (IY), L
    ld 3 (IY), H
; save PC (not current PC, the PC before the NMI)
    pop HL ; bc the stack currently has the return address of the NMI
    ld 4 (IY), L
    ld 5 (IY), H
; save BC
    ld 6 (IY), C
    ld 7 (IY), B
; save DE
    ld 8 (IY), E
    ld 9 (IY), D

    call swap_states
    ld IY, (current_proc)

; load AF
    ld L, 2 (IY)
    ld H, 3 (IY)
    ;ld AF, HL
    push HL
    pop AF
; load BC
    ld L, 6 (IY)
    ld H, 7 (IY)
; load DE
    ;ld DE, (#proc_a_state+8)
    ld L, 8 (IY)
    ld H, 9 (IY)
; load PC (stored in IY)
    ; nobody uses IY so were free to mangle it (we're gonna jump to it later)
    ld L, 4 (IY)
    ld H, 5 (IY)
    push HL ; we still need IY, so we'll pop it after loading HL
; load HL
    ld L, 0 (IY)
    ld H, 1 (IY)

    pop IY ; actually load IY (address to jump to)

    ;out (4), A
    push AF
        ld A, #1
        out (6), A
        out (6), A
    pop AF
    im 2 ; idk if i have to do ts
    ei
    jp (IY)

pop_and_ret:
    POP_ALL
    ret

; current_proc = proc_list[current_pid*2]
update_current_proc:
    ld A, (current_pid)
    sla A ; multiply by two (sets carry flag!)
    ld HL, #proc_list
    ; set BC to A
    ld B, #0
    ld C, A
    ; now HL = proc_list + (current_pid*2)
    add HL, BC
    
    ; need to ld BC, (HL)
    ld C, (HL)
    inc HL
    ld B, (HL)
    
    ; now BC = current_proc
    ld (current_proc), BC
    ret

swap_states:
    ld A, (current_pid)
    inc A
    and A, #1
    ld (current_pid), A
    call update_current_proc
    ret

    ld HL, (proc_a_state+0)
    ld DE, (proc_b_state+0)
    ld (proc_b_state+0), HL
    ld (proc_a_state+0), DE
    ld HL, (proc_a_state+2)
    ld DE, (proc_b_state+2)
    ld (proc_b_state+2), HL
    ld (proc_a_state+2), DE
    ld HL, (proc_a_state+4)
    ld DE, (proc_b_state+4)
    ld (proc_b_state+4), HL
    ld (proc_a_state+4), DE
    ld HL, (proc_a_state+6)
    ld DE, (proc_b_state+6)
    ld (proc_b_state+6), HL
    ld (proc_a_state+6), DE
    ld HL, (proc_a_state+8)
    ld DE, (proc_b_state+8)
    ld (proc_b_state+8), HL
    ld (proc_a_state+8), DE
    ret


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
    ;out (4), A ; to debug int_table errors
    im 2
    ei


    ld hl, #s__INITIALIZER
    ld de, #s__INITIALIZED
    ld bc, #l__INITIALIZER
    ldir

    call _setup

    ; kinda concerning that i have to do this myself...
    ld A, #0
    ld (_count_b), A
    ld (_count_b+1), A
    ld (_count_a), A
    ld (_count_a+1), A

    ; setup proc_b_state
    ; make sure that process b actually runs the right code
    ld HL, #proc_b
    ld (proc_b_state+4), HL

    ; setup proc_list
    ld HL, #proc_a_state
    ld (proc_list), HL
    ld HL, #proc_b_state
    ld (proc_list+2), HL

    ; setup current_pid
    ld A, #0
    ld (current_pid), A

    call update_current_proc

    halt
    jp proc_a
    ;jp .


proc_a:
    ld HL, (_count_a)
    inc HL
    ld (_count_a), HL
    ;ld B, #255
    ;djnz .
    jp proc_a
    
proc_b:
    jp _thingy
    ld HL, (_count_b)
    inc HL
    ld (_count_b), HL
    ;ld B, #255
    ;djnz .
    jp proc_b



unknown_int:
    ei
    reti

end_frame_int:
    PUSH_ALL
    
    ld A, (in_video_int)
    cp A, #0
    jp NZ, end_interrupt_pop
    
    ;inc A
    ld A, #1
    ld (in_video_int), A

    ei
    call _end_frame

    ld A, #0
    ld (in_video_int), A

    POP_ALL

    call swap_tasks
    jp end_interrupt



end_interrupt_pop:
    ;out (4), A
    POP_ALL
end_interrupt:
    ei
    reti

.area _int_table (ABS)

int_table:
    .dw unknown_int
    .dw end_frame_int

