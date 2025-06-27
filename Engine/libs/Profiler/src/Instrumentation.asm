.data
; declare externs for our Profiler functions
EXTERNDEF ProfileEnter:PROC
EXTERNDEF ProfileExit:PROC

.code

; Setup _penter
_penter PROC EXPORT
	; Perform standard prolog first
	push RAX
	push RCX
	push RDX
	push r8
	push r9
	push r10
	push r11
	push RBX ; Pushing an extra register to align stack w/ multiple of 16

	; get the retun address off the stack & store to RCX
	mov RCX, [RSP + 40h]

	; create space on the stack for params [RCX, RDX, R8, R9]  (C calling convention)
	sub RSP, 20h
	 
	call OFFSET ProfileEnter

	; move RSP back, undo space for params
	add RSP, 20h

	; Perform standard epilog
	pop RBX
	pop r11
	pop r10
	pop r9
	pop r8
	pop RDX
	pop RCX
	pop RAX

	ret
_penter ENDP

; Setup _pexit
_pexit PROC EXPORT
	; Perform standard prolog first
	push RAX
	push RCX
	push RDX
	push r8
	push r9
	push r10
	push r11
	push RBX ; Pushing an extra register to align stack w/ multiple of 16
	
	; get the retun address off the stack & store to RCX
	mov RCX, [RSP + 40h]

	; create space on the stack for params [RCX, RDX, R8, R9]  (C calling convention)
	sub RSP, 20h
	
	call OFFSET ProfileExit 

	; move RSP back, undo space for params
	add RSP, 20h

	; Perform standard epilog
	pop RBX
	pop r11
	pop r10
	pop r9
	pop r8
	pop RDX
	pop RCX
	pop RAX

	ret
_pexit ENDP

END