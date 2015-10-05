; Err, I'm not exactly sure what I'm doing, y'know.
; But it better be fast.

	XDEF _HyperKhorner

	Section CODE


; Register defs

INPIX4	equr D0
TABA4		equr D1
TABB4		equr D2
BITAB		equr D3
BITCD		equr D4
BITEF		equr D5
BITGH		equr D6
BITM		equr D7

TABBASE	equr A0
BITBASE	equr A1
OUTBIT	equr A2

INPIX		equr A3
MASKGEN	equr A3

BITSPLIT	equr A4
PIXCNT	equr A5
ODDMASK	equr A6

_HyperKhorner move.q	#0,d2 ;nothing in particular right now

; PIXCNT must be set to 0 at every 16-pixel entry
; ODDMASK needs to be initialized to $00FF00FF at every 16-pixel entry

; INPIX4 needs to be prepared before this point for each 4-pixel entry


DoFour	move.b	INPIX4,INPIX			; Grab first pixel into INPIX
	lsr.l		#8,INPIX4				; Shift remaining pixels over
	add.l		INPIX,INPIX				; Multiply by eight to get table offset
	add.l		INPIX,INPIX				;  from base ((INPIX+INPIX)+(INPIX+INPIX))+
	add.l		INPIX,INPIX				;  ((INPIX+INPIX)+(INPIX+INPIX))
	move.l	INPIX(TABBASE),TABA	; Fetch table entry
	addq.l	#4,INPIX					; Get address for second half of entry
	move.l	INPIX(TABBASE),TABB	; Fetch second half of entry

; This generates the mask value. Any input pixel
; value above 15 will set the BITM (mask) bit, aka transparent.
; This relies on the Z flag state being correct from the second-half
; fetch (line above) so be careful adding code inbetween there and here.
; Note: We trash INPIX (same as MASKGEN) at this point. We're done with it.

	sne		MASKGEN					; Set all 8 bits in maskgen if non-zero move
	andi.b	#1,MASKGEN				; Mask off all but LSb
	or.b		MASKGEN,BITM			; Move mask bit into mask accumulator
; Can't rightly figure if this next one should be a left or right.
	ror.l		#1,BITM					; ...and a step to the riiiight...


; Chew on the A/B bitplanes
	move.l	TABA4,MASKGEN			; Get the bits into scratch space
	and.l		ODDMASK,MASKGEN		; Knock off the ones we don't want (yet)
	or.l		ODDMASK,BITAB			; And move into A/B accumulator
	ror.l		#1,BITAB					; ...and a step to the riiiight...

; C/D bitplanes
	not.l		ODDMASK					; Turn $00FF00FF into $FF00FF00 for other half
	move.l	TABA4,MASKGEN			; Get the bits into scratch space
	and.l		ODDMASK,MASKGEN		; Knock off the ones we don't want
	or.l		ODDMASK,BITCD			; And move into C/D accumulator
	ror.l		#1,BITCD					; ...and a step to the riiiight...

; Note: for my 4-bitplane implementation, I disable the following code

; E/F bits
;	not.l		ODDMASK					; Turn $FF00FF00 into $00FF00FF for other half
;	move.l	TABB4,MASKGEN			; Get the bits into scratch space
;	and.l		ODDMASK,MASKGEN		; Knock off the ones we don't want (yet)
;	or.l		ODDMASK,BITEF			; And move into E/F accumulator
;	ror.l		#1,BITEF					; ...and a step to the riiiight...

; G/H
;	not.l		ODDMASK					; Turn $00FF00FF into $FF00FF00 for other half
;	move.l	TABB4,MASKGEN			; Get the bits into scratch space
;	and.l		ODDMASK,MASKGEN		; Knock off the ones we don't want
;	or.l		ODDMASK,BITGH			; And move into G/H accumulator
;	ror.l		#1,BITGH					; ...and a step to the riiiight...

	cmpi.b	$F,PIXCNT				; have we finished a 16-pixel block?
	beq		WriteOut					; Yep, finish up
	
	move.b	PIXCNT,INPIX			; Check to see if finished with 4-pixel group
	andi.b	#3,INPIX					; Toss high bits
	cmpi.b	#3,INPIX					; Are we theeeeerrrrrreeee yet?
	bne		DoFour					; Not yet, children. Just a few more cycles.



	END
