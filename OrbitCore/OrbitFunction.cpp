//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "Core.h"
#include "OrbitFunction.h"
#include "OrbitModule.h"
#include "Serialization.h"
#include "Pdb.h"
#include "Capture.h"
#include "Log.h"
#include "Params.h"
#include "OrbitProcess.h"
#include "TcpServer.h"
#include "SamplingProfiler.h"
#include "Utils.h"

#ifdef _WIN32
#include "OrbitDia.h"
#include "SymbolUtils.h"
#include "DiaParser.h"
#include <dia2.h>
#endif

//-----------------------------------------------------------------------------
Function::Function() 
	: m_Address(0)
	, m_Size(0)
	, m_Line(0)
	, m_ModBase(0)
	, m_Selected(0)
	, m_CallConv(-1)
	, m_Id(0)
	, m_ParentId(0)
	, m_Pdb(nullptr)
	, m_NameHash(0)
	, m_OrbitType(OrbitType::NONE)
{
}

//-----------------------------------------------------------------------------
Function::~Function()
{
}

//-----------------------------------------------------------------------------
void Function::SetAsMainFrameFunction()
{
    Capture::GMainFrameFunction = (ULONG64)m_Pdb->GetHModule() + this->m_Address;
    m_Selected = true;
}

//-----------------------------------------------------------------------------
const std::wstring & Function::PrettyName()
{
    if( m_PrettyName.size() == 0 )
    {
        m_PrettyName = m_Name;
    }

    return m_PrettyName;
}

//-----------------------------------------------------------------------------
bool Function::Hookable()
{
    // Don't allow hooking in asm implemented functions (strcpy, stccat...) 
    // TODO: give this better thought.  Here is the theory:
    // Functions that loop back to first 5 bytes of instructions will explode as
    // the IP lands in the middle of the relative jump instruction...
    // Ideally, we would detect such a loop back and not allow hooking.
    if( m_File.find( L".asm" ) != std::wstring::npos )
    {
        return false;
    }

    CV_call_e conv = (CV_call_e)m_CallConv;
    return ( ( conv == CV_CALL_NEAR_C || CV_CALL_THISCALL ) && m_Size >= 5 )
        || ( GParams.m_AllowUnsafeHooking && m_Size == 0 );
}

//-----------------------------------------------------------------------------
void Function::PreHook()
{
    // Unreal
    if( Capture::GUnrealSupported )
    {
        Type* parent = GetParentType();
        if( parent && parent->IsA( L"UObject" ) )
        {
            m_OrbitType = Function::UNREAL_ACTOR;
        }
    }

    // MemFunc
    if( IsMemoryFunc() )
    {
        //if( this->IsMemberFunction() )
    }
}

//-----------------------------------------------------------------------------
DWORD64 Function::GetVirtualAddress() const
{
    return m_Address + ( m_Pdb ? (DWORD64)m_Pdb->GetHModule() : 0 );
}

//-----------------------------------------------------------------------------
std::wstring Function::GetModuleName()
{
    if( m_Pdb )
    {
        return m_Pdb->GetName();
    }
    else
    {
        std::shared_ptr<Module> module = Capture::GTargetProcess->GetModuleFromAddress( m_Address );
        return module ? module->m_Name : L"";
    }
}

//-----------------------------------------------------------------------------
Type * Function::GetParentType()
{
    return m_ParentId ? &m_Pdb->GetTypeFromId( m_ParentId ) : nullptr;
}

//-----------------------------------------------------------------------------
void Function::ResetStats()
{
    if( m_Stats == nullptr )
    {
        m_Stats = std::make_shared<FunctionStats>();
    }
    else
    {
        m_Stats->Reset();
    }
}

//-----------------------------------------------------------------------------
void Function::GetDisassembly()
{
    if( m_Pdb && Capture::Connect() )
    {
        Message msg( Msg_GetData );
        ULONG64 address = (ULONG64)m_Pdb->GetHModule() + (ULONG64)m_Address;
        msg.m_Header.m_DataTransferHeader.m_Address = address;
        msg.m_Header.m_DataTransferHeader.m_Type = DataTransferHeader::Code;
        DWORD64 size = m_Size;

        // dll
        if( m_Size == 0 )
        {
            std::shared_ptr<Module> module = Capture::GTargetProcess->GetModuleFromAddress( address );
            if( module )
            {
                DWORD64 maxSize = module->m_AddressEnd - address;
                size = std::min( GParams.m_NumBytesAssembly, maxSize );
            }
        }

        msg.m_Size = (int)size;
        GTcpServer->Send( msg );
    }
}

void Function::FindFile()
{
#ifdef _WIN32
	LineInfo lineInfo;
	SymUtils::GetLineInfo( m_Address + (DWORD64)m_Pdb->GetHModule(), lineInfo );
	if( lineInfo.m_File != L"" )
		m_File = lineInfo.m_File;
	m_File = ToLower( m_File );
	m_Line = lineInfo.m_Line;
#endif
}

//-----------------------------------------------------------------------------
const TCHAR* Function::GetCallingConventionString( int a_CallConv )
{
    const TCHAR* CallingConv[] = {
        L"NEAR_C",      //CV_CALL_NEAR_C      = 0x00, // near right to left push, caller pops stack
        L"FAR_C",       //CV_CALL_FAR_C       = 0x01, // far right to left push, caller pops stack
        L"NEAR_PASCAL", //CV_CALL_NEAR_PASCAL = 0x02, // near left to right push, callee pops stack
        L"FAR_PASCAL",  //CV_CALL_FAR_PASCAL  = 0x03, // far left to right push, callee pops stack
        L"NEAR_FAST",   //CV_CALL_NEAR_FAST   = 0x04, // near left to right push with regs, callee pops stack
        L"FAR_FAST",    //CV_CALL_FAR_FAST    = 0x05, // far left to right push with regs, callee pops stack
        L"SKIPPED",     //CV_CALL_SKIPPED     = 0x06, // skipped (unused) call index
        L"NEAR_STD",    //CV_CALL_NEAR_STD    = 0x07, // near standard call
        L"FAR_STD",     //CV_CALL_FAR_STD     = 0x08, // far standard call
        L"NEAR_SYS",    //CV_CALL_NEAR_SYS    = 0x09, // near sys call
        L"FAR_SYS",     //CV_CALL_FAR_SYS     = 0x0a, // far sys call
        L"THISCALL",    //CV_CALL_THISCALL    = 0x0b, // this call (this passed in register)
        L"MIPSCALL",    //CV_CALL_MIPSCALL    = 0x0c, // Mips call
        L"GENERIC",     //CV_CALL_GENERIC     = 0x0d, // Generic call sequence
        L"ALPHACALL",   //CV_CALL_ALPHACALL   = 0x0e, // Alpha call
        L"PPCCALL",     //CV_CALL_PPCCALL     = 0x0f, // PPC call
        L"SHCALL",      //CV_CALL_SHCALL      = 0x10, // Hitachi SuperH call
        L"ARMCALL",     //CV_CALL_ARMCALL     = 0x11, // ARM call
        L"AM33CALL",    //CV_CALL_AM33CALL    = 0x12, // AM33 call
        L"TRICALL",     //CV_CALL_TRICALL     = 0x13, // TriCore Call
        L"SH5CALL",     //CV_CALL_SH5CALL     = 0x14, // Hitachi SuperH-5 call
        L"M32RCALL",    //CV_CALL_M32RCALL    = 0x15, // M32R Call
        L"CLRCALL",     //CV_CALL_CLRCALL     = 0x16, // clr call
        L"INLINE",      //CV_CALL_INLINE      = 0x17, // Marker for routines always inlined and thus lacking a convention
        L"NEAR_VECTOR", //CV_CALL_NEAR_VECTOR = 0x18, // near left to right push with regs, callee pops stack
        L"RESERVED" };  //CV_CALL_RESERVED    = 0x19  // first unused call enumeration

    return a_CallConv >= 0 ? CallingConv[a_CallConv] : L"UnknownCallConv";
}

//-----------------------------------------------------------------------------
ORBIT_SERIALIZE( Function, 1 )
{
    ORBIT_NVP_VAL( 0, m_Name );
    ORBIT_NVP_VAL( 0, m_PrettyName );
    ORBIT_NVP_VAL( 0, m_Address );
    ORBIT_NVP_VAL( 0, m_Size );
    ORBIT_NVP_VAL( 0, m_Module );
    ORBIT_NVP_VAL( 0, m_File );
    ORBIT_NVP_VAL( 0, m_Line );
    ORBIT_NVP_VAL( 0, m_ModBase );
    ORBIT_NVP_VAL( 0, m_CallConv );
    ORBIT_NVP_VAL( 1, m_Stats );
}

//-----------------------------------------------------------------------------
//Argument::Register Argument::GetArgRegister( int a_ArgIndex, bool isFloat )
//{
//    const Register IntRegs[] = { RCX, RDX, R8, R9 };
//    const Register FltRegs[] = { XMM0, XMM1, XMM2, XMM3 };
//
//    assert( a_ArgIndex <= 4 );
//
//    return !isFloat ? IntRegs[a_ArgIndex] : FltRegs[a_ArgIndex]; 
//}

//-----------------------------------------------------------------------------
bool FunctionParam::InRegister( int a_Index )
{
    return a_Index < 4;
}

//-----------------------------------------------------------------------------
bool FunctionParam::IsFloat()
{
    return ( m_Type.find( TEXT( "float" ) ) != std::wstring::npos ||
        m_Type.find( TEXT( "double" ) ) != std::wstring::npos );
}

//-----------------------------------------------------------------------------
void Function::ProcessArgumentInfo()
{
#ifdef _WIN32
    m_ArgInfo.clear();
    m_ArgInfo.reserve( m_Params.size() );

    int argIndex = IsMemberFunction() ? 1 : 0;

    for( FunctionParam & param : m_Params )
    {
        Argument arg;
        arg.m_Index = argIndex;
        arg.m_Reg = (CV_HREG_e)param.m_SymbolInfo.Register;
        arg.m_Offset = (DWORD)param.m_SymbolInfo.Address;
        arg.m_NumBytes = param.m_SymbolInfo.Size;
        m_ArgInfo.push_back( arg );

        ++argIndex;
    }
#endif
}

//-----------------------------------------------------------------------------
bool Function::IsMemberFunction()
{
    // TODO
    return false;
}

//-----------------------------------------------------------------------------
void Function::Print()
{
#ifdef _WIN32
    if( !m_Pdb )
    {
        return;
    }

    std::shared_ptr<OrbitDiaSymbol> diaSymbol = m_Pdb->GetDiaSymbolFromId( this->m_Id );
    if( diaSymbol->m_Symbol )
    {
        OrbitDia::DiaDump( diaSymbol->m_Symbol );

        if( m_ParentId )
        {
            Type & type = m_Pdb->GetTypeFromId( m_ParentId );
            type.LoadDiaInfo();
            type.GenerateDiaHierarchy();
        }

        DiaParser parser;
        parser.PrintFunctionType( diaSymbol->m_Symbol );
        ORBIT_VIZ( parser.m_Log );
    }

    LineInfo lineInfo;
    SymUtils::GetLineInfo( m_Address + (DWORD64)m_Pdb->GetHModule(), lineInfo );
    ORBIT_VIZV( lineInfo.m_File );
    ORBIT_VIZV( lineInfo.m_Line );
    ORBIT_VIZV( lineInfo.m_Address );
#endif

    ORBIT_VIZV( m_Address );
    ORBIT_VIZV( m_Selected );

    if( m_Params.size() )
    {
        ORBIT_VIZ( "\nParams:" );
        for( auto & var : m_Params )
        {
            ORBIT_VIZV( var.m_Name );
            ORBIT_VIZV( var.m_Address );
            ORBIT_VIZV( var.m_ParamType );
            ORBIT_VIZV( var.m_Type );
        }
    }
}
