; Err, I'm not exactly sure what I'm doing, y'know.
; But it better be fast.

	XDEF _HK4M

	Section CODE

; Definitions

TRANSCOL	equ	#$FF

; Register defs

INPIX		equr	D0
ONE		equr	D1
BITA		equr	D2
BITB		equr	D3
BITC		equr	D4
BITD		equr	D5
LOOPCNT	equr	D6
BITM		equr	D7


PLANE0	equr	A0	; Set before entry
PLANE1	equr	A1	; Set before entry
PLANE2	equr	A2	; Set before entry
PLANE3	equr	A4	; Set before entry
PLANEM	equr	A4	; Set before entry
PIXCNT	equr	A5	; Set before entry
INPTR		equr	A6	; Set before entry



; This code makes heavy use of Logical Shift Right (LSL) shoving the least
;  significant bit off of a chunky pixel into the eXtend bit, followed
;  by a ROtate with eXtend Right (ROXR) pulling the X bit onto the left
;  edge of one of eight (or four, in the stripped version) pixel
;  accumulation registers.
;
; The immediate constant #1 is loaded into D1, because it's convenient,
;  fast, and makes the code slightly shorter.
;
; This code is unrolled, and at 160 bytes should fit into a 68030 inst
;  cache. You could roll it back up, and probably get it < 80 bytes to
;  fir it into a 68020 inst cache. Such was not my concern.

_HK4M	
		moveq		#1,ONE			; ONE = 1
		moveq		#0,LOOPCNT		; Clear loop
		move.l	PIXCNT,INPIX	; So we can shift
		lsr.l		#2,INPIX			; Operations are done 4 pixels at a time
		move.l	INPIX,PIXCNT

Fetch	move.l	(INPTR)+,INPIX	; Fetch Pixel
		moveq		#0,BITM 			; So we can roll 0's later

; Pixel 0
		cmpi.b	TRANSCOL,INPIX	; Generate transparent Mask?
		bne.b		NMsk0
		or.b		ONE,BITM			; Set bit to 1
NMsk0	ror.l		ONE,BITM

		add.l		INPIX,INPIX
		addx.l	BITA,BITA
		add.l		INPIX,INPIX
		addx.l	BITB,BITB
		add.l		INPIX,INPIX
		addx.l	BITC,BITC
		add.l		INPIX,INPIX
		addx.l	BITD,BITD
		lsl.l		#4,INPIX			; Shift off cruft

; Pixel 1
		cmpi.b	TRANSCOL,INPIX	; Generate transparent Mask?
		bne.b		NMsk1
		or.b		ONE,BITM
NMsk1	ror.l		ONE,BITM

		add.l		INPIX,INPIX
		addx.l	BITA,BITA
		add.l		INPIX,INPIX
		addx.l	BITB,BITB
		add.l		INPIX,INPIX
		addx.l	BITC,BITC
		add.l		INPIX,INPIX
		addx.l	BITD,BITD
		lsl.l		#4,INPIX			; Shift off cruft

; Pixel 2
		cmpi.b	TRANSCOL,INPIX	; Generate transparent Mask?
		bne.b		NMsk2
		or.b		ONE,BITM
NMsk2	ror.l		ONE,BITM

		add.l		INPIX,INPIX
		addx.l	BITA,BITA
		add.l		INPIX,INPIX
		addx.l	BITB,BITB
		add.l		INPIX,INPIX
		addx.l	BITC,BITC
		add.l		INPIX,INPIX
		addx.l	BITD,BITD
		lsl.l		#4,INPIX			; Shift off cruft

; Pixel 3
		cmpi.b	TRANSCOL,INPIX	; Generate transparent Mask?
		bne.b		NMsk3
		or.b		ONE,BITM
NMsk3	ror.l		ONE,BITM

		add.l		INPIX,INPIX
		addx.l	BITA,BITA
		add.l		INPIX,INPIX
		addx.l	BITB,BITB
		add.l		INPIX,INPIX
		addx.l	BITC,BITC
		add.l		INPIX,INPIX
		addx.l	BITD,BITD
		lsl.l		#4,INPIX			; Shift off cruft

; Check to see if done with 32 pixels
		move.l	LOOPCNT,INPIX	; INPIX is working space
		andi.b	#7,INPIX			; keep bottom 4 bits
		cmpi.b	#7,INPIX			; Done yet?
		beq		Skip				; Yep
		add.b		ONE,LOOPCNT
JFet	bra		Fetch
		
; Must be ready to write bitplanes
Skip	move.l	BITA,(PLANE0)+
		move.l	BITB,(PLANE1)+
		move.l	BITC,(PLANE2)+
		move.l	BITD,(PLANE3)+
		move.l	BITM,(PLANEM)+

; Are we _really_ done?
		cmp.l		PIXCNT,LOOPCNT
		blt.b		JFet

		rts


	END
