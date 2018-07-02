.DATA 
.CODE

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
	mov     R11, 0FFFFFFFFFFFFFFFh  ;// Dummy function delimiter, never executed
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
    mov     R11, 0123456789ABCDEFh  ;// Will be overwritten by callback address
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
	mov     R11, 0FFFFFFFFFFFFFFFh  ;// Dummy function delimiter, never executed
OrbitEpilogAsm ENDP

OrbitGetSSEContext PROC
movdqu xmmword ptr[RCX], xmm0
add    RCX , 16
movdqu xmmword ptr[RCX], xmm1
add    RCX , 16
movdqu xmmword ptr[RCX], xmm2
add    RCX , 16
movdqu xmmword ptr[RCX], xmm3
add    RCX , 16
movdqu xmmword ptr[RCX], xmm4
add    RCX , 16
movdqu xmmword ptr[RCX], xmm5
add    RCX , 16
movdqu xmmword ptr[RCX], xmm6
add    RCX , 16
movdqu xmmword ptr[RCX], xmm7
add    RCX , 16
movdqu xmmword ptr[RCX], xmm8
add    RCX , 16
movdqu xmmword ptr[RCX], xmm9
add    RCX , 16
movdqu xmmword ptr[RCX], xmm10
add    RCX , 16
movdqu xmmword ptr[RCX], xmm11
add    RCX , 16
movdqu xmmword ptr[RCX], xmm12
add    RCX , 16
movdqu xmmword ptr[RCX], xmm13
add    RCX , 16
movdqu xmmword ptr[RCX], xmm14
add    RCX , 16
movdqu xmmword ptr[RCX], xmm15
ret
OrbitGetSSEContext ENDP

OrbitSetSSEContext PROC
movdqu xmm0, xmmword ptr[RCX]
add    RCX , 16
movdqu xmm1, xmmword ptr[RCX]
add    RCX , 16
movdqu xmm2, xmmword ptr[RCX]
add    RCX , 16
movdqu xmm3, xmmword ptr[RCX]
add    RCX , 16
movdqu xmm4, xmmword ptr[RCX]
add    RCX , 16
movdqu xmm5, xmmword ptr[RCX]
add    RCX , 16
movdqu xmm6, xmmword ptr[RCX]
add    RCX , 16
movdqu xmm7, xmmword ptr[RCX]
add    RCX , 16
movdqu xmm8, xmmword ptr[RCX]
add    RCX , 16
movdqu xmm9, xmmword ptr[RCX]
add    RCX , 16
movdqu xmm10, xmmword ptr[RCX]
add    RCX , 16
movdqu xmm11, xmmword ptr[RCX]
add    RCX , 16
movdqu xmm12, xmmword ptr[RCX]
add    RCX , 16
movdqu xmm13, xmmword ptr[RCX]
add    RCX , 16
movdqu xmm14, xmmword ptr[RCX]
add    RCX , 16
movdqu xmm15, xmmword ptr[RCX]
ret
OrbitSetSSEContext ENDP

END