| This is a copy of HerKhorner4M-1.asm from SAS/C-Amiga Project, but syntax adapted for gcc
| Comments start with |, Labels need a colon

| Err, I'm not exactly sure what I'm doing, y'know.
| But it better be fast.

    XDEF _HK4M

|    Section CODE

| Definitions

TRANSCOL    =   0xFF

| Register defs

INPIX       =    D0
ONE         =    D1
BITA        =    D2
BITB        =    D3
BITC        =    D4
BITD        =    D5
LOOPCNT     =    D6
BITM        =    D7


PLANE0      =    A0  | Set before entry
PLANE1      =    A1  | Set before entry
PLANE2      =    A2  | Set before entry
PLANE3      =    A3  | Set before entry
PLANEM      =    A4  | Set before entry
PIXCNT      =    A5  | Set before entry
INPTR       =    A6  | Set before entry



| This code makes heavy use of Logical Shift Right (LSL) shoving the least
|  significant bit off of a chunky pixel into the eXtend bit, followed
|  by a ROtate with eXtend Right (ROXR) pulling the X bit onto the left
|  edge of one of eight (or four, in the stripped version) pixel
|  accumulation registers.
|
| The immediate constant #1 is loaded into D1, because it's convenient,
|  fast, and makes the code slightly shorter.
|
| This code is unrolled, and at 160 bytes should fit into a 68030 inst
|  cache. You could roll it back up, and probably get it < 80 bytes to
|  fir it into a 68020 inst cache. Such was not my concern.

_HK4M:
        move.l  D2,-(A7)             | Stack it, eh?
        move.l  D3,-(A7)
        move.l  D4,-(A7)
        move.l  D5,-(A7)
        move.l  D6,-(A7)
        move.l  D7,-(A7)

        move.l  A2,-(A7)
        move.l  A3,-(A7)
        move.l  A4,-(A7)
        move.l  A5,-(A7)
        move.l  A6,-(A7)

        movem.l (a0),a0-a6          | AF: We have all parameters in an array on the stack now. We passed the pointer to the arry in A0.
                                    | now they are in the Registers again

        moveq       #1,ONE          | ONE = 1
        moveq       #0,LOOPCNT      | Clear loop
        move.l  PIXCNT,INPIX        | So we can shift
        lsr.l       #2,INPIX        | Operations are done 4 pixels at a time
        sub.l       ONE,INPIX       | Beware of the wascally fencepost
        move.l  INPIX,PIXCNT

Fetch:  move.l  (INPTR)+,INPIX      | Fetch Pixel
        moveq       #0,BITM         | So we can roll 0's later

| Pixel 0
        cmpi.b  #TRANSCOL,INPIX     | Generate transparent Mask?
        beq.b       NMsk0
        or.b        ONE,BITM        | Set bit to 1
NMsk0:  ror.l       ONE,BITM

        lsl.l       #4,INPIX        | Shift off cruft
        add.l       INPIX,INPIX
        addx.l  BITD,BITD
        add.l       INPIX,INPIX
        addx.l  BITC,BITC
        add.l       INPIX,INPIX
        addx.l  BITB,BITB
        add.l       INPIX,INPIX
        addx.l  BITA,BITA

| Pixel 1
        cmpi.b  #TRANSCOL,INPIX     | Generate transparent Mask?
        beq.b       NMsk1
        or.b        ONE,BITM
NMsk1:  ror.l       ONE,BITM

        lsl.l       #4,INPIX        | Shift off cruft
        add.l       INPIX,INPIX
        addx.l  BITD,BITD
        add.l       INPIX,INPIX
        addx.l  BITC,BITC
        add.l       INPIX,INPIX
        addx.l  BITB,BITB
        add.l       INPIX,INPIX
        addx.l  BITA,BITA

| Pixel 2
        cmpi.b  #TRANSCOL,INPIX     | Generate transparent Mask?
        beq.b       NMsk2
        or.b        ONE,BITM
NMsk2:  ror.l       ONE,BITM

        lsl.l       #4,INPIX        | Shift off cruft
        add.l       INPIX,INPIX
        addx.l  BITD,BITD
        add.l       INPIX,INPIX
        addx.l  BITC,BITC
        add.l       INPIX,INPIX
        addx.l  BITB,BITB
        add.l       INPIX,INPIX
        addx.l  BITA,BITA

| Pixel 3
        cmpi.b  #TRANSCOL,INPIX     | Generate transparent Mask?
        beq.b       NMsk3
        or.b        ONE,BITM
NMsk3:  ror.l       ONE,BITM

        lsl.l       #4,INPIX        | Shift off cruft
        add.l       INPIX,INPIX
        addx.l  BITD,BITD
        add.l       INPIX,INPIX
        addx.l  BITC,BITC
        add.l       INPIX,INPIX
        addx.l  BITB,BITB
        add.l       INPIX,INPIX
        addx.l  BITA,BITA

| Check to see if done with 32 pixels
        move.l  LOOPCNT,INPIX       | INPIX is working space
        andi.b  #7,INPIX            | keep bottom 4 bits
        cmpi.b  #7,INPIX            | Done yet?
        beq     Skip                | Yep
JFet:   add.l       ONE,LOOPCNT
        bra     Fetch

| Must be ready to write bitplanes
Skip:   move.l  BITA,(PLANE0)+
        move.l  BITB,(PLANE1)+
        move.l  BITC,(PLANE2)+
        move.l  BITD,(PLANE3)+
        move.l  BITM,(PLANEM)+

| Are we _really_ done?
        cmp.l       PIXCNT,LOOPCNT
        blt.b       JFet

      move.l    (A7)+,A6
      move.l    (A7)+,A5
      move.l    (A7)+,A4
      move.l    (A7)+,A3
      move.l    (A7)+,A2

        move.l  (A7)+,D7
      move.l    (A7)+,D6
      move.l    (A7)+,D5
      move.l    (A7)+,D4
      move.l    (A7)+,D3
      move.l    (A7)+,D2

        rts


|    END
