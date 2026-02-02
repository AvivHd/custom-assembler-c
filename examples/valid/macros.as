;this file spread macros

.define sz = 2
MAIN:	mov r3, LIST[sz]
;
LOOP: 	jmp L1
	mcr mcro
		cmp r3, #sz
		bne END
	endmcr
	prn #-5
	
	
	
	mov STR[5], STR[2]
	sub r1, r4
; comment
	mcro
L1: 	inc K
	bne LOOP
END: hlt
.define len = 4
STR: .string "ab"cdef"
LIST: .data 6, -9, len
K: .data 22
