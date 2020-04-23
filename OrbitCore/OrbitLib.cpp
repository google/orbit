//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "OrbitLib.h"

#include "Callstack.h"
#include "CrashHandler.h"
#include "Hijacking.h"
#include "ScopeTimer.h"
#include "TcpClient.h"
#include "TimerManager.h"
#include "Utils.h"

std::string GHost;
bool GIsCaptureEnabled = false;

//-----------------------------------------------------------------------------
UserScopeTimer::UserScopeTimer(const char* a_Name) : m_Valid(false) {
  // TODO: assert on size
  if (GIsCaptureEnabled) {
    m_Valid = true;
    new (m_Data) ScopeTimer(a_Name);
  }
}

//-----------------------------------------------------------------------------
UserScopeTimer::~UserScopeTimer() {
  if (m_Valid) {
    ScopeTimer* Timer = (ScopeTimer*)m_Data;
    Timer->~ScopeTimer();
  }
}

//-----------------------------------------------------------------------------
void Orbit::Init(const std::string& a_Host) {
  PRINT_FUNC;
  PRINT_VAR(a_Host);

  GTimerManager = nullptr;
  GHost = a_Host;
  GTcpClient = std::make_unique<TcpClient>(a_Host);

  if (GTcpClient->IsValid()) {
    GTimerManager = std::make_unique<TimerManager>(true);
  } else {
    GTcpClient = nullptr;
  }
}

//-----------------------------------------------------------------------------
void Orbit::InitRemote(const std::string& a_Host) { Init(a_Host); }

//-----------------------------------------------------------------------------
HMODULE GetCurrentModule() {
#ifdef _WIN32
  HMODULE hModule = NULL;
  GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
                    (LPCTSTR)GetCurrentModule, &hModule);
  return hModule;
#else
  return nullptr;
#endif
}

//-----------------------------------------------------------------------------
void Orbit::DeInit() {
  if (GTimerManager) {
    GTimerManager->Stop();
    GTimerManager = nullptr;
  }

#ifdef _WIN32
  HMODULE module = GetCurrentModule();
  FreeLibraryAndExitThread(module, 0);
#endif
}

//-----------------------------------------------------------------------------
void Orbit::Start() {
  if (GTimerManager) {
    GTimerManager->StartClient();
    GIsCaptureEnabled = true;
  } else {
    LOG("GTimerManager not created yet");
  }
}

//-----------------------------------------------------------------------------
void Orbit::Stop() {
  GTimerManager->StopClient();
  GIsCaptureEnabled = false;

#ifdef _WIN32
  Hijacking::DisableAllHooks();
#endif
}
