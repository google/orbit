//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "Disassembler.h"

#include "..\external\capstone\include\platform.h"
#include "..\external\capstone\include\capstone.h"

//-----------------------------------------------------------------------------
void Disassembler::LogHex( const unsigned char *str, size_t len )
{
    const unsigned char *c;

    LOGF( "Code: " );
    for( c = str; c < str + len; c++ ) 
    {
        LOGF( "0x%02x ", *c & 0xff );
    }
    LOGF( "\n" );
}

//-----------------------------------------------------------------------------
void Disassembler::Disassemble( const unsigned char* a_MachineCode, int a_Size, DWORD64 a_Address, bool a_Is64Bit )
{
    std::wstring disAsm;
    csh handle;
    cs_arch arch = CS_ARCH_X86;
    cs_insn *insn;
    size_t count;
    cs_err err;
    cs_mode mode = a_Is64Bit ? CS_MODE_64 : CS_MODE_32;

    LOGF( "\n" );
    LOGF( "Platform: %s\n", a_Is64Bit ? "X86 64 (Intel syntax)" : "X86 32 (Intel syntax)" );
    err = cs_open( arch, mode, &handle );
    if( err ) 
    {
        LOGF( "Failed on cs_open() with error returned: %u\n", err );
        return;
    }

    count = cs_disasm( handle, a_MachineCode, a_Size, a_Address, 0, &insn );

    if( count ) 
    {
        size_t j;

        for( j = 0; j < count; j++ ) 
        {
            LOGF( "0x%" PRIx64 ":\t%-12s %s\n"
                , insn[j].address
                , insn[j].mnemonic
                , insn[j].op_str );
        }

        // print out the next offset, after the last insn
        LOGF( "0x%" PRIx64 ":\n", insn[j - 1].address + insn[j - 1].size );


        // free memory allocated by cs_disasm()
        cs_free( insn, count );
    }
    else 
    {
        LOGF( "****************\n" );
        LOGF( "ERROR: Failed to disasm given code!\n" );
    }

    LOGF( "\n" );

    cs_close( &handle );
}