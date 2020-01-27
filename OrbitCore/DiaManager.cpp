//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "DiaManager.h"

#include "ObjectCount.h"
#include "Platform.h"
#include "ScopeTimer.h"

#ifdef _WIN32
#include <Shellapi.h>

#include "PrintSymbol.h"
#include "dia2dump.h"
#endif

//-----------------------------------------------------------------------------
void OrbitDiaReleasePtr(IUnknown* a_Symbol) {
#ifdef _WIN32
  a_Symbol->Release();
#endif
}

//-----------------------------------------------------------------------------
void OrbitSmartPtrCreated() {
  // PRINT_VAR( GObjectCounter.Inc("DiaPtr") );
}

//-----------------------------------------------------------------------------
void OrbitSmartPtrDestroyed() {
  // PRINT_VAR( GObjectCounter.Dec("DiaPtr") );
}

//-----------------------------------------------------------------------------
typedef HRESULT(WINAPI* pfnGetFactory)(REFCLSID, REFIID, void**);

//-----------------------------------------------------------------------------
DiaManager::DiaManager() {}

//-----------------------------------------------------------------------------
DiaManager::~DiaManager() {}

//-----------------------------------------------------------------------------
bool InitDataSource(IDiaDataSource** a_DiaDataSource) {
  SCOPE_TIMER_LOG(L"InitDataSource");

  HRESULT hr = CoInitialize(NULL);
  std::wstring dllFullPath = Path::GetExecutablePath() + L"msdia140.dll";

  // Load library once
  static HMODULE msDiaModule;
  if (msDiaModule == nullptr) {
    LoadLibrary(dllFullPath.c_str());
  }

  hr = CoCreateInstance(__uuidof(DiaSource), NULL, CLSCTX_INPROC_SERVER,
                        __uuidof(IDiaDataSource), (void**)a_DiaDataSource);
  if (!SUCCEEDED(hr)) {
    ORBIT_VIZ("CoCreateInstance Failed : ");
    ORBIT_VIZ(dllFullPath);
    ORBIT_VIZ("\n");
    std::wstring msg = Format(L"HRESULT = %08X\n", hr);
    ORBIT_VIZ(msg);

    return false;
  }

  return true;
}

//-----------------------------------------------------------------------------
bool DiaManager::LoadDataFromPdb(const wchar_t* a_FileName,
                                 IDiaDataSource** a_DiaDataSource,
                                 IDiaSession** a_Session,
                                 IDiaSymbol** a_GlobalSymbol) {
  if (!*a_DiaDataSource) {
    InitDataSource(a_DiaDataSource);
  }

  if (*a_DiaDataSource) {
    wchar_t wszExt[MAX_PATH];
    _wsplitpath_s(a_FileName, NULL, 0, NULL, 0, NULL, 0, wszExt, MAX_PATH);
    HRESULT hr = CoInitialize(NULL);

    if (!_wcsicmp(wszExt, L".pdb")) {
      // Open and prepare a program database (.pdb) file as a debug data source
      hr = (*a_DiaDataSource)->loadDataFromPdb(a_FileName);

      if (FAILED(hr)) {
        std::wstring msg =
            Format(L"loadDataFromPdb failed - HRESULT = %08X\n", hr);
        ORBIT_VIZ(msg);
        return false;
      }
    } else {
      // CCallback callback; // Receives callbacks from the DIA symbol locating
      // procedure,
      //                    // thus enabling a user interface to report on the
      //                    progress of
      //                    // the location attempt. The client application may
      //                    optionally
      //                    // provide a reference to its own implementation of
      //                    this
      //                    // virtual base class to the
      //                    IDiaDataSource::loadDataForExe method.
      // callback.AddRef();

      //// Open and prepare the debug data associated with the executable

      // hr = ( *ppSource )->loadDataForExe( a_FileName, wszSearchPath,
      // &callback );

      // if( FAILED( hr ) ) {
      //    PRINTF( L"loadDataForExe failed - HRESULT = %08X\n", hr );

      //    return false;
      //}
    }

    // Open a session for querying symbols

    hr = (*a_DiaDataSource)->openSession(a_Session);

    if (FAILED(hr)) {
      std::wstring msg = Format(L"openSession failed - HRESULT = %08X\n", hr);
      ORBIT_VIZ(msg);
      return false;
    }

    // Retrieve a reference to the global scope
    hr = (*a_Session)->get_globalScope(a_GlobalSymbol);

    if (hr != S_OK) {
      ORBIT_VIZ(L"get_globalScope failed\n");
      return false;
    }

    // Set Machine type for getting correct register names

    DWORD dwMachType = 0;
    if ((*a_GlobalSymbol)->get_machineType(&dwMachType) == S_OK) {
      switch (dwMachType) {
        case IMAGE_FILE_MACHINE_I386:
          g_dwMachineType = CV_CFL_80386;
          break;
        case IMAGE_FILE_MACHINE_IA64:
          g_dwMachineType = CV_CFL_IA64;
          break;
        case IMAGE_FILE_MACHINE_AMD64:
          g_dwMachineType = CV_CFL_AMD64;
          break;
      }
    }
  }

  return true;
}

void DiaManager::InitMsDiaDll() {
  std::wstring dllFullPath = Path::GetExecutablePath() + L"msdia140.dll";
  std::wstring args = L"/C regsvr32 /s " + dllFullPath;
  ShellExecute(0, L"open", L"cmd.exe", args.c_str(), 0, SW_HIDE);
}
