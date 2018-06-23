//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include "BaseTypes.h"
#include "SerializationMacros.h"
#include "Threading.h"
#include "DiaManager.h"

#include <set>
#include <unordered_set>
#include <memory>
#include <map>

class Function;
class Type;
class Variable;
class Thread;
class Session;
struct Module;
struct IDiaSymbol;

//-----------------------------------------------------------------------------
class Process
{
public:
    Process();
    Process( DWORD a_ID );
    ~Process();
    
    void Init();
    void LoadDebugInfo();
    void ListModules();
    void EnumerateThreads();
    void UpdateCpuTime();
    void UpdateThreadUsage();
    void SortThreadsByUsage();
    void SortThreadsById();
    bool IsElevated() const { return m_IsElevated; }
    bool HasThread( DWORD a_ThreadId ) const { return m_ThreadIds.find( a_ThreadId ) != m_ThreadIds.end(); }
    void AddThreadId( DWORD a_ThreadId ) { m_ThreadIds.insert(a_ThreadId); }
    void RemoveThreadId( DWORD a_ThreadId ) { m_ThreadIds.erase(a_ThreadId); };
    void AddModule( std::shared_ptr<Module> & a_Module );
    void FindPdbs( const std::vector< std::wstring > & a_SearchLocations );

    static bool IsElevated( HANDLE a_Process );
    static bool SetPrivilege( LPCTSTR a_Name, bool a_Enable );

    std::map< DWORD64, std::shared_ptr<Module> >& GetModules() { return m_Modules; }
    std::map< std::wstring, std::shared_ptr<Module> >& GetNameToModulesMap() { return m_NameToModuleMap; }
    std::shared_ptr<Module> FindModule( const std::wstring & a_ModuleName );

    const std::wstring & GetName() const { return m_Name; }
    const std::wstring & GetFullName() const { return m_FullName; }
    DWORD GetID() const { return m_ID; }
    double GetCpuUsage() const { return m_CpuUsage; }
    HANDLE GetHandle() const { return m_Handle; }
    bool GetIs64Bit() const { return m_Is64Bit; }
    int NumModules() const { return (int)m_Modules.size(); }
    bool GetIsRemote() const { return m_IsRemote; }
    void SetIsRemote( bool val ) { m_IsRemote = val; }

    Function* GetFunctionFromAddress( DWORD64 a_Address, bool a_IsExact = true );
    std::shared_ptr<Module> GetModuleFromAddress( DWORD64 a_Address );
    std::shared_ptr<OrbitDiaSymbol> SymbolFromAddress( DWORD64 a_Address );
    bool LineInfoFromAddress( DWORD64 a_Address, struct LineInfo & o_LineInfo );

    void LoadSession(const Session & a_Session);
    void SaveSession();

    std::vector<Function*>& GetFunctions() { return m_Functions; }
    std::vector<Type*>&     GetTypes()     { return m_Types; }
    std::vector<Variable*>& GetGlobals()   { return m_Globals; }
    std::vector<std::shared_ptr<Thread> >& GetThreads(){ return m_Threads; }

    void AddWatchedVariable( std::shared_ptr<Variable> a_Variable ) { m_WatchedVariables.push_back( a_Variable ); }
    const std::vector< std::shared_ptr<Variable> > & GetWatchedVariables(){ return m_WatchedVariables; }
    void RefreshWatchedVariables();
    void ClearWatchedVariables();

    void AddType( Type & a_Type );
    void SetID( DWORD a_ID );

    Mutex & GetDataMutex() { return m_DataMutex; }

    DWORD64 GetOutputDebugStringAddress();
    DWORD64 GetRaiseExceptionAddress();

    void FindCoreFunctions();

    std::wstring     m_Name;
    std::wstring     m_FullName;

    ORBIT_SERIALIZABLE;

protected:
    void ClearTransients();

private:
    DWORD       m_ID;
    HANDLE      m_Handle;
    bool        m_IsElevated;

    FILETIME    m_LastUserTime;
    FILETIME    m_LastKernTime;
    double      m_CpuUsage;
    Timer       m_UpdateCpuTimer;
    bool        m_Is64Bit;
    bool        m_DebugInfoLoaded;
    bool        m_IsRemote;
    Mutex       m_DataMutex;

    std::map< DWORD64, std::shared_ptr<Module> > m_Modules;
    std::map< std::wstring, std::shared_ptr<Module> > m_NameToModuleMap;
    std::vector<std::shared_ptr<Thread> >   m_Threads;
    std::unordered_set<DWORD>               m_ThreadIds;

    // Transients
    std::vector< Function* >    m_Functions;
    std::vector< Type* >        m_Types;
    std::vector< Variable* >    m_Globals;
    std::vector< std::shared_ptr<Variable> > m_WatchedVariables;
    
    std::unordered_set< unsigned long long > m_UniqueTypeHash;
};

