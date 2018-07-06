.DATA 
.CODE

; https://msdn.microsoft.com/en-us/library/9z1stfyw.aspx
; Register      Status          Use
; RAX           Volatile        Return value register
; RCX           Volatile        First integer argument
; RDX           Volatile        Second integer argument
; R8            Volatile        Third integer argument
; R9            Volatile        Fourth integer argument
; R10:R11       Volatile        Must be preserved as needed by caller; used in syscall/sysret instructions
; R12:R15       Nonvolatile     Must be preserved by callee
; RDI           Nonvolatile     Must be preserved by callee
; RSI           Nonvolatile     Must be preserved by callee
; RBX           Nonvolatile     Must be preserved by callee
; RBP           Nonvolatile     May be used as a frame pointer; must be preserved by callee
; RSP           Nonvolatile     Stack pointer
; XMM0, YMM0    Volatile        First FP argument; first vector-type argument when __vectorcall is used
; XMM1, YMM1    Volatile        Second FP argument; second vector-type argument when __vectorcall is used
; XMM2, YMM2    Volatile        Third FP argument; third vector-type argument when __vectorcall is used
; XMM3, YMM3    Volatile        Fourth FP argument; fourth vector-type argument when __vectorcall is used
; XMM4, YMM4    Volatile        Must be preserved as needed by caller; fifth vector-type argument when __vectorcall is used
; XMM5, YMM5    Volatile        Must be preserved as needed by caller; sixth vector-type argument when __vectorcall is used
; XMM6:XMM15, YMM6:YMM15        Nonvolatile (XMM), Volatile (upper half of YMM) Must be preserved as needed by callee. YMM registers must be preserved as needed by caller.

OrbitPrologAsm PROC
    sub     RSP, 8                  ;// will hold address of trampoline
    push    RBP                     
    mov     RBP, RSP
    
    push    RAX                     ;// Save volatile registers
    push    RCX
    push    RDX
    push    R8
    push    R9
    push    R10
    push    R11

    mov     RDX, RSP                ;// pointer to context structure
    mov     R8,  RBP                ;// compute size of context for sanity check
    sub     R8,  RSP

    sub     RSP, 20h                ;// Shadow space - NOTE: stack pointer needs to be aligned on 16 bytes at this point                

                                    ;// CALL USER PROLOG - void Prolog( void* a_OriginalFunctionAddress, void* a_Context, unsigned a_ContextSize );
    mov     RCX, 0123456789ABCDEFh  ;// Pass in address of original function
    mov     RAX, 0123456789ABCDEFh  ;// Will be ovewritten with callback address
    call    RAX                     ;// User prolog function  

    add     RSP, 20h
                                    ;// OVERWRITE RETURN ADDRESS
    mov     R10, 0123456789ABCDEFh  ;// will be overwritten with epilog address
    mov     qword ptr[RBP+16], R10  ;// overwrite return address with epilog address


    mov     R11, 0123456789ABCDEFh  ;// Will be ovewritten with address of trampoline to original function
    mov     qword ptr[RBP+8], R11   ;// Write address of trampoline for ret instruction

    pop     R11
    pop     R10
    pop     R9
    pop     R8
    pop     RDX
    pop     RCX
    pop     RAX

    pop     RBP
    ret                             ;// Jump to orignial function through trampoline
    mov     R11, 0FFFFFFFFFFFFFFFh  ;// Dummy function delimiter, never executed
OrbitPrologAsm  ENDP


OrbitEpilogAsm PROC
    push    RAX                     ;// Save RAX (return value)
    push    RBX
    push    RCX
    push    RDX
    push    R8
    push    R9
    sub     RSP , 16                
    movdqu  xmmword ptr[RSP], xmm0  ;// Save XMM0 (float return value)
    mov     R11, 0123456789ABCDEFh  ;// Will be overwritten by callback address
    sub     RSP, 20h                 
    call    R11                     ;// Call user epilog (returns original caller address)
    add     RSP, 20h                 
    mov     R11, RAX                ;// RDX contains return address
    movdqu  xmm0, xmmword ptr[RSP]  ;// XMM0 contains float return value
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
movdqu xmmword ptr[RCX+0*16],  xmm0
movdqu xmmword ptr[RCX+1*16],  xmm1
movdqu xmmword ptr[RCX+2*16],  xmm2
movdqu xmmword ptr[RCX+3*16],  xmm3
movdqu xmmword ptr[RCX+4*16],  xmm4
movdqu xmmword ptr[RCX+5*16],  xmm5
movdqu xmmword ptr[RCX+6*16],  xmm6
movdqu xmmword ptr[RCX+7*16],  xmm7
movdqu xmmword ptr[RCX+8*16],  xmm8
movdqu xmmword ptr[RCX+9*16],  xmm9
movdqu xmmword ptr[RCX+10*16], xmm10
movdqu xmmword ptr[RCX+11*16], xmm11
movdqu xmmword ptr[RCX+12*16], xmm12
movdqu xmmword ptr[RCX+13*16], xmm13
movdqu xmmword ptr[RCX+14*16], xmm14
movdqu xmmword ptr[RCX+15*16], xmm15
ret
OrbitGetSSEContext ENDP

OrbitSetSSEContext PROC
movdqu xmm0,  xmmword ptr[RCX+0*16]
movdqu xmm1,  xmmword ptr[RCX+1*16]
movdqu xmm2,  xmmword ptr[RCX+2*16]
movdqu xmm3,  xmmword ptr[RCX+3*16]
movdqu xmm4,  xmmword ptr[RCX+4*16]
movdqu xmm5,  xmmword ptr[RCX+5*16]
movdqu xmm6,  xmmword ptr[RCX+6*16]
movdqu xmm7,  xmmword ptr[RCX+7*16]
movdqu xmm8,  xmmword ptr[RCX+8*16]
movdqu xmm9,  xmmword ptr[RCX+9*16]
movdqu xmm10, xmmword ptr[RCX+10*16]
movdqu xmm11, xmmword ptr[RCX+11*16]
movdqu xmm12, xmmword ptr[RCX+12*16]
movdqu xmm13, xmmword ptr[RCX+13*16]
movdqu xmm14, xmmword ptr[RCX+14*16]
movdqu xmm15, xmmword ptr[RCX+15*16]
ret
OrbitSetSSEContext ENDP

END