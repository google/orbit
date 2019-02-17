//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "TcpClient.h"
#include "OrbitLib.h"
#include "ScopeTimer.h"
#include "TimerManager.h"
#include "Hijacking.h"
#include "Callstack.h"
#include "CrashHandler.h"

std::string GHost;
bool GIsCaptureEnabled = false;

//-----------------------------------------------------------------------------
UserScopeTimer::UserScopeTimer( const char* a_Name ) : m_Valid( false )
{
    // TODO: assert on size
    if( GIsCaptureEnabled )
    {
        m_Valid = true;
        new(m_Data)ScopeTimer( a_Name );
    }
}

//-----------------------------------------------------------------------------
UserScopeTimer::~UserScopeTimer()
{
    if( m_Valid )
    {
        ScopeTimer* Timer = (ScopeTimer*)m_Data;
        Timer->~ScopeTimer();
    }
}

//-----------------------------------------------------------------------------
void Orbit::Init( const std::string & a_Host )
{
    PRINT_FUNC;
    PRINT_VAR(a_Host);
    
    delete GTimerManager;
    GTimerManager = nullptr;
    
    GHost = a_Host;
    GTcpClient = std::make_unique<TcpClient>(a_Host);

    if( GTcpClient->IsValid() )
    {
        GTcpClient->Start();
        GTimerManager = new TimerManager( true );
    }
    else
    {
        GTcpClient = nullptr;
    }
}

//-----------------------------------------------------------------------------
void Orbit::InitRemote( const std::string & a_Host )
{
    Init( a_Host );
    
    CrashHandler::SendMiniDump();
}

//-----------------------------------------------------------------------------
HMODULE GetCurrentModule()
{
    HMODULE hModule = NULL;
    GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCTSTR)GetCurrentModule, &hModule);
    return hModule;
}

//-----------------------------------------------------------------------------
void Orbit::DeInit()
{
    if( GTimerManager )
    {
        GTimerManager->Stop();
    }

    delete GTimerManager;
    GTimerManager = nullptr;
    HMODULE module = GetCurrentModule();
    FreeLibraryAndExitThread(module, 0);
}

//-----------------------------------------------------------------------------
void Orbit::Start()
{
    if( GTimerManager )
    {
	    GTimerManager->StartClient();
        GIsCaptureEnabled = true;
    }
    else
    {
        PRINT("GTimerManager not created yet");
    }
}

//-----------------------------------------------------------------------------
void Orbit::Stop()
{
	GTimerManager->StopClient();
    GIsCaptureEnabled = false;
    Hijacking::DisableAllHooks();
}
