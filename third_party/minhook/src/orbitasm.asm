.code

OrbitPrologAsm PROC
    mov     R11, RSP                ; //Save address of return address
    push    RCX
	push    RDX
	push    R8
	push    R9
    push    RBX

    sub     RSP , 16
    movdqu xmmword ptr[RSP], xmm0
    sub     RSP , 16
    movdqu xmmword ptr[RSP], xmm1
    sub     RSP , 16
    movdqu xmmword ptr[RSP], xmm2
    sub     RSP , 16
    movdqu xmmword ptr[RSP], xmm3

    push    R11                     ;// Store value of address of return address on stack
    push    RAX                     ;// NOTE: stack pointer needs to be aligned on 16 bytes at this point

    sub     RSP, 20h 

                                    ;// CALL USER PROLOG 
    mov     RCX, 0123456789ABCDEFh  ;// Pass in address of original function
    mov     RDX, R11                ;// Pass in address of return address via RDX (might be modified by callee)
    mov     RAX, 0123456789ABCDEFh  ;// Will be ovewritten with callback address
    call    RAX                     ;// User prolog function  

    add     RSP, 20h
    
    pop     RAX
    pop     R11
                                    ;// OVERWRITE RETURN ADDRESS
    mov     R10, 0123456789ABCDEFh  ;// will be overwritten with epilog address
    mov     qword ptr[R11], R10     ;// overwrite return address with epilog address

    movdqu xmm3, xmmword ptr[RSP]
    add     RSP , 16
    movdqu xmm2, xmmword ptr[RSP]
    add     RSP , 16
    movdqu xmm1, xmmword ptr[RSP]
    add     RSP , 16
    movdqu xmm0, xmmword ptr[RSP]
    add     RSP , 16

    pop     RBX
    pop     R9
    pop     R8
    pop     RDX
    pop     RCX

    mov     R11, 0123456789ABCDEFh  ;// Will be ovewritten with address of trampoline to original function
    jmp     R11                     ;// Jump to orignial function through trampolime TODO: Make this a relative jump for better perf and 7 bytes saving
OrbitPrologAsm	ENDP


OrbitEpilogAsm PROC
    push    RAX                     ;// Save eax (return value)
	push    RBX
	push    RCX
	push    RDX
	push    R8
	push    R9
    sub     RSP , 16                
    movdqu xmmword ptr[RSP], xmm0   ;// Save XMM0 (float return value)
    mov     R11, 0123456789ABCDEFh
    sub     RSP, 20h
    call    R11                     ;// Call user epilog (returns original caller address)
    add     RSP, 20h
    mov     R11, RAX                ;// RDX contains return address
    movdqu xmm0, xmmword ptr[RSP]   ;// XMM0 contains float return value
    add     RSP , 16                
	pop     R9
	pop     R8
	pop     RDX
	pop     RCX
	pop     RBX
    pop     RAX                     ;// RAX contains return value
    push    R11                     ;// Push caller address on stack
    ret                             ;// return
OrbitEpilogAsm ENDP

END