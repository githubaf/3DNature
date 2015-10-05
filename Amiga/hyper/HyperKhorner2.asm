; Err, I'm not exactly sure what I'm doing, y'know.
; But it better be fast.

	XDEF _HyperKhorner

	Section CODE

; Definitions

TRANSCOL	equ	$FF

; Register defs

INPIX		equr	D0

ONE		equr	D1
PIXCNT	equr	D1

BITAB		equr	D2
BITCD		equr	D3
BITEF		equr	D4
BITGH		equr	D5
BITM		equr	D6
PLANE7	equr	D7

PLANE0	equr	A0
PLANE1	equr	A1
PLANE2	equr	A2
PLANE3	equr	A3
PLANE4	equr	A4
PLANE5	equr	A5
PLANE6	equr	A6

; This code makes heavy use of Logical Shift Right (LSL) shoving the least
;  significant bit off of a chunky pixel into the eXtend bit, followed
;  by a ROtate with eXtend Right (ROXR) pulling the X bit onto the left
;  edge of one of eight (or four, in the stripped version) pixel
;  accumulation registers. The immediate constant #1 is loaded into D1
;  (here called ONE) because I thought it would make the code smaller,
;  and possibly faster. (??)
;
; In the upper word of D6, I store a pixel count, for loop purposes. It
;  might help (on 680[3+]0 CPUs) to unroll the loops, but an unrolled
;  version would not fit into the 68020's 128-byte cache, and the
;  loop version will, I think.
;
; The pixel accumulators (BITxx) and the input pixel register (INPIX)
;  must all be D(ata) registers, since A(ddress) registers do not
;  support LSx or ROxx (Logical Shift or ROtate) instructions. Since I
;  only _have_ eight registers (D0-D7), and for my purposes I also need
;  a Mask generated on the fly, this poses a quandry. Therefore in the
;  8-plane version, I store the 16-bit bitplane output accumulators in
;  pairs in the low and high words of 4 Data registers, and use the
;  SWAP instruction to toggle which one I'm using. (P.S. I can't see
;  a more efficient way than this. Yet.) The 4-plane version does not have
;  this problem, and is faster for it.
;
; For 68000-use, I'd use LSL.W in INPIX, and then SWAP after 16 shifts.
;  According to my Motorola book, LSL.W is 2 cycles shorter than LSL.L, and
;  even adding the overhead for a SWAP (4), we;d still come out 8 cycles ahead.
;  On 680[2+]0 LSL is the same speed regardless of the width.
;
; In the 4-plane version, we have to throw in an extra LSR.W between bytes,
;  to shift off the upper bits we're not using.
;
; BitPlane pointers are in A0-A6 and D7 (A7 being unavailable, natch ;) in
;  the 8-plane version, and simply A0-A3 in the 4-plane version.
;
; I think I save a bit by loading 4 pixels as a longword for input, chewing
;  on everything using registers only, and spitting out 16 pixels as 8 (or 4)
;  16-bit words for output. The 16-bit output wins with regard to Amiga chip
;  memory, I only wish I could interleave the writes with something else
;  useful, as I think 68020+ machines will spend some time spinning their
;  wheels waiting for the bus to finish. Dunno if anything can be done about
;  that. BitPlane pointers are postincremented. (Duh.)
;
; Y'know, I wonder if I could make an FPU-only version that used the FPU
;  registers as scratch space for the pixel accumulators, thus saving me the
;  pair of SWAP instrux per each accumulator. Then again, the 4-bit version
;  could care less. Hmm. (Yes, I ramble.)


_HyperKhorner move.l	#1,ONE	; In our universe, ONE = 1. How 'bout yours?

; INPIX needs to be prepared before this point for each 4-pixel entry

; Do one Pixel
	; A

DoPix	lsr.l		ONE,INPIX		; Shift LSb into X flag (0LSb -> X)
	roxr.w	ONE,BITAB		; out of X into A (X -> A)
	swap		BITAB				; Swap A & B, B now in LSW (A <-> B = B)

	; B
	lsr.l		ONE,INPIX		; 1LSb -> X
	roxr.w	ONE,BITAB		; X -> B
	swap		BITAB				; B <-> A = A (for next loop)
	
	; C
	lsr.l		ONE,INPIX		; 2LSb -> X
	roxr.w	ONE,BITCD		; X -> C
	swap		BITCD				; C <-> D = D

	; D
	lsr.l		ONE,INPIX		; 3LSb -> X
	roxr.w	ONE,BITCD		; X -> D
	swap		BITCD				; D <-> C = C (for next time around)

	; E
	lsr.l		ONE,INPIX		; 4LSb -> X
	roxr.w	ONE,BITEF		; X -> E
	swap		BITEF				; E <-> F = F
	
	; F
	lsr.l		ONE,INPIX		; 5LSb -> X
	roxr.w	ONE,BITEF		; X -> F
	swap		BITEF				; F <-> E = E (for next...)
	
	; G
	lsr.l		ONE,INPIX		; 6LSb -> X
	roxr.w	ONE,BITGH		; X -> G
	swap		BITGH				; G <-> H = H
	
	; H
	lsr.l		ONE,INPIX		; 7LSb (MSb) -> X
	roxr.w	ONE,BITGH		; X -> H
	swap		BITGH				; H <-> G = G

	swap		BITM				; M <-> PPIXCNT = PIXCNT
	move.b	PIXCNT,BITM		; move counter into scratch space
	andi.b	#3,BITM			; mask out high bits
	cmpi.b	#3,BITM			; are we done with 32 input bits (4 pixels)?
	bne		DoPix				; Nope
	
	cmpi.b	#15, PIXCNT		; Finished 16 loops yet?
	blt		DoPix				; Nope, keep going
