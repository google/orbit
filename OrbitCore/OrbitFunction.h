//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include <string>
#include "TypeInfoStructs.h"
#include "OrbitDbgHelp.h"
#include "cvconst.h"
#include "BaseTypes.h"
#include "FunctionStats.h"
#include "SerializationMacros.h"

class Pdb;

//-----------------------------------------------------------------------------
struct FunctionParam
{
    FunctionParam(){ memset( this, 0, sizeof( FunctionParam ) ); }
    wstring     m_Name;
    wstring     m_ParamType;
    wstring     m_Type;
    wstring     m_Address;
    SYMBOL_INFO m_SymbolInfo;

    bool InRegister( int a_Index );
    bool IsPointer() { return m_Type.find( L"*" ) != wstring::npos; }
    bool IsRef() { return m_Type.find( L"&" ) != wstring::npos; }
    bool IsFloat();
};

//-----------------------------------------------------------------------------
struct Argument
{
    Argument() { memset( this, 0, sizeof( *this ) ); }
    DWORD      m_Index;
    CV_HREG_e  m_Reg;
    DWORD      m_Offset;
    DWORD      m_NumBytes;
};

//-----------------------------------------------------------------------------
struct FunctionArgInfo
{
    FunctionArgInfo() : m_NumStackBytes(0), m_ArgDataSize(0) {}
    int m_NumStackBytes;
    int m_ArgDataSize;
    std::vector< Argument > m_Args;
};

//-----------------------------------------------------------------------------
class Function
{
public:
    Function();
    ~Function();

    void Print();
    void SetAsMainFrameFunction();
    void AddParameter( const FunctionParam & a_Param ){ m_Params.push_back( a_Param ); }
    const std::wstring & PrettyName();
    inline const std::string & PrettyNameStr() { if( m_PrettyNameStr.size() == 0 ) m_PrettyNameStr = ws2s( m_PrettyName ); return m_PrettyNameStr; }
    inline const std::wstring & Lower() { if( m_PrettyNameLower.size() == 0 ) m_PrettyNameLower = ToLower( m_PrettyName ); return m_PrettyNameLower; }
    static const TCHAR* GetCallingConventionString( int a_CallConv );
    void ProcessArgumentInfo();
    bool IsMemberFunction();
    unsigned long long Hash() { if( m_NameHash == 0 ) { m_NameHash = StringHash( m_PrettyName ); } return m_NameHash; }
    bool Hookable();
    void Select(){ if( Hookable() ) m_Selected = true; }
    void PreHook();
    void UnSelect(){ m_Selected = false; }
    void ToggleSelect() { /*if( Hookable() )*/ m_Selected = !m_Selected; }
    bool IsSelected() const { return m_Selected; }
    DWORD64 GetVirtualAddress() const;
    bool IsOrbitFunc() { return m_OrbitType != OrbitType::NONE; }
    bool IsOrbitZone() { return m_OrbitType == ORBIT_TIMER_START || m_OrbitType == ORBIT_TIMER_STOP; }
    bool IsOrbitStart(){ return m_OrbitType == ORBIT_TIMER_START; }
    bool IsOrbitStop() { return m_OrbitType == ORBIT_TIMER_STOP; }
    bool IsRealloc()   { return m_OrbitType == REALLOC; }
    bool IsAlloc()     { return m_OrbitType == ALLOC; }
    bool IsFree()      { return m_OrbitType == FREE; }
    bool IsMemoryFunc(){ return IsFree() || IsAlloc() || IsRealloc(); }
    std::wstring GetModuleName();
    class Type* GetParentType();
    void ResetStats();
    void GetDisassembly();
    void FindFile();

    enum MemberID
    {
        NAME,
        ADDRESS,
        MODULE,
        FILE,
        LINE,
        SELECTED,
        INDEX,
        SIZE,
        CALL_CONV,
        NUM_EXPOSED_MEMBERS
    };

    enum OrbitType
    {
        NONE,
        ORBIT_TIMER_START,
        ORBIT_TIMER_STOP,
        ORBIT_LOG,
        ORBIT_OUTPUT_DEBUG_STRING,
        UNREAL_ACTOR,
        ALLOC,
        FREE,
        REALLOC,
        ORBIT_DATA,
        NUM_TYPES
    };

    ORBIT_SERIALIZABLE;

public:
    wstring  m_Name;
    wstring  m_PrettyName;
    string   m_PrettyNameStr;
    wstring  m_PrettyNameLower;
    DWORD64  m_Address;
    ULONG    m_Size;
    wstring  m_Module;
    wstring  m_File;
    int      m_Line;
    ULONG64  m_ModBase;
    int      m_CallConv;
    ULONG    m_Id;
    DWORD    m_ParentId;
    vector<FunctionParam> m_Params;
    vector<Argument>      m_ArgInfo;
    Pdb*                  m_Pdb;
    unsigned long long    m_NameHash;
    OrbitType             m_OrbitType;
    std::shared_ptr<FunctionStats> m_Stats;

protected:
    bool     m_Selected;
};
