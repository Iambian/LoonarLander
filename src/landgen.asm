XDEF _landgen

XREF _surfaceheight
XREF _landingpadx
XREF _landingpadw

.ASSUME ADL=1

_landgen:
	PUSH IX
	;START
	LD HL,(0E30014h)
	LD DE,38240 ;HALF OF WHAT WILL TAKE POINTER TO LAST LINE IN BUFFER
	ADD HL,DE
	ADD HL,DE
	LD IX,_surfaceheight
	LD DE,-320
	LD C,160
	LD A,0EFh
landgen_mainloop:
	LD A,240
	SUB A,(IX+0)
	LD B,A
	LD A,0EFh
	PUSH HL
landgen_subloop1:
		LD (HL),A
		ADD HL,DE
		DJNZ landgen_subloop1
	POP HL
	INC HL
	LD A,240
	SUB A,(IX+1)
	LD B,A
	LD A,0EFh
	PUSH HL
landgen_subloop2:
		LD (HL),A
		ADD HL,DE
		DJNZ landgen_subloop2
	POP HL
	INC HL
	LEA IX,IX+2
	DEC C
	JR NZ,landgen_mainloop
	;END
	LD DE,(_landingpadx)
	LD HL,_surfaceheight
	ADD HL,DE
	LD L,(HL) ;get Y pos
	INC L
	LD H,160
	MLT HL
	ADD HL,HL
	ADD HL,DE ;get offset
	LD DE,(0E30014h)
	ADD HL,DE ;get address
	LD DE,320
	LD C,3
landgen_padloop:
	LD A,(_landingpadw)
	LD B,A
	LD A,0E4h
	PUSH HL
landgen_padsubloop:
		LD (HL),A
		INC HL
		DJNZ landgen_padsubloop
	POP HL
	ADD HL,DE
	DEC C
	JR NZ,landgen_padloop
	;END
	POP IX
	RET

