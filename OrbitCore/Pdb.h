//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include "OrbitDbgHelp.h"
#include "OrbitType.h"
#include "Variable.h"

#include <vector>
#include <functional>
#include <thread>
#include <atomic>

struct IDiaSymbol;
struct IDiaSession;
struct IDiaDataSource;

#ifdef _WIN32

class Pdb
{
public:
    Pdb( const wchar_t* a_PdbName );
    ~Pdb();

    void Init();
    
    virtual bool LoadPdb( const wchar_t* a_PdbName );
    virtual void LoadPdbAsync( const wchar_t* a_PdbName, std::function<void()> a_CompletionCallback );

    bool LoadDataFromPdb();
    bool LoadPdbDia();
    void Update();
    void AddFunction( Function & a_Function );
    void CheckOrbitFunction( Function & a_Function );
    void AddType( const Type & a_Type );
    void AddGlobal( const Variable & a_Global );
    void PrintFunction( Function & a_Func );
    void OnReceiveMessage( const Message & a_Msg );
    void AddArgumentRegister( const std::string & a_Reg, const std::string & a_Function );

    const std::wstring & GetName() const             { return m_Name; }
    const std::wstring & GetFileName() const         { return m_FileName; }
    std::vector<Function>&    GetFunctions()         { return m_Functions; }
    std::vector<Type>&        GetTypes()             { return m_Types; }
    std::vector<Variable>&    GetGlobals()           { return m_Globals; }
    HMODULE              GetHModule()                { return m_MainModule; }
    Type &               GetTypeFromId( ULONG a_Id ) { return m_TypeMap[a_Id]; }
    Type*                GetTypePtrFromId( ULONG a_ID );
    
    GUID                 GetGuid();

    
    void SetMainModule( HMODULE a_Module ){ m_MainModule = a_Module; }

    void Print() const;
    void PrintGlobals() const;
    void PopulateFunctionMap();
    void PopulateStringFunctionMap();
    void Clear();
    void Reserve();
    void ApplyPresets();

    Function* GetFunctionFromExactAddress( uint64_t a_Address );
    Function* GetFunctionFromProgramCounter( uint64_t a_Address );
    std::shared_ptr<OrbitDiaSymbol> SymbolFromAddress( uint64_t a_Address );
    bool LineInfoFromAddress( uint64_t a_Address, struct LineInfo & o_LineInfo );

    void SetLoadTime( float a_LoadTime ) { m_LastLoadTime = a_LoadTime; }
    float GetLoadTime() { return m_LastLoadTime; }

    std::wstring GetCachedName();
    std::wstring GetCachedKey();
    bool Load( const std::string & a_CachedPdb );
    void Save();

    bool IsLoading() const { return m_IsLoading; }

    template <class Archive>
    void serialize( Archive & ar, std::uint32_t const version )
    {
        /*ar( CEREAL_NVP(m_Name)
          , CEREAL_NVP(m_FileName)
          , CEREAL_NVP(m_FileNameW)
          , CEREAL_NVP(m_Functions)
          , CEREAL_NVP(m_Types)
          , CEREAL_NVP(m_Globals)
          , CEREAL_NVP(m_ModuleInfo)
          , CEREAL_NVP(m_TypeMap) );*/
    }

    std::shared_ptr<OrbitDiaSymbol> GetDiaSymbolFromId(ULONG a_Id);
    void ProcessData();
protected:
    void SendStatusToUi();

protected:
    // State
    std::unique_ptr<std::thread>        m_LoadingThread;
    std::atomic<bool>                   m_FinishedLoading;
    std::atomic<bool>                   m_IsLoading;
    std::atomic<bool>                   m_IsPopulatingFunctionMap;
    std::atomic<bool>                   m_IsPopulatingFunctionStringMap;
    std::function<void()>               m_LoadingCompleteCallback;
    HMODULE                             m_MainModule;
    float                               m_LastLoadTime;
    bool                                m_LoadedFromCache;
    std::vector< Variable >             m_WatchedVariables;
    std::set<std::string>               m_ArgumentRegisters;
    std::map<std::string, std::vector< std::string > >  m_RegFunctionsMap;

    // Data
    std::wstring                        m_Name;
    std::wstring                        m_FileName;
    std::vector<Function>               m_Functions;
    std::vector<Type>                   m_Types;
    std::vector<Variable>               m_Globals;
    IMAGEHLP_MODULE64                   m_ModuleInfo;
    std::unordered_map<ULONG, Type>     m_TypeMap;
    std::map<uint64_t, Function*>        m_FunctionMap;
    std::unordered_map<unsigned long long, Function*> m_StringFunctionMap;
    Timer*                              m_LoadTimer;
    
    // DIA
    IDiaSession*                        m_DiaSession;
    IDiaSymbol*                         m_DiaGlobalSymbol;
    IDiaDataSource*	                    m_DiaDataSource;
};

#else
class Pdb
{
public:
    Pdb( const wchar_t* a_PdbName ){}
    Pdb( const char* a_PdbName){}
    ~Pdb(){}

    void Init(){}

    virtual bool LoadPdb( const wchar_t* a_PdbName );
    virtual void LoadPdbAsync( const wchar_t* a_PdbName, std::function<void()> a_CompletionCallback );

    bool LoadDataFromPdb() { LoadPdb(m_Name.c_str()); }
    bool LoadPdbDia(){return false;}
    void Update(){}
    void AddFunction( Function & a_Function ){ m_Functions.push_back( a_Function ); }
    void CheckOrbitFunction( Function & a_Function ){}
    void AddType( const Type & a_Type ){}
    void AddGlobal( const Variable & a_Global ){}
    void PrintFunction( Function & a_Func ){}
    void OnReceiveMessage( const Message & a_Msg ){}
    void AddArgumentRegister( const std::string & a_Reg, const std::string & a_Function ){}

    const std::wstring & GetName() const             { return m_Name; }
    const std::wstring & GetFileName() const         { return m_FileName; }
    std::vector<Function>&    GetFunctions()         { return m_Functions; }
    std::vector<Type>&        GetTypes()             { return m_Types; }
    std::vector<Variable>&    GetGlobals()           { return m_Globals; }
    HMODULE              GetHModule()                { return m_MainModule; }
    Type &               GetTypeFromId( ULONG a_Id ) { return m_TypeMap[a_Id]; }
    Type*                GetTypePtrFromId( ULONG a_ID );

    GUID                 GetGuid();


    void SetMainModule( HMODULE a_Module ){ m_MainModule = a_Module; }

    void Print() const;
    void PrintGlobals() const;
    void PopulateFunctionMap();
    void PopulateStringFunctionMap();
    void Clear();
    void Reserve();
    void ApplyPresets();

    Function* GetFunctionFromExactAddress( uint64_t a_Address );
    Function* GetFunctionFromProgramCounter( uint64_t a_Address );
    IDiaSymbol* SymbolFromAddress( uint64_t a_Address );
    bool LineInfoFromAddress( uint64_t a_Address, struct LineInfo & o_LineInfo );
    Function* FunctionFromName( const std::string& a_Name );

    void SetLoadTime( float a_LoadTime ) { m_LastLoadTime = a_LoadTime; }
    float GetLoadTime() { return m_LastLoadTime; }

    std::wstring GetCachedName();
    std::wstring GetCachedKey();
    bool Load( const std::string & a_CachedPdb );
    void Save();

    bool IsLoading() const { return m_IsLoading; }

    template <class Archive>
    void serialize( Archive & ar, std::uint32_t const version )
    {
    }

    IDiaSymbol* GetDiaSymbolFromId( ULONG a_Id );
    void ProcessData();
protected:
    void SendStatusToUi();

protected:
    // State
    std::unique_ptr<std::thread>        m_LoadingThread;
    std::atomic<bool>                   m_FinishedLoading;
    std::atomic<bool>                   m_IsLoading;
    std::atomic<bool>                   m_IsPopulatingFunctionMap;
    std::atomic<bool>                   m_IsPopulatingFunctionStringMap;
    std::function<void()>               m_LoadingCompleteCallback;
    HMODULE                             m_MainModule = 0;
    float                               m_LastLoadTime = 0;
    std::vector< Variable >             m_WatchedVariables;
    std::set<std::string>               m_ArgumentRegisters;
    std::map<std::string, std::vector< std::string > >  m_RegFunctionsMap;

    // Data
    std::wstring                            m_Name;
    std::wstring                            m_FileName;
    std::vector<Function>                   m_Functions;
    std::vector<Type>                       m_Types;
    std::vector<Variable>                   m_Globals;
    std::unordered_map<ULONG, Type>         m_TypeMap;
    std::map<uint64_t, Function*>           m_FunctionMap;
    std::unordered_map<unsigned long long, Function*> m_StringFunctionMap;
    Timer*                                  m_LoadTimer = nullptr;
};
#endif

extern std::shared_ptr<Pdb> GPdbDbg;

