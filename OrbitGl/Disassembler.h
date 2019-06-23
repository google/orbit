//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include <string>
#include "BaseTypes.h"

#include "../OrbitCore/Utils.h"

//-----------------------------------------------------------------------------
class Disassembler
{
public:
    void Disassemble( const unsigned char* a_MachineCode, size_t a_Size, DWORD64 a_Address, bool a_Is64Bit );
    const std::wstring & GetResult() { return m_String; }

    template<typename... Args>
    inline void LOGF( const char* const _Format, Args&&... args )
    {
        std::string log = Format( _Format, std::forward<Args>( args )... );
        m_String += s2ws(log);
    }

    void LogHex( const unsigned char *str, size_t len );

public:
    std::wstring m_String;
};