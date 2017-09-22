.DATA 
.CODE

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