//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "Core.h"
#include "OrbitModule.h"
#include "Serialization.h"
#include "Pdb.h"
#include <string>

#ifndef WIN32
#include "LinuxUtils.h"
#include "Capture.h"
#include "ScopeTimer.h"
#include "OrbitUnreal.h"
#include "Params.h"
#include "OrbitProcess.h"
#include "Path.h"
#endif

//-----------------------------------------------------------------------------
Module::Module()
{
}

//-----------------------------------------------------------------------------
std::wstring Module::GetPrettyName()
{
    if( m_PrettyName.size() == 0 )
    {
        #ifdef WIN32
        m_PrettyName = Format( "%s [%I64x - %I64x] %s\r\n", m_Name.c_str(), m_AddressStart, m_AddressEnd, m_FullName.c_str() );
        m_AddressRange = Format( "[%I64x - %I64x]", m_AddressStart, m_AddressEnd );
        #else
        m_PrettyName = m_FullName;
        m_AddressRange = Format( "[%016llx - %016llx]", m_AddressStart, m_AddressEnd );
        m_PdbName = m_FullName;
        m_FoundPdb = true;
        #endif
    }

    return s2ws(m_PrettyName);
}

//-----------------------------------------------------------------------------
bool Module::IsDll() const
{
    return ToLower( Path::GetExtension( s2ws(m_FullName) ) ) == std::wstring( L".dll" ) ||
           Contains(m_Name, ".so" );
}

//-----------------------------------------------------------------------------
bool Module::LoadDebugInfo()
{
    std::wstring pdbName = s2ws(m_PdbName);
    m_Pdb = std::make_shared<Pdb>( pdbName.c_str() );
    m_Pdb->SetMainModule( (HMODULE)m_AddressStart );

    PRINT_VAR(m_FoundPdb);
    if( m_FoundPdb )
    {
        return m_Pdb->LoadDataFromPdb();
    }

    return false;
}

//-----------------------------------------------------------------------------
uint64_t Module::ValidateAddress( uint64_t a_Address )
{
    if( ContainsAddress(a_Address ) )
        return a_Address;

    // Treat input address as RVA
    uint64_t newAddress = m_AddressStart + a_Address;
    if( ContainsAddress(newAddress) )
        return newAddress;

    return 0xbadadd;
}

//-----------------------------------------------------------------------------
void Module::SetLoaded(bool a_Value)
{
    m_Loaded = a_Value;
}

//-----------------------------------------------------------------------------
ORBIT_SERIALIZE( Module, 0 )
{
    ORBIT_NVP_VAL( 0, m_Name );
    ORBIT_NVP_VAL( 0, m_FullName );
    ORBIT_NVP_VAL( 0, m_PdbName );
    ORBIT_NVP_VAL( 0, m_Directory );
    ORBIT_NVP_VAL( 0, m_PrettyName );
    ORBIT_NVP_VAL( 0, m_AddressRange );
    ORBIT_NVP_VAL( 0, m_DebugSignature );
    ORBIT_NVP_VAL( 0, m_AddressStart );
    ORBIT_NVP_VAL( 0, m_AddressEnd );
    ORBIT_NVP_VAL( 0, m_EntryPoint );
    ORBIT_NVP_VAL( 0, m_FoundPdb );
    ORBIT_NVP_VAL( 0, m_Selected );
    ORBIT_NVP_VAL( 0, m_Loaded );
    ORBIT_NVP_VAL( 0, m_PdbSize );
}

#ifndef WIN32

//-----------------------------------------------------------------------------
Function* Pdb::FunctionFromName( const std::string& a_Name )
{
    uint64_t hash = StringHash(a_Name);
    auto iter = m_StringFunctionMap.find(hash);
    return (iter == m_StringFunctionMap.end()) ? nullptr : iter->second;
}

//-----------------------------------------------------------------------------
bool Pdb::LoadPdb( const wchar_t* a_PdbName )
{
    m_FileName = a_PdbName;
    m_Name = Path::GetFileName(m_FileName);

    {
        SCOPE_TIMER_LOG(L"nm");
        // nm
        std::string nmCommand = std::string("nm ") + ws2s(a_PdbName) + std::string(" -n -l");
        std::string nmResult = LinuxUtils::ExecuteCommand(nmCommand.c_str());
        std::stringstream nmStream(nmResult);
        std::string line;
        while (std::getline(nmStream, line, '\n'))
        {
            std::vector<std::string> tokens = Tokenize(line, " \t");
            if (tokens.size() > 2) // nm
            {
                Function func;
                func.m_Name = tokens[2];
                func.m_Address = std::stoull(tokens[0], nullptr, 16);
                func.m_PrettyName = LinuxUtils::Demangle(tokens[2].c_str());
                func.m_Module = ws2s(Path::GetFileName(a_PdbName));
                func.m_Pdb = this;
                this->AddFunction(func);

                // Debug - Temporary
                if (Contains(func.m_PrettyName, "btCollisionDispatcher::needsCollision"))
                {
                    PRINT_VAR(func.m_PrettyName);
                }
            }
        }
    }
    ProcessData();

    {
        SCOPE_TIMER_LOG(L"bpftrace -l");
        // find functions that can receive bpftrace uprobes
        std::string bpfCommand = std::string("bpftrace -l 'uprobe: ") + ws2s(a_PdbName) + std::string("'");
        std::string bpfResult = LinuxUtils::ExecuteCommand(bpfCommand.c_str());
        std::stringstream bpfStream(bpfResult);
        std::string line;
        while (std::getline(bpfStream, line, '\n'))
        {
            std::vector<std::string> tokens = Tokenize(line);
            if (tokens.size() == 2) // bpftrace
            {
                Function func;
                auto probeTokens = Tokenize(tokens[1], ":");
                if (probeTokens.size() == 2)
                {
                    std::string mangled = probeTokens[1];
                    std::string demangled = LinuxUtils::Demangle(probeTokens[1].c_str());

                    // Debug - Temporary
                    if (Contains(demangled, "btCollisionDispatcher::needsCollision"))
                        PRINT_VAR(demangled);
                    Function* func = FunctionFromName(demangled);
                    if (func && func->m_Probe.empty())
                    {
                        std::string probe = Replace(tokens[1], ".", "*");
                        func->m_Probe = probe;
                    }
                }
            }
        }
    }

    return true;
}

//-----------------------------------------------------------------------------
void Pdb::LoadPdbAsync( const wchar_t* a_PdbName, std::function<void()> a_CompletionCallback )
{
    m_LoadingCompleteCallback = a_CompletionCallback;
    LoadPdb(a_PdbName);
    a_CompletionCallback();
}

//-----------------------------------------------------------------------------
void Pdb::ProcessData()
{
    SCOPE_TIMER_LOG( L"ProcessData" );
    ScopeLock lock( Capture::GTargetProcess->GetDataMutex() );

    auto & functions = Capture::GTargetProcess->GetFunctions();
    auto & globals   = Capture::GTargetProcess->GetGlobals();

    functions.reserve( functions.size() + m_Functions.size() );

    for( Function & func : m_Functions )
    {
        func.m_Pdb = this;
        functions.push_back( &func );
        GOrbitUnreal.OnFunctionAdded( &func );
    }

    if( GParams.m_FindFileAndLineInfo )
    {
        SCOPE_TIMER_LOG(L"Find File and Line info");
        for( Function & func : m_Functions )
        {
            func.FindFile();
        }
    }

    for( Type & type : m_Types )
    {
        type.m_Pdb = this;
        Capture::GTargetProcess->AddType( type );
        GOrbitUnreal.OnTypeAdded( &type );
    }

    for( Variable & var : m_Globals )
    {
        var.m_Pdb = this;
        globals.push_back( &var );
    }

    for( auto & it : m_TypeMap )
    {
        it.second.m_Pdb = this;
    }

    PopulateFunctionMap();
    PopulateStringFunctionMap();
}

//-----------------------------------------------------------------------------
void Pdb::PopulateFunctionMap()
{
    m_IsPopulatingFunctionMap = true;
    
    SCOPE_TIMER_LOG( L"Pdb::PopulateFunctionMap" );

    uint64_t baseAddress = (uint64_t)GetHModule();
    bool rvaAddresses = false;

    // It seems nm can return addresses as VA or RVA
    // TODO: investigate, use simple test to detect RVA
    for( Function & function : m_Functions )
    {
        if( function.m_Address > 0 && function.m_Address < baseAddress )
        {
            rvaAddresses = true;
            break;
        }
    }

    for( Function & Function : m_Functions )
    {
        uint64_t RVA = rvaAddresses ? Function.m_Address : Function.m_Address - (uint64_t)GetHModule();
        m_FunctionMap.insert( std::make_pair( RVA, &Function ) );
    }

    m_IsPopulatingFunctionMap = false;
}

//-----------------------------------------------------------------------------
void Pdb::PopulateStringFunctionMap()
{
    m_IsPopulatingFunctionStringMap = true;

    {
        //SCOPE_TIMER_LOG( L"Reserving map" );
        m_StringFunctionMap.reserve( unsigned ( 1.5f * (float)m_Functions.size() ) );
    }

    {
        //SCOPE_TIMER_LOG( L"Hash" );
        for( Function & Function : m_Functions )
        {
            Function.Hash();
        }
    }

    {
        //SCOPE_TIMER_LOG( L"Map inserts" );

        for( Function & Function : m_Functions )
        {
            m_StringFunctionMap[Function.m_NameHash] = &Function;
        }
    }

    m_IsPopulatingFunctionStringMap = false;
}

//-----------------------------------------------------------------------------
Function* Pdb::GetFunctionFromExactAddress( uint64_t a_Address )
{
    uint64_t RVA = a_Address - (uint64_t)GetHModule();
    auto iter = m_FunctionMap.find(RVA);
    return (iter != m_FunctionMap.end()) ? iter->second : nullptr;
}

//-----------------------------------------------------------------------------
Function* Pdb::GetFunctionFromProgramCounter( uint64_t a_Address )
{
    uint64_t RVA = a_Address - (uint64_t)GetHModule();

    auto it = m_FunctionMap.upper_bound( RVA );
    if (!m_FunctionMap.empty() && it != m_FunctionMap.begin())
    {
        --it;
        Function* func = it->second;
        return func;
    }

    return nullptr;
}

#endif

//-----------------------------------------------------------------------------
ORBIT_SERIALIZE(ModuleDebugInfo, 0)
{
    ORBIT_NVP_VAL(0, m_Pid);
    ORBIT_NVP_VAL(0, m_Name);
    ORBIT_NVP_VAL(0, m_Functions);
}