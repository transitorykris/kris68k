;------------------------------------------------------------
;                                  ___ ___ _   
;  ___ ___ ___ ___ ___       _____|  _| . | |_ 
; |  _| . |_ -|  _| . |     |     | . | . | '_|
; |_| |___|___|___|___|_____|_|_|_|___|___|_,_| 
;                     |_____|       firmware v1                 
;------------------------------------------------------------
; Copyright (c)2019 Ross Bamford
; See top-level LICENSE.md for licence information.
;
; TRAP #14 handler and extended function wrappers
;
; TRAP 14 provides access to some of the basic functionality
; of the firmware after the bootloader has terminated.
;
; D1 is expected to contain the function code. Other arguments
; depend on the specific function - See README for details.
;------------------------------------------------------------

; TRAP 16 is a temporary hack while I sort out how to do
; proper pre-emptive multitasking and interrupt handling (kris)

    include "../shared/equates.S"

    import context_swap

; EFP table addresses (from the firmware's equates.S)
;EFP_PRINT       equ    $420
;EFP_PRINTLN     equ    $424
;EFP_PRINTCHAR   equ    $428
;EFP_HALT        equ    $42C
;EFP_SENDCHAR    equ    $430
;EFP_RECVCHAR    equ    $434
;EFP_CLRSCR      equ    $438
;EFP_MOVEXY      equ    $43C
;EFP_SETCURSOR   equ    $440

    section .text

; This is the exception handler...
TRAP_14_HANDLER::
    ori.w   #$700,sr                  ; Disable interrupts
    cmp.l   #0,D1                     ; FC == 0?
    beq.s   .PRINT                    ;   PRINT if so...
    cmp.l   #1,D1                     ; FC == 1?
    beq.s   .PRINTLN                  ;   PRINTLN if so...
    cmp.l   #2,D1                     ; FC == 2?
    beq.s   .SENDCHAR                 ;   SENDCHAR if so...
    cmp.l   #3,D1                     ; FC == 3?
    beq.s   .RECVCHAR                 ;   RECVCHAR if so...
    cmp.l   #4,D1                     ; FC == 4?
    beq.s   .PRINTCHAR                ;   PRINTCHAR if so...
    cmp.l   #5,D1                     ; FC == 5?
    beq.s   .SETCURSOR                ;   SETCURSOR if so...
    bra     context_swap              ; RTE happens here

.PRINT
    move.l  A1,-(A7)
    move.l  EFP_PRINT,A1
    jsr     (A1)
    move.l  (A7)+,A1
    rte

.PRINTLN
    move.l  A1,-(A7)
    move.l  EFP_PRINTLN,A1
    jsr     (A1)
    move.l  (A7)+,A1
    rte

.SENDCHAR
    move.l  A1,-(A7)
    move.l  EFP_SENDCHAR,A1
    jsr     (A1)
    move.l  (A7)+,A1
    rte

.RECVCHAR
    move.l  A1,-(A7)
    ; Note: hard coding this, we really want to be able to switch
    ; between MFP and DUART for automatically
    jsr RECVCHAR_DUART_NONBLOCKING
    move.l  (A7)+,A1
    rte
    
.PRINTCHAR
    move.l  A1,-(A7)
    move.l  EFP_PRINTCHAR,A1
    jsr     (A1)
    move.l  (A7)+,A1
    rte

.SETCURSOR
    move.l  A1,-(A7)
    move.l  EFP_SETCURSOR,A1
    jsr     (A1)
    move.l  (A7)+,A1
    rte

; Falls through returning NULL if no characters are waiting
RECVCHAR_DUART_NONBLOCKING:
    move.l  A0,-(A7)            ; Stash A0
    move.l  SDB_UARTBASE,A0     ; Get DUART base address
.BUSYLOOP
    btst.b  #0,DUART_SRA(A0)
    beq.s   .NOCHR              ; No characters waiting for us
.GOTCHR:
    move.b  DUART_RBA(A0),D0
    move.l  (A7)+,A0            ; Restore A0
    rts
.NOCHR
    move.b  #0,D0               ; Send a NULL otherwise
    move.l  (A7)+,A0            ; Restore A0
    rts

; Falls through returning NULL if no characters are waiting
RECVCHAR_MFP_NONBLOCKING::
    bclr.b  #7,MFP_GPDR         ; Lower RTS
.BEGIN
    move.b  MFP_RSR,D0          ; Get RSR
    btst    #7,D0               ; Is buffer_full bit set?
    bne.s   .GOTCHR             ; Yes - Go to receive character

    btst    #6,D0               ; Else, do we have an overrun error?
    bne.s   .GOTERR             ; .. Yes - handle that
    bra.s   .NOCHR              ; No - we're out

.GOTERR
    move.b  MFP_UDR,D0          ; Empty buffer
    bchg.b  #1,MFP_GPDR         ; And toggle I1
    bra.s   .BEGIN              ; And continue testing...
    
.GOTCHR
    move.b  MFP_UDR,D0          ; Get the data
    rts

.NOCHR
    move.b #0,D0                ; Send a NULL otherwise
    rts

; Wraps FW_PRINT so it can be called from C-land
;
; Modifies: A0 (Will point to address after null terminator)
FW_PRINT_C::
    move.l  (4,A7),A0                 ; Get C char* from the stack into A0
    bra.s   FW_PRINT                  ; Call EARLY_PRINT

; Wraps FW_PRINTLN so it can be called from C-land
;
; Modifies: A0 (Will point to address after null terminator)
;
; It's cheaper size-wise to just duplicate this here...
FW_PRINTLN_C::
    move.l  (4,A7),A0                 ; Get C char* from the stack into A0
    bra.s   FW_PRINTLN                ; Call EARLY_PRINT

; Firmware PRINT null-terminated string pointed to by A0
; Uses PRINT function pointed to by EFP table
;
; Modifies: A0 (Will point to address after null terminator)
FW_PRINT::
    move.l  A1,-(A7)
    move.l  EFP_PRINT,A1
    jsr     (A1)
    move.l  (A7)+,A1
    rts
    
; PRINT null-terminated string pointed to by A0 followed by CRLF.
; Uses PRINTLN function pointed to by EFP table
;
; Modifies: A0 (Will point to address after null terminator)
FW_PRINTLN::
    move.l  A1,-(A7)
    move.l  EFP_PRINTLN,A1
    jsr     (A1)
    move.l  (A7)+,A1
    rts

; PRINT a single character contained in D0.
; Uses PRINTCHAR function pointed to by EFP table.
;
; Modifies: Nothing
FW_PRINTCHAR::
    move.l  A1,-(A7)
    move.l  EFP_PRINTCHAR,A1
    jsr     (A1)
    move.l  (A7)+,A1
    rts

; Move cursor to X,Y coordinates contained in D1.W.
; High byte is X, low byte is Y.
; Uses PRINTCHAR function pointed to by EFP table.
;
; Modifies: Nothing
FW_MOVEXY::
    move.l  A1,-(A7)
    move.l  EFP_MOVEXY,A1
    jsr     (A1)
    move.l  (A7)+,A1
    rts

; Clear the screen.
; Uses CLRSCR function pointed to by EFP table.
;
; Modifies: Nothing
FW_CLRSCR::
    move.l  A1,-(A7)
    move.l  EFP_CLRSCR,A1
    jsr     (A1)
    move.l  (A7)+,A1
    rts
