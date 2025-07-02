; (C) 2025 DigiPen (USA) Corporation

.data
EXTERNDEF SyncEngineProfilerEnterFunc:PROC
EXTERNDEF SyncEngineProfilerExitFunc:PROC

.code

_penter PROC EXPORT
    push RAX
    push RCX
    push RDX
    push r8
    push r9
    push r10
    push r11
    ; push arbitrary reg for stack alignment
    push RBX

    mov RCX, [RSP + 40h]  ; get return address

    sub RSP, 20h  ; create space for params (RCX, RDX, R8, R9)

    call OFFSET SyncEngineProfilerEnterFunc

    add RSP, 20h

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


_pexit PROC EXPORT
    push RAX
    push RCX
    push RDX
    push r8
    push r9
    push r10
    push r11
    ; push arbitrary reg for stack alignment
    push RBX

    mov RCX, [RSP + 40h]  ; get return address

    sub RSP, 20h  ; create space for params (RCX, RDX, R8, R9)

    call OFFSET SyncEngineProfilerExitFunc

    add RSP, 20h

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
