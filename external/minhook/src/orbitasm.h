#pragma once 

#ifdef _M_X64
char OrbitProlog[] = 
{ 0x4C, 0x8B, 0xDC
, 0x51
, 0x52
, 0x41, 0x50
, 0x41, 0x51
, 0x53
, 0x48, 0x83, 0xEC, 0x10
, 0xF3, 0x0F, 0x7F, 0x04, 0x24
, 0x48, 0x83, 0xEC, 0x10
, 0xF3, 0x0F, 0x7F, 0x0C, 0x24
, 0x48, 0x83, 0xEC, 0x10
, 0xF3, 0x0F, 0x7F, 0x14, 0x24
, 0x48, 0x83, 0xEC, 0x10
, 0xF3, 0x0F, 0x7F, 0x1C, 0x24
, 0x41, 0x53
, 0x50
, 0x48, 0x83, 0xEC, 0x20
, 0x48, 0xB9, 0xEF, 0xCD, 0xAB, 0x89, 0x67, 0x45, 0x23, 0x01
, 0x49, 0x8B, 0xD3
, 0x48, 0xB8, 0xEF, 0xCD, 0xAB, 0x89, 0x67, 0x45, 0x23, 0x01
, 0xFF, 0xD0
, 0x48, 0x83, 0xC4, 0x20
, 0x58
, 0x41, 0x5B
, 0x49, 0xBA, 0xEF, 0xCD, 0xAB, 0x89, 0x67, 0x45, 0x23, 0x01
, 0x4D, 0x89, 0x13
, 0xF3, 0x0F, 0x6F, 0x1C, 0x24
, 0x48, 0x83, 0xC4, 0x10
, 0xF3, 0x0F, 0x6F, 0x14, 0x24
, 0x48, 0x83, 0xC4, 0x10
, 0xF3, 0x0F, 0x6F, 0x0C, 0x24
, 0x48, 0x83, 0xC4, 0x10
, 0xF3, 0x0F, 0x6F, 0x04, 0x24
, 0x48, 0x83, 0xC4, 0x10
, 0x5B
, 0x41, 0x59
, 0x41, 0x58
, 0x5A
, 0x59
, 0x49, 0xBB, 0xEF, 0xCD, 0xAB, 0x89, 0x67, 0x45, 0x23, 0x01
, 0x41, 0xFF, 0xE3 };

enum OrbitPrologOffset64
{
    Prolog_OriginalFunction = 55,
    Prolog_CallbackAddress  = 68,
    Prolog_EpilogAddress    = 87,
    Prolog_OriginalAddress  = 143
};

/*
00007FF7206312D0 4C 8B DC                      mov         r11,rsp
00007FF7206312D3 51                            push        rcx
00007FF7206312D4 52                            push        rdx
00007FF7206312D5 41 50                         push        r8
00007FF7206312D7 41 51                         push        r9
00007FF7206312D9 53                            push        rbx
00007FF7206312DA 48 83 EC 10                   sub         rsp,10h
00007FF7206312DE F3 0F 7F 04 24                movdqu      xmmword ptr [rsp],xmm0
00007FF7206312E3 48 83 EC 10                   sub         rsp,10h
00007FF7206312E7 F3 0F 7F 0C 24                movdqu      xmmword ptr [rsp],xmm1
00007FF7206312EC 48 83 EC 10                   sub         rsp,10h
00007FF7206312F0 F3 0F 7F 14 24                movdqu      xmmword ptr [rsp],xmm2
00007FF7206312F5 48 83 EC 10                   sub         rsp,10h
00007FF7206312F9 F3 0F 7F 1C 24                movdqu      xmmword ptr [rsp],xmm3
00007FF7206312FE 41 53                         push        r11
00007FF720631300 50                            push        rax
00007FF720631301 48 83 EC 20                   sub         rsp,20h
00007FF720631305 48 B9 EF CD AB 89 67 45 23 01 mov         rcx,123456789ABCDEFh
00007FF72063130F 49 8B D3                      mov         rdx,r11
00007FF720631312 48 B8 EF CD AB 89 67 45 23 01 mov         rax,123456789ABCDEFh
00007FF72063131C FF D0                         call        rax
00007FF72063131E 48 83 C4 20                   add         rsp,20h
00007FF720631322 58                            pop         rax
00007FF720631323 41 5B                         pop         r11
00007FF720631325 49 BA EF CD AB 89 67 45 23 01 mov         r10,123456789ABCDEFh
00007FF72063132F 4D 89 13                      mov         qword ptr [r11],r10
00007FF720631332 F3 0F 6F 1C 24                movdqu      xmm3,xmmword ptr [rsp]
00007FF720631337 48 83 C4 10                   add         rsp,10h
00007FF72063133B F3 0F 6F 14 24                movdqu      xmm2,xmmword ptr [rsp]
00007FF720631340 48 83 C4 10                   add         rsp,10h
00007FF720631344 F3 0F 6F 0C 24                movdqu      xmm1,xmmword ptr [rsp]
00007FF720631349 48 83 C4 10                   add         rsp,10h
00007FF72063134D F3 0F 6F 04 24                movdqu      xmm0,xmmword ptr [rsp]
00007FF720631352 48 83 C4 10                   add         rsp,10h
00007FF720631356 5B                            pop         rbx
00007FF720631357 41 59                         pop         r9
00007FF720631359 41 58                         pop         r8
00007FF72063135B 5A                            pop         rdx
00007FF72063135C 59                            pop         rcx
00007FF72063135D 49 BB EF CD AB 89 67 45 23 01 mov         r11,123456789ABCDEFh
00007FF720631367 41 FF E3                      jmp         r11
*/


char OrbitEpilog[] =
{ 0x50
, 0x53
, 0x51
, 0x52
, 0x41, 0x50
, 0x41, 0x51
, 0x48, 0x83, 0xEC, 0x10
, 0xF3, 0x0F, 0x7F, 0x04, 0x24
, 0x49, 0xBB, 0xEF, 0xCD, 0xAB, 0x89, 0x67, 0x45, 0x23, 0x01
, 0x48, 0x83, 0xEC, 0x20
, 0x41, 0xFF, 0xD3
, 0x48, 0x83, 0xC4, 0x20
, 0x4C, 0x8B, 0xD8
, 0xF3, 0x0F, 0x6F, 0x04, 0x24
, 0x48, 0x83, 0xC4, 0x10
, 0x41, 0x59
, 0x41, 0x58
, 0x5A
, 0x59
, 0x5B
, 0x58
, 0x41, 0x53
, 0xC3 };

enum OrbitEpilogOffset
{
    Epilog_CallbackAddress = 19
};

/*
00007FF7F1C7136A 50                            push        rax
00007FF7F1C7136B 53                            push        rbx
00007FF7F1C7136C 51                            push        rcx
00007FF7F1C7136D 52                            push        rdx
00007FF7F1C7136E 41 50                         push        r8
00007FF7F1C71370 41 51                         push        r9
00007FF7F1C71372 48 83 EC 10                   sub         rsp,10h
00007FF7F1C71376 F3 0F 7F 04 24                movdqu      xmmword ptr [rsp],xmm0
00007FF7F1C7137B 49 BB EF CD AB 89 67 45 23 01 mov         r11,123456789ABCDEFh
00007FF7F1C71385 48 83 EC 20                   sub         rsp,20h
00007FF7F1C71389 41 FF D3                      call        r11
00007FF7F1C7138C 48 83 C4 20                   add         rsp,20h
00007FF7F1C71390 4C 8B D8                      mov         r11,rax
00007FF7F1C71393 F3 0F 6F 04 24                movdqu      xmm0,xmmword ptr [rsp]
00007FF7F1C71398 48 83 C4 10                   add         rsp,10h
00007FF7F1C7139C 41 59                         pop         r9
00007FF7F1C7139E 41 58                         pop         r8
00007FF7F1C713A0 5A                            pop         rdx
00007FF7F1C713A1 59                            pop         rcx
00007FF7F1C713A2 5B                            pop         rbx
00007FF7F1C713A3 58                            pop         rax
00007FF7F1C713A4 41 53                         push        r11
00007FF7F1C713A6 C3                            ret
*/

#endif // #if _M_X64


#ifndef _M_X64

__declspec( noinline ) __declspec( naked ) void OrbitPrologAsm()
{
    __asm
    {
        push    ebp
        push    eax
        push    ecx
        push    edx

        sub     esp, 16
        movdqu xmmword ptr[esp], xmm0
        sub     esp, 16
        movdqu xmmword ptr[esp], xmm1
        sub     esp, 16
        movdqu xmmword ptr[esp], xmm2
        sub     esp, 16
        movdqu xmmword ptr[esp], xmm3

        mov     eax, esp
        add     eax, 80
        push    eax                         // Pass in address of return address
        mov     ecx, 0x12345678             // Pass in address of original function
        push    ecx
        mov     eax, 0x12345678             // Set address of user prolog
        call    eax                         // Call user prolog
        add     esp, 8                      // Clear args from stack frame

        movdqu xmm3, xmmword ptr[esp]
        add     esp, 16
        movdqu xmm2, xmmword ptr[esp]
        add     esp, 16
        movdqu xmm1, xmmword ptr[esp]
        add     esp, 16
        movdqu xmm0, xmmword ptr[esp]
        add     esp, 16

        pop     edx
        pop     ecx
        pop     eax
        pop     ebp

        mov     dword ptr[esp], 0x12345678  // Overwrite return address with address of OrbitEpilog
        mov     eax, 0x12345678             // Address of trampoline to original function
        jmp     eax                         // Jump to trampoline to original function
    }
}

/*
00E11376 55                   push        ebp
00E11377 50                   push        eax
00E11378 51                   push        ecx
00E11379 52                   push        edx
00E1137A 83 EC 10             sub         esp,10h
00E1137D F3 0F 7F 04 24       movdqu      xmmword ptr [esp],xmm0
00E11382 83 EC 10             sub         esp,10h
00E11385 F3 0F 7F 0C 24       movdqu      xmmword ptr [esp],xmm1
00E1138A 83 EC 10             sub         esp,10h
00E1138D F3 0F 7F 14 24       movdqu      xmmword ptr [esp],xmm2
00E11392 83 EC 10             sub         esp,10h
00E11395 F3 0F 7F 1C 24       movdqu      xmmword ptr [esp],xmm3
00E1139A 8B C4                mov         eax,esp
00E1139C 83 C0 50             add         eax,50h
00E1139F 50                   push        eax
00E113A0 B9 78 56 34 12       mov         ecx,12345678h
00E113A5 51                   push        ecx
00E113A6 B8 78 56 34 12       mov         eax,12345678h
00E113AB FF D0                call        eax
00E113AD 83 C4 08             add         esp,8
00E113B0 F3 0F 6F 1C 24       movdqu      xmm3,xmmword ptr [esp]
00E113B5 83 C4 10             add         esp,10h
00E113B8 F3 0F 6F 14 24       movdqu      xmm2,xmmword ptr [esp]
00E113BD 83 C4 10             add         esp,10h
00E113C0 F3 0F 6F 0C 24       movdqu      xmm1,xmmword ptr [esp]
00E113C5 83 C4 10             add         esp,10h
00E113C8 F3 0F 6F 04 24       movdqu      xmm0,xmmword ptr [esp]
00E113CD 83 C4 10             add         esp,10h
00E113D0 5A                   pop         edx
00E113D1 59                   pop         ecx
00E113D2 58                   pop         eax
00E113D3 5D                   pop         ebp
00E113D4 C7 04 24 78 56 34 12 mov         dword ptr [esp],12345678h
00E113DB B8 78 56 34 12       mov         eax,12345678h
00E113E0 FF E0                jmp         eax
*/

char OrbitProlog[] = 
{ 0x55
, 0x50
, 0x51
, 0x52
, 0x83, 0xEC, 0x10
, 0xF3, 0x0F, 0x7F, 0x04, 0x24
, 0x83, 0xEC, 0x10
, 0xF3, 0x0F, 0x7F, 0x0C, 0x24
, 0x83, 0xEC, 0x10
, 0xF3, 0x0F, 0x7F, 0x14, 0x24
, 0x83, 0xEC, 0x10
, 0xF3, 0x0F, 0x7F, 0x1C, 0x24
, 0x8B, 0xC4
, 0x83, 0xC0, 0x50
, 0x50
, 0xB9, 0x78, 0x56, 0x34, 0x12
, 0x51
, 0xB8, 0x78, 0x56, 0x34, 0x12
, 0xFF, 0xD0
, 0x83, 0xC4, 0x08
, 0xF3, 0x0F, 0x6F, 0x1C, 0x24
, 0x83, 0xC4, 0x10
, 0xF3, 0x0F, 0x6F, 0x14, 0x24
, 0x83, 0xC4, 0x10
, 0xF3, 0x0F, 0x6F, 0x0C, 0x24
, 0x83, 0xC4, 0x10
, 0xF3, 0x0F, 0x6F, 0x04, 0x24
, 0x83, 0xC4, 0x10
, 0x5A
, 0x59
, 0x58
, 0x5D
, 0xC7, 0x04, 0x24, 0x78, 0x56, 0x34, 0x12
, 0xB8, 0x78, 0x56, 0x34, 0x12
, 0xFF, 0xE0 };

enum HookPrologOffset
{
    Prolog_OriginalFunction= 43,
    Prolog_CallbackAddress = 49,
    Prolog_EpilogAddress   = 97,
    Prolog_OriginalAddress = 102
};

__declspec(naked) void HookEpilogAsm()
{
    __asm
    {
        push    eax                    // Save eax (return value)
        sub     esp, 16
        movdqu xmmword ptr[ESP], xmm0; // Save XMM0 (float return value)
        mov     ecx, 0x12345678
        call    ecx                    // Call user epilog (returns original caller address)
        mov     edx, eax               // edx contains caller address
        movdqu xmm0, xmmword ptr[ESP]; // XMM0 contains float return value
        add     ESP, 16
        pop     eax                    // eax contains return value
        push    edx                    // Push caller address on stack
        ret
    }
}

/*
013219C9 50                   push        eax
013219CA 83 EC 10             sub         esp,10h
013219CD F3 0F 7F 04 24       movdqu      xmmword ptr [esp],xmm0
013219D2 B9 78 56 34 12       mov         ecx,12345678h
013219D7 FF D1                call        ecx
013219D9 8B D0                mov         edx,eax
013219DB F3 0F 6F 04 24       movdqu      xmm0,xmmword ptr [esp]
013219E0 83 C4 10             add         esp,10h
013219E3 58                   pop         eax
013219E4 52                   push        edx
013219E5 C3                   ret
*/

char OrbitEpilog[] =
{ 0x50
, 0x83, 0xEC, 0x10
, 0xF3, 0x0F, 0x7F, 0x04, 0x24
, 0xB9, 0x78, 0x56, 0x34, 0x12
, 0xFF, 0xD1
, 0x8B, 0xD0
, 0xF3, 0x0F, 0x6F, 0x04, 0x24
, 0x83, 0xC4, 0x10
, 0x58
, 0x52
, 0xC3 };

enum OrbitEpilogOffset
{
    Epilog_CallbackAddress = 10
};

#endif
