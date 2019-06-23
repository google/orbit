#include "OrbitAsm.h"
#include <sstream>
#include <vector>

OrbitProlog GProlog;
OrbitEpilog GEpilog;

//-----------------------------------------------------------------------------
#ifdef _WIN64
std::vector<byte> dummyEnd     = { 0x49, 0xBB, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x0F };
std::vector<byte> dummyAddress = { 0xEF, 0xCD, 0xAB, 0x89, 0x67, 0x45, 0x23, 0x01 };
#else
std::vector<byte> dummyEnd     = { 0xB8, 0xFF, 0xFF, 0xFF, 0x0F };
std::vector<byte> dummyAddress = { 0x78, 0x56, 0x34, 0x12 };
#endif

//-----------------------------------------------------------------------------
#define PRINT_VAR( var )	    PrintVar( #var, var )
#define PRINT_VARN( name, var )	PrintVar( name, var )
#define PRINT_VAR_INL( var )    PrintVar( #var, var, true )

//-----------------------------------------------------------------------------
template<class T>
inline void PrintVar( const char* a_VarName, const T& a_Value, bool a_SameLine = false )
{
    std::stringstream l_StringStream;
    l_StringStream << a_VarName << " = " << a_Value;
    if( !a_SameLine ) l_StringStream << std::endl;
    OutputDebugStringA( l_StringStream.str().c_str() );
}

//-----------------------------------------------------------------------------
size_t FindSize( const byte* a_Code, size_t a_MaxBytes = 1024 )
{
    size_t matchSize = dummyEnd.size();

    for( size_t i = 0; i < a_MaxBytes; ++i )
    {
        size_t j = 0;
        for( j = 0; j < matchSize; ++j )
        {
            if( a_Code[i+j] != dummyEnd[j] )
                break;
        }

        if( j == matchSize )
            return i;
    }
    
    return 0;
}

//-----------------------------------------------------------------------------
std::vector<size_t> FindOffsets( const byte* a_Code, size_t a_NumOffsets, const std::vector<byte> & a_Identifier, size_t a_MaxBytes = 1024 )
{
    size_t matchSize = a_Identifier.size();
    std::vector<size_t> offsets;

    for( size_t i = 0; i < a_MaxBytes; ++i )
    {
        size_t j = 0;
        for( j = 0; j < matchSize; ++j )
        {
            if( a_Code[i + j] != a_Identifier[j] )
                break;
        }

        if( j == matchSize )
        {
            offsets.push_back(i);
            i += matchSize;
            if( offsets.size() == a_NumOffsets )
                break;
        }
    }

    return offsets;
}

//-----------------------------------------------------------------------------
void OrbitProlog::Init()
{
    if( m_Data.m_Code == nullptr )
    {
        m_Data.m_Code = (byte*)&OrbitPrologAsm;
        m_Data.m_Size = FindSize( m_Data.m_Code );
        PRINT_VARN( "PrologSize", m_Data.m_Size );
        
        std::vector<size_t> offsets = FindOffsets( m_Data.m_Code, Prolog_NumOffsets, dummyAddress );
        
        if( offsets.size() != Prolog_NumOffsets )
        {
            OutputDebugStringA( "OrbitAsm: Did not find expected number of offsets in the Prolog\n" );
            return;
        }

        for( size_t i = 0; i < offsets.size(); ++i )
        {
            m_Data.m_Offsets[i] = offsets[i];
        }
    }
}

//-----------------------------------------------------------------------------
void OrbitEpilog::Init()
{
    if( !m_Data.m_Code )
    {
        m_Data.m_Code = (byte*)&OrbitEpilogAsm;
        m_Data.m_Size = FindSize( m_Data.m_Code );
        PRINT_VARN( "EpilogSize", m_Data.m_Size );

        std::vector<size_t> offsets = FindOffsets( m_Data.m_Code, Epilog_NumOffsets, dummyAddress );

        if( offsets.size() != Epilog_NumOffsets )
        {
            OutputDebugStringA( "OrbitAsm: Did not find expected number of offsets in the Epilog\n" );
            return;
        }
        
        for( int i = 0; i < Epilog_NumOffsets; ++i )
        {
            m_Data.m_Offsets[i] = offsets[i];
        }
    }
}

//-----------------------------------------------------------------------------
const Prolog* GetOrbitProlog()
{
    if( !GProlog.m_Data.m_Code )
        GProlog.Init();
    return &GProlog.m_Data;
}

//-----------------------------------------------------------------------------
const Epilog* GetOrbitEpilog()
{
    if( !GEpilog.m_Data.m_Code )
        GEpilog.Init();
    return &GEpilog.m_Data;
}

#ifndef _WIN64

//-----------------------------------------------------------------------------
__declspec( naked ) void OrbitPrologAsm()
{
    __asm
    {
        push    ebp
        mov     ebp, esp
        push    eax
        push    ecx
        push    edx

        sub     esp, 64
        movdqu xmmword ptr[esp+48], xmm0
        movdqu xmmword ptr[esp+32], xmm1
        movdqu xmmword ptr[esp+16], xmm2
        movdqu xmmword ptr[esp+0],  xmm3

        sub     ebp, esp
        push    ebp                         // Pass in size of context
        lea     eax, [esp-8]
        push    eax                         // Pass in context
        mov     ecx, 0x12345678             // Pass in address of original function
        push    ecx
        mov     eax, 0x12345678             // Set address of user prolog
        call    eax                         // Call user prolog
        add     esp, 12                     // Clear args from stack frame

        movdqu xmm3, xmmword ptr[esp+0]
        movdqu xmm2, xmmword ptr[esp+16]
        movdqu xmm1, xmmword ptr[esp+32]
        movdqu xmm0, xmmword ptr[esp+48]
        add     esp, 64

        pop     edx
        pop     ecx
        pop     eax
        pop     ebp

        mov     dword ptr[esp], 0x12345678  // Overwrite return address with address of OrbitEpilog
        mov     eax, 0x12345678             // Address of trampoline to original function
        jmp     eax                         // Jump to trampoline to original function
        mov     eax, 0x0FFFFFFF             // Dummy function delimiter, never executed
    }
}

//-----------------------------------------------------------------------------
__declspec( naked ) void OrbitEpilogAsm()
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
        mov     eax, 0x0FFFFFFF        // Dummy function delimiter, never executed
    }
}

#endif
