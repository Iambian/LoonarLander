;NOTE: This is not USB-friendly as this overrwrites timer 3.

XDEF _starttimer
XDEF _readtimer

.ASSUME ADL=1
;--------------------
SEGMENT DATA
VALUES:
DL	1024   ;COUNTER REGISTER
DL	1     ;RESET VALUE (0 TO MAKE IT STOP)
VALUESEND:

;--------------------
SEGMENT CODE
_starttimer:
	DI
	LD IY,0F20000h
	XOR A
	LD (IY+30h),A      ;DISABLE TIMERS
	LEA DE,IY+0
	LD HL,VALUES
	LD BC,8
	LDIR               ;LOAD REGISTER VALUES
	LD HL,0000_000_000_000_011b
	LD (IY+30h),HL     ;TIMER CTRL REG ENABLE, SET XTAL TIMER 1, COUNT DOWN, NO INT.
	RET
	
_readtimer:
	LD HL,(0F20000h)  ;POLL MSB TO DETECT UNDERFLOW
	RET
	
	
	
	
	
	
	
	
	
	


