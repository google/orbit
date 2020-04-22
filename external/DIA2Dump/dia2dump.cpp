// Dia2Dump.cpp : Defines the entry point for the console application.
//
// This is a part of the Debug Interface Access SDK
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
// This source code is only intended as a supplement to the
// Debug Interface Access SDK and related electronic documentation
// provided with the library.
// See these sources for detailed information regarding the
// Debug Interface Access SDK API.
//

#include "dia2dump.h"

#include "../../../OrbitCore/OrbitDia.h"
#include "../../../OrbitCore/ScopeTimer.h"
#include "../../../OrbitGl/App.h"
#include "Callback.h"
#include "PrintSymbol.h"

#pragma warning(disable : 4100)

const wchar_t* g_szFilename;
IDiaDataSource* g_pDiaDataSource;
IDiaSession* g_pDiaSession;
IDiaSymbol* g_pGlobalSymbol;
DWORD g_dwMachineType = CV_CFL_80386;
DWORD g_NumFunctions = 0;
DWORD g_NumUserTypes = 0;

////////////////////////////////////////////////////////////
//
int wmain(int argc, wchar_t* argv[]) {
  FILE* pFile;

  if (argc < 2) {
    PrintHelpOptions();
    return -1;
  }

  if (_wfopen_s(&pFile, argv[argc - 1], L"r") || !pFile) {
    // invalid file name or file does not exist

    PrintHelpOptions();
    return -1;
  }

  fclose(pFile);

  g_szFilename = argv[argc - 1];

  // CoCreate() and initialize COM objects

  if (!LoadDataFromPdb(g_szFilename, &g_pDiaDataSource, &g_pDiaSession,
                       &g_pGlobalSymbol)) {
    return -1;
  }

  if (argc == 2) {
    // no options passed; print all pdb info

    DumpAllPdbInfo(g_pDiaSession, g_pGlobalSymbol);
  }

  else if (!_wcsicmp(argv[1], L"-all")) {
    DumpAllPdbInfo(g_pDiaSession, g_pGlobalSymbol);
  }

  else if (!ParseArg(argc - 2, &argv[1])) {
    CleanupDia();

    return -1;
  }

  // release COM objects and CoUninitialize()

  CleanupDia();

  system("pause");

  return 0;
}

typedef HRESULT(WINAPI* pfnGetFactory)(REFCLSID, REFIID, void**);

////////////////////////////////////////////////////////////
// Create an IDiaData source and open a PDB file
//
bool LoadDataFromPdb(const wchar_t* szFilename, IDiaDataSource** ppSource,
                     IDiaSession** ppSession, IDiaSymbol** ppGlobal) {
  wchar_t wszExt[MAX_PATH];
  wchar_t* wszSearchPath =
      L"SRV**\\\\symbols\\symbols";  // Alternate path to search for debug data
  DWORD dwMachType = 0;

  HRESULT hr = CoInitialize(NULL);

  // Obtain access to the provider

  hr = CoCreateInstance(__uuidof(DiaSource), NULL, CLSCTX_INPROC_SERVER,
                        __uuidof(IDiaDataSource), (void**)ppSource);

  if (FAILED(hr)) {
    ORBIT_LOG(
        absl::StrFormat("CoCreateInstance failed - HRESULT = %08X\n", hr));

    // try loading dll by hand
    HMODULE msDiaModule = LoadLibrary(L"msdia140.dll");

    if (msDiaModule == NULL) return false;

    pfnGetFactory procAddr =
        (pfnGetFactory)GetProcAddress(msDiaModule, "DllGetClassObject");
    if (procAddr == NULL) return false;

    IClassFactory* pFactory = NULL;
    hr = procAddr(__uuidof(DiaSource), IID_IClassFactory, (void**)&pFactory);
    if (SUCCEEDED(hr)) {
      hr = pFactory->CreateInstance(NULL, __uuidof(IDiaDataSource),
                                    (void**)ppSource);
      pFactory->Release();
    } else {
      return false;
    }
  }

  _wsplitpath_s(szFilename, NULL, 0, NULL, 0, NULL, 0, wszExt, MAX_PATH);

  if (!_wcsicmp(wszExt, L".pdb")) {
    // Open and prepare a program database (.pdb) file as a debug data source

    hr = (*ppSource)->loadDataFromPdb(szFilename);

    if (FAILED(hr)) {
      PRINTF(L"loadDataFromPdb failed - HRESULT = %08X\n", hr);

      return false;
    }
  }

  else {
    CCallback callback;  // Receives callbacks from the DIA symbol locating
                         // procedure, thus enabling a user interface to report
                         // on the progress of the location attempt. The client
                         // application may optionally provide a reference to
                         // its own implementation of this virtual base class to
                         // the IDiaDataSource::loadDataForExe method.
    callback.AddRef();

    // Open and prepare the debug data associated with the executable

    hr = (*ppSource)->loadDataForExe(szFilename, wszSearchPath, &callback);

    if (FAILED(hr)) {
      PRINTF(L"loadDataForExe failed - HRESULT = %08X\n", hr);

      return false;
    }
  }

  // Open a session for querying symbols

  hr = (*ppSource)->openSession(ppSession);

  if (FAILED(hr)) {
    PRINTF(L"openSession failed - HRESULT = %08X\n", hr);

    return false;
  }

  // Retrieve a reference to the global scope

  hr = (*ppSession)->get_globalScope(ppGlobal);

  if (hr != S_OK) {
    PRINTF(L"get_globalScope failed\n");

    return false;
  }

  // Set Machine type for getting correct register names

  if ((*ppGlobal)->get_machineType(&dwMachType) == S_OK) {
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

  return true;
}

////////////////////////////////////////////////////////////
// Release DIA objects and CoUninitialize
//
void CleanupDia() {
  if (g_pGlobalSymbol) {
    g_pGlobalSymbol->Release();
    g_pGlobalSymbol = NULL;
  }

  if (g_pDiaSession) {
    g_pDiaSession->Release();
    g_pDiaSession = NULL;
  }

  if (g_pDiaDataSource) {
    g_pDiaDataSource->Release();
    g_pDiaDataSource = NULL;
  }

  CoUninitialize();
}

////////////////////////////////////////////////////////////
// Parse the arguments of the program
//
bool ParseArg(int argc, wchar_t* argv[]) {
  int iCount = 0;
  bool bReturn = true;

  if (!argc) {
    return true;
  }

  if (!_wcsicmp(argv[0], L"-?")) {
    PrintHelpOptions();

    return true;
  }

  else if (!_wcsicmp(argv[0], L"-help")) {
    PrintHelpOptions();

    return true;
  }

  else if (!_wcsicmp(argv[0], L"-m")) {
    // -m                : print all the mods

    iCount = 1;
    bReturn = bReturn && DumpAllMods(g_pGlobalSymbol);
    argc -= iCount;
    bReturn = bReturn && ParseArg(argc, &argv[iCount]);
  }

  else if (!_wcsicmp(argv[0], L"-p")) {
    // -p                : print all the publics

    iCount = 1;
    bReturn = bReturn && DumpAllPublics(g_pGlobalSymbol);
    argc -= iCount;
    bReturn = bReturn && ParseArg(argc, &argv[iCount]);
  }

  else if (!_wcsicmp(argv[0], L"-s")) {
    // -s                : print symbols

    iCount = 1;
    bReturn = bReturn && DumpAllSymbols(g_pGlobalSymbol);
    argc -= iCount;
    bReturn = bReturn && ParseArg(argc, &argv[iCount]);
  }

  else if (!_wcsicmp(argv[0], L"-g")) {
    // -g                : print all the globals

    iCount = 1;
    bReturn = bReturn && DumpAllGlobals(g_pGlobalSymbol);
    argc -= iCount;
    bReturn = bReturn && ParseArg(argc, &argv[iCount]);
  }

  else if (!_wcsicmp(argv[0], L"-t")) {
    // -t                : print all the types

    iCount = 1;
    bReturn = bReturn && DumpAllTypes(g_pGlobalSymbol);
    argc -= iCount;
    bReturn = bReturn && ParseArg(argc, &argv[iCount]);
  }

  else if (!_wcsicmp(argv[0], L"-f")) {
    // -f                : print all the files

    iCount = 1;
    bReturn = bReturn && DumpAllFiles(g_pDiaSession, g_pGlobalSymbol);
    argc -= iCount;
    bReturn = bReturn && ParseArg(argc, &argv[iCount]);
  }

  else if (!_wcsicmp(argv[0], L"-l")) {
    if (argc > 1 && *argv[1] != L'-') {
      // -l RVA [bytes]    : print line number info at RVA address in the bytes
      // range

      DWORD dwRVA = 0;
      DWORD dwRange = MAX_RVA_LINES_BYTES_RANGE;

      swscanf_s(argv[1], L"%x", &dwRVA);
      if (argc > 2 && *argv[2] != L'-') {
        swscanf_s(argv[2], L"%d", &dwRange);
        iCount = 3;
      }

      else {
        iCount = 2;
      }

      bReturn = bReturn && DumpAllLines(g_pDiaSession, dwRVA, dwRange);
    }

    else {
      // -l            : print line number info

      bReturn = bReturn && DumpAllLines(g_pDiaSession, g_pGlobalSymbol);
      iCount = 1;
    }

    argc -= iCount;
    bReturn = bReturn && ParseArg(argc, &argv[iCount]);
  }

  else if (!_wcsicmp(argv[0], L"-c")) {
    // -c                : print section contribution info

    iCount = 1;
    bReturn = bReturn && DumpAllSecContribs(g_pDiaSession);
    argc -= iCount;
    bReturn = bReturn && ParseArg(argc, &argv[iCount]);
  }

  else if (!_wcsicmp(argv[0], L"-dbg")) {
    // -dbg              : dump debug streams

    iCount = 1;
    bReturn = bReturn && DumpAllDebugStreams(g_pDiaSession);
    argc -= iCount;
    bReturn = bReturn && ParseArg(argc, &argv[iCount]);
  }

  else if (!_wcsicmp(argv[0], L"-injsrc")) {
    if (argc > 1 && *argv[1] != L'-') {
      // -injsrc filename          : dump injected source filename

      bReturn = bReturn && DumpInjectedSource(g_pDiaSession, argv[1]);
      iCount = 2;
    }

    else {
      // -injsrc           : dump all injected source

      bReturn = bReturn && DumpAllInjectedSources(g_pDiaSession);
      iCount = 1;
    }

    argc -= iCount;
    bReturn = bReturn && ParseArg(argc, &argv[iCount]);
  }

  else if (!_wcsicmp(argv[0], L"-sf")) {
    // -sf               : dump all source files

    iCount = 1;
    bReturn = bReturn && DumpAllSourceFiles(g_pDiaSession, g_pGlobalSymbol);
    argc -= iCount;
    bReturn = bReturn && ParseArg(argc, &argv[iCount]);
  }

  else if (!_wcsicmp(argv[0], L"-oem")) {
    // -oem              : dump all OEM specific types

    iCount = 1;
    bReturn = bReturn && DumpAllOEMs(g_pGlobalSymbol);
    argc -= iCount;
    bReturn = bReturn && ParseArg(argc, &argv[iCount]);
  }

  else if (!_wcsicmp(argv[0], L"-fpo")) {
    if (argc > 1 && *argv[1] != L'-') {
      DWORD dwRVA = 0;

      if (iswdigit(*argv[1])) {
        // -fpo [RVA]        : dump frame pointer omission information for a
        // function address

        swscanf_s(argv[1], L"%x", &dwRVA);
        bReturn = bReturn && DumpFPO(g_pDiaSession, dwRVA);
      }

      else {
        // -fpo [symbolname] : dump frame pointer omission information for a
        // function symbol

        bReturn = bReturn && DumpFPO(g_pDiaSession, g_pGlobalSymbol, argv[1]);
      }

      iCount = 2;
    }

    else {
      bReturn = bReturn && DumpAllFPO(g_pDiaSession);
      iCount = 1;
    }

    argc -= iCount;
    bReturn = bReturn && ParseArg(argc, &argv[iCount]);
  }

  else if (!_wcsicmp(argv[0], L"-compiland")) {
    if ((argc > 1) && (*argv[1] != L'-')) {
      // -compiland [name] : dump symbols for this compiland

      bReturn = bReturn && DumpCompiland(g_pGlobalSymbol, argv[1]);
      argc -= 2;
    }

    else {
      PRINTF(L"ERROR - ParseArg(): missing argument for option '-line'");

      return false;
    }

    argc -= iCount;
    bReturn = bReturn && ParseArg(argc, &argv[iCount]);
  }

  else if (!_wcsicmp(argv[0], L"-lines")) {
    if ((argc > 1) && (*argv[1] != L'-')) {
      DWORD dwRVA = 0;

      if (iswdigit(*argv[1])) {
        // -lines <RVA>                  : dump line numbers for this address\n"

        swscanf_s(argv[1], L"%x", &dwRVA);
        bReturn = bReturn && DumpLines(g_pDiaSession, dwRVA);
      }

      else {
        // -lines <symbolname>           : dump line numbers for this function

        bReturn = bReturn && DumpLines(g_pDiaSession, g_pGlobalSymbol, argv[1]);
      }

      iCount = 2;
    }

    else {
      PRINTF(L"ERROR - ParseArg(): missing argument for option '-compiland'");

      return false;
    }

    argc -= iCount;
    bReturn = bReturn && ParseArg(argc, &argv[iCount]);
  }

  else if (!_wcsicmp(argv[0], L"-type")) {
    // -type <symbolname>: dump this type in detail

    if ((argc > 1) && (*argv[1] != L'-')) {
      bReturn = bReturn && DumpType(g_pGlobalSymbol, argv[1]);
      iCount = 2;
    }

    else {
      PRINTF(L"ERROR - ParseArg(): missing argument for option '-type'");

      return false;
    }

    argc -= iCount;
    bReturn = bReturn && ParseArg(argc, &argv[iCount]);
  }

  else if (!_wcsicmp(argv[0], L"-label")) {
    // -label <RVA>      : dump label at RVA
    if ((argc > 1) && (*argv[1] != L'-')) {
      DWORD dwRVA = 0;

      swscanf_s(argv[1], L"%x", &dwRVA);
      bReturn = bReturn && DumpLabel(g_pDiaSession, dwRVA);
      iCount = 2;
    }

    else {
      PRINTF(L"ERROR - ParseArg(): missing argument for option '-label'");

      return false;
    }

    argc -= iCount;
    bReturn = bReturn && ParseArg(argc, &argv[iCount]);
  }

  else if (!_wcsicmp(argv[0], L"-sym")) {
    if ((argc > 1) && (*argv[1] != L'-')) {
      DWORD dwRVA = 0;
      const wchar_t* szChildname = NULL;

      iCount = 2;

      if (argc > 2 && *argv[2] != L'-') {
        szChildname = argv[2];
        iCount = 3;
      }

      if (iswdigit(*argv[1])) {
        // -sym <RVA> [childname]        : dump child information of symbol at
        // this address

        swscanf_s(argv[1], L"%x", &dwRVA);
        bReturn =
            bReturn && DumpSymbolWithRVA(g_pDiaSession, dwRVA, szChildname);
      }

      else {
        // -sym <symbolname> [childname] : dump child information of this symbol

        bReturn = bReturn &&
                  DumpSymbolsWithRegEx(g_pGlobalSymbol, argv[1], szChildname);
      }
    }

    else {
      PRINTF(L"ERROR - ParseArg(): missing argument for option '-sym'");

      return false;
    }

    argc -= iCount;
    bReturn = bReturn && ParseArg(argc, &argv[iCount]);
  }

  else if (!_wcsicmp(argv[0], L"-lsrc")) {
    // -lsrc  <file> [line]          : dump line numbers for this source file

    if ((argc > 1) && (*argv[1] != L'-')) {
      DWORD dwLine = 0;

      iCount = 2;
      if (argc > 2 && *argv[2] != L'-') {
        swscanf_s(argv[1], L"%d", &dwLine);
        iCount = 3;
      }

      bReturn =
          bReturn && DumpLinesForSourceFile(g_pDiaSession, argv[1], dwLine);
    }

    else {
      PRINTF(L"ERROR - ParseArg(): missing argument for option '-lsrc'");

      return false;
    }

    argc -= iCount;
    bReturn = bReturn && ParseArg(argc, &argv[iCount]);
  }

  else if (!_wcsicmp(argv[0], L"-ps")) {
    // -ps <RVA> [-n <number>]       : dump symbols after this address, default
    // 16

    if ((argc > 1) && (*argv[1] != L'-')) {
      DWORD dwRVA = 0;
      DWORD dwRange;

      swscanf_s(argv[1], L"%x", &dwRVA);
      if (argc > 3 && !_wcsicmp(argv[2], L"-n")) {
        swscanf_s(argv[3], L"%d", &dwRange);
        iCount = 4;
      }

      else {
        dwRange = 16;
        iCount = 2;
      }

      bReturn = bReturn &&
                DumpPublicSymbolsSorted(g_pDiaSession, dwRVA, dwRange, true);
    }

    else {
      PRINTF(L"ERROR - ParseArg(): missing argument for option '-ps'");

      return false;
    }

    argc -= iCount;
    bReturn = bReturn && ParseArg(argc, &argv[iCount]);
  }

  else if (!_wcsicmp(argv[0], L"-psr")) {
    // -psr <RVA> [-n <number>]       : dump symbols before this address,
    // default 16

    if ((argc > 1) && (*argv[1] != L'-')) {
      DWORD dwRVA = 0;
      DWORD dwRange;

      swscanf_s(argv[1], L"%x", &dwRVA);

      if (argc > 3 && !_wcsicmp(argv[2], L"-n")) {
        swscanf_s(argv[3], L"%d", &dwRange);
        iCount = 4;
      }

      else {
        dwRange = 16;
        iCount = 2;
      }

      bReturn = bReturn &&
                DumpPublicSymbolsSorted(g_pDiaSession, dwRVA, dwRange, false);
    }

    else {
      PRINTF(L"ERROR - ParseArg(): missing argument for option '-psr'");

      return false;
    }

    argc -= iCount;
    bReturn = bReturn && ParseArg(argc, &argv[iCount]);
  }

  else if (!_wcsicmp(argv[0], L"-annotations")) {
    // -annotations <RVA>: dump annotation symbol for this RVA

    if ((argc > 1) && (*argv[1] != L'-')) {
      DWORD dwRVA = 0;

      swscanf_s(argv[1], L"%x", &dwRVA);
      bReturn = bReturn && DumpAnnotations(g_pDiaSession, dwRVA);
      iCount = 2;
    }

    else {
      PRINTF(L"ERROR - ParseArg(): missing argument for option '-maptosrc'");

      return false;
    }

    argc -= iCount;
    bReturn = bReturn && ParseArg(argc, &argv[iCount]);
  }

  else if (!_wcsicmp(argv[0], L"-maptosrc")) {
    // -maptosrc <RVA>   : dump src RVA for this image RVA

    if ((argc > 1) && (*argv[1] != L'-')) {
      DWORD dwRVA = 0;

      swscanf_s(argv[1], L"%x", &dwRVA);
      bReturn = bReturn && DumpMapToSrc(g_pDiaSession, dwRVA);
      iCount = 2;
    }

    else {
      PRINTF(L"ERROR - ParseArg(): missing argument for option '-maptosrc'");

      return false;
    }

    argc -= iCount;
    bReturn = bReturn && ParseArg(argc, &argv[iCount]);
  }

  else if (!_wcsicmp(argv[0], L"-mapfromsrc")) {
    // -mapfromsrc <RVA> : dump image RVA for src RVA

    if ((argc > 1) && (*argv[1] != L'-')) {
      DWORD dwRVA = 0;

      swscanf_s(argv[1], L"%x", &dwRVA);
      bReturn = bReturn && DumpMapFromSrc(g_pDiaSession, dwRVA);
      iCount = 2;
    }

    else {
      PRINTF(L"ERROR - ParseArg(): missing argument for option '-mapfromsrc'");

      return false;
    }

    argc -= iCount;
    bReturn = bReturn && ParseArg(argc, &argv[iCount]);
  }

  else {
    PRINTF(L"ERROR - unknown option %s\n", argv[0]);

    PrintHelpOptions();

    return false;
  }

  return bReturn;
}

////////////////////////////////////////////////////////////
// Display the usage
//
void PrintHelpOptions() {
  static const wchar_t* const helpString =
      L"usage: Dia2Dump.exe [ options ] <filename>\n"
      L"  -?                : print this help\n"
      L"  -all              : print all the debug info\n"
      L"  -m                : print all the mods\n"
      L"  -p                : print all the publics\n"
      L"  -g                : print all the globals\n"
      L"  -t                : print all the types\n"
      L"  -f                : print all the files\n"
      L"  -s                : print symbols\n"
      L"  -l [RVA [bytes]]  : print line number info at RVA address in the "
      L"bytes range\n"
      L"  -c                : print section contribution info\n"
      L"  -dbg              : dump debug streams\n"
      L"  -injsrc [file]    : dump injected source\n"
      L"  -sf               : dump all source files\n"
      L"  -oem              : dump all OEM specific types\n"
      L"  -fpo [RVA]        : dump frame pointer omission information for a "
      L"func addr\n"
      L"  -fpo [symbolname] : dump frame pointer omission information for a "
      L"func symbol\n"
      L"  -compiland [name] : dump symbols for this compiland\n"
      L"  -lines <funcname> : dump line numbers for this function\n"
      L"  -lines <RVA>      : dump line numbers for this address\n"
      L"  -type <symbolname>: dump this type in detail\n"
      L"  -label <RVA>      : dump label at RVA\n"
      L"  -sym <symbolname> [childname] : dump child information of this "
      L"symbol\n"
      L"  -sym <RVA> [childname]        : dump child information of symbol at "
      L"this addr\n"
      L"  -lsrc  <file> [line]          : dump line numbers for this source "
      L"file\n"
      L"  -ps <RVA> [-n <number>]       : dump symbols after this address, "
      L"default 16\n"
      L"  -psr <RVA> [-n <number>]      : dump symbols before this address, "
      L"default 16\n"
      L"  -annotations <RVA>: dump annotation symbol for this RVA\n"
      L"  -maptosrc <RVA>   : dump src RVA for this image RVA\n"
      L"  -mapfromsrc <RVA> : dump image RVA for src RVA\n";

  PRINTF(helpString);
}

////////////////////////////////////////////////////////////
// Dump all the data stored in a PDB
//
void DumpAllPdbInfo(IDiaSession* pSession, IDiaSymbol* pGlobal) {
  DumpAllMods(pGlobal);
  DumpAllPublics(pGlobal);
  DumpAllSymbols(pGlobal);
  DumpAllGlobals(pGlobal);
  DumpAllTypes(pGlobal);
  DumpAllFiles(pSession, pGlobal);
  DumpAllLines(pSession, pGlobal);
  DumpAllSecContribs(pSession);
  DumpAllDebugStreams(pSession);
  DumpAllInjectedSources(pSession);
  DumpAllFPO(pSession);
  DumpAllOEMs(pGlobal);
}

////////////////////////////////////////////////////////////
// Dump all the modules information
//
bool DumpAllMods(IDiaSymbol* pGlobal) {
  PRINTF(L"\n\n*** MODULES\n\n");

  // Retrieve all the compiland symbols

  OrbitDiaEnumSymbols pEnumSymbols;

  if (FAILED(pGlobal->findChildren(SymTagCompiland, NULL, nsNone,
                                   &pEnumSymbols.m_Symbol))) {
    return false;
  }

  OrbitDiaSymbol pCompiland;
  ULONG celt = 0;
  ULONG iMod = 1;

  while (SUCCEEDED(pEnumSymbols->Next(1, &pCompiland.m_Symbol, &celt)) &&
         (celt == 1)) {
    BSTR bstrName;
    if (pCompiland->get_name(&bstrName) != S_OK) {
      PRINTF(L"ERROR - Failed to get the compiland's name\n");
      return false;
    }

    PRINTF(L"%04X %s\n", iMod++, bstrName);
    SysFreeString(bstrName);
  }

  putwchar(L'\n');

  return true;
}

////////////////////////////////////////////////////////////
// Dump all the public symbols - SymTagPublicSymbol
//
bool DumpAllPublics(IDiaSymbol* pGlobal) {
  PRINTF(L"\n\n*** PUBLICS\n\n");
  OrbitDiaEnumSymbols pEnumSymbols;
  if (FAILED(pGlobal->findChildren(SymTagPublicSymbol, NULL, nsNone,
                                   &pEnumSymbols.m_Symbol))) {
    return false;
  }

  OrbitDiaSymbol pSymbol;
  ULONG celt = 0;

  while (SUCCEEDED(pEnumSymbols->Next(1, &pSymbol.m_Symbol, &celt)) &&
         (celt == 1)) {
    PrintPublicSymbol(pSymbol.m_Symbol);
    pSymbol.Release();
  }

  putwchar(L'\n');
  return true;
}

//-----------------------------------------------------------------------------
bool DumpAllFunctions(IDiaSymbol* pGlobal) {
  SCOPE_TIMER_LOG("DumpFunctions");
  OrbitDiaEnumSymbols pEnumSymbols;
  HRESULT res = pGlobal->findChildren(SymTagFunction, NULL, nsNone,
                                      &pEnumSymbols.m_Symbol);
  if (FAILED(res)) {
    PrintLastError();
    return false;
  }

  OrbitDiaSymbol pSymbol;
  ULONG celt = 0;
  while (SUCCEEDED(pEnumSymbols->Next(1, &pSymbol.m_Symbol, &celt)) &&
         (celt == 1)) {
    ++g_NumFunctions;
    std::shared_ptr<Function> Func = std::make_shared<Function>();
    DWORD dwRVA;
    DWORD callingConv;
    BSTR bstrName;

    if (pSymbol->get_relativeVirtualAddress(&dwRVA) == S_OK) {
      Func->SetAddress(dwRVA);
    }

    if (pSymbol->get_name(&bstrName) == S_OK) {
      Func->SetPrettyName(ws2s(bstrName));
      SysFreeString(bstrName);
    }

    DWORD indexID;
    if (pSymbol->get_symIndexId(&indexID) == S_OK) {
      Func->SetId(indexID);
    }

    // get_lengthProlog
    ULONGLONG length;
    if (pSymbol->get_length(&length) == S_OK) {
      Func->SetSize((ULONG)length);
    }

    OrbitDiaSymbol pFuncType;
    if (pSymbol->get_type(&pFuncType.m_Symbol) == S_OK) {
      if (pFuncType->get_callingConvention(&callingConv) == S_OK) {
        Func->SetCallingConvention(callingConv);
      }
    }

    DWORD classParentId = 0;
    OrbitDiaSymbol classParentSym;
    if (pSymbol->get_classParent(&classParentSym.m_Symbol) == S_OK) {
      if (classParentSym.m_Symbol->get_symIndexId(&classParentId) == S_OK) {
        Func->SetParentId(classParentId);
      }
    }

    BSTR bstrFile;
    if (pSymbol->get_sourceFileName(&bstrFile) == S_OK) {
      Func->SetFile(ws2s(bstrFile));
      SysFreeString(bstrName);
    }

    const std::string& pretty_name = Func->PrettyName();
    if (pretty_name[0] != '`') {
      GPdbDbg->AddFunction(Func);
    }

    pSymbol.Release();
  }

  return true;
}

//-----------------------------------------------------------------------------
bool DumpTypes(IDiaSymbol* pGlobal) {
  SCOPE_TIMER_LOG("DumpTypes");
  OrbitDiaEnumSymbols pEnumSymbols;

  if (FAILED(pGlobal->findChildren(SymTagUDT, NULL, nsNone,
                                   &pEnumSymbols.m_Symbol))) {
    PRINTF(L"ERROR - DumpAllUDTs() returned no symbols\n");
    return false;
  }

  OrbitDiaSymbol pSymbol;
  ULONG celt = 0;

  while (SUCCEEDED(pEnumSymbols->Next(1, &pSymbol.m_Symbol, &celt)) &&
         (celt == 1)) {
    Type orbitType;

    // PrintTypeInDetail(pSymbol, 0);
    // PrintUDT(pSymbol);

    BSTR bstrName;
    if (pSymbol->get_name(&bstrName) != S_OK) {
      orbitType.m_Name = "???";
    } else {
      orbitType.m_Name = ws2s(bstrName);
      SysFreeString(bstrName);
    }

    DWORD indexID;
    if (pSymbol->get_symIndexId(&indexID) == S_OK) {
      orbitType.m_Id = indexID;
    }

    DWORD unmodifiedID;
    if (pSymbol->get_unmodifiedTypeId(&unmodifiedID) == S_OK) {
      orbitType.m_UnmodifiedId = unmodifiedID;
    }

    ULONGLONG ulLen;
    if (pSymbol->get_length(&ulLen) == S_OK) {
      orbitType.m_Length = ulLen;
    }

    GPdbDbg->AddType(orbitType);
    ++g_NumUserTypes;
    pSymbol.Release();
  }

  return true;
}

////////////////////////////////////////////////////////////
// Dump all the symbol information stored in the compilands
//
bool DumpAllSymbols(IDiaSymbol* pGlobal) {
  PRINTF(L"\n\n*** SYMBOLS\n\n\n");

  // Retrieve the compilands first

  OrbitDiaEnumSymbols pEnumSymbols;

  if (FAILED(pGlobal->findChildren(SymTagCompiland, NULL, nsNone,
                                   &pEnumSymbols.m_Symbol))) {
    return false;
  }

  OrbitDiaSymbol pCompiland;
  ULONG celt = 0;

  while (SUCCEEDED(pEnumSymbols->Next(1, &pCompiland.m_Symbol, &celt)) &&
         (celt == 1)) {
    PRINTF(L"\n** Module: ");
    BSTR bstrName;

    if (pCompiland->get_name(&bstrName) != S_OK) {
      PRINTF(L"(???)\n\n");
    } else {
      PRINTF(L"%s\n\n", bstrName);
      SysFreeString(bstrName);
    }

    // Find all the symbols defined in this compiland and print their info
    OrbitDiaEnumSymbols pEnumChildren;

    if (SUCCEEDED(pCompiland->findChildren(SymTagNull, NULL, nsNone,
                                           &pEnumChildren.m_Symbol))) {
      OrbitDiaSymbol pSymbol;
      ULONG celtChildren = 0;

      while (
          SUCCEEDED(pEnumChildren->Next(1, &pSymbol.m_Symbol, &celtChildren)) &&
          (celtChildren == 1)) {
        PrintSymbol(pSymbol.m_Symbol, 0);
        pSymbol.Release();
      }
    }

    pCompiland.Release();
  }

  putwchar(L'\n');
  return true;
}

////////////////////////////////////////////////////////////
// Dump all the global symbols - SymTagFunction,
//  SymTagThunk and SymTagData
//
bool DumpAllGlobals(IDiaSymbol* pGlobal) {
  OrbitDiaEnumSymbols pEnumSymbols;
  OrbitDiaSymbol pSymbol;
  enum SymTagEnum dwSymTags[] = {SymTagFunction, SymTagThunk, SymTagData};
  ULONG celt = 0;

  PRINTF(L"\n\n*** GLOBALS\n\n");

  for (size_t i = 0; i < _countof(dwSymTags); i++) {
    if (SUCCEEDED(pGlobal->findChildren(dwSymTags[i], NULL, nsNone,
                                        &pEnumSymbols.m_Symbol))) {
      while (SUCCEEDED(pEnumSymbols->Next(1, &pSymbol.m_Symbol, &celt)) &&
             (celt == 1)) {
        PrintGlobalSymbol(pSymbol.m_Symbol);
        pSymbol.Release();
      }
    }

    else {
      PRINTF(L"ERROR - DumpAllGlobals() returned no symbols\n");

      return false;
    }
  }

  PRINTF(L"\n");

  return true;
}

bool OrbitDumpAllGlobals(IDiaSymbol* pGlobal) {
  SCOPE_TIMER_LOG("DumpAllGlobals");
  OrbitDiaEnumSymbols pEnumSymbols;
  OrbitDiaSymbol pSymbol;
  ULONG celt = 0;

  if (SUCCEEDED(pGlobal->findChildren(SymTagData, NULL, nsNone,
                                      &pEnumSymbols.m_Symbol))) {
    while (SUCCEEDED(pEnumSymbols->Next(1, &pSymbol.m_Symbol, &celt)) &&
           (celt == 1)) {
      OrbitAddGlobalSymbol(pSymbol.m_Symbol);
      pSymbol.Release();
    }

    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////
// Dump all the types information
//  (type symbols can be UDTs, enums or typedefs)
//
bool DumpAllTypes(IDiaSymbol* pGlobal) {
  PRINTF(L"\n\n*** TYPES\n");

  bool f1 = DumpAllUDTs(pGlobal);
  bool f2 = DumpAllEnums(pGlobal);
  bool f3 = DumpAllTypedefs(pGlobal);

  return f1 && f2 && f3;
}

////////////////////////////////////////////////////////////
// Dump all the user defined types (UDT)
//
bool DumpAllUDTs(IDiaSymbol* pGlobal) {
  PRINTF(L"\n\n** User Defined Types\n\n");

  OrbitDiaEnumSymbols pEnumSymbols;

  if (FAILED(pGlobal->findChildren(SymTagUDT, NULL, nsNone,
                                   &pEnumSymbols.m_Symbol))) {
    PRINTF(L"ERROR - DumpAllUDTs() returned no symbols\n");

    return false;
  }

  OrbitDiaSymbol pSymbol;
  ULONG celt = 0;

  while (SUCCEEDED(pEnumSymbols->Next(1, &pSymbol.m_Symbol, &celt)) &&
         (celt == 1)) {
    PrintTypeInDetail(pSymbol.m_Symbol, 0);
    pSymbol.Release();
  }

  putwchar(L'\n');
  return true;
}

////////////////////////////////////////////////////////////
// Dump all the enum types from the pdb
//
bool DumpAllEnums(IDiaSymbol* pGlobal) {
  PRINTF(L"\n\n** ENUMS\n\n");

  OrbitDiaEnumSymbols pEnumSymbols;

  if (FAILED(pGlobal->findChildren(SymTagEnum, NULL, nsNone,
                                   &pEnumSymbols.m_Symbol))) {
    PRINTF(L"ERROR - DumpAllEnums() returned no symbols\n");
    return false;
  }

  OrbitDiaSymbol pSymbol;
  ULONG celt = 0;

  while (SUCCEEDED(pEnumSymbols->Next(1, &pSymbol.m_Symbol, &celt)) &&
         (celt == 1)) {
    PrintTypeInDetail(pSymbol.m_Symbol, 0);
    pSymbol.Release();
  }

  putwchar(L'\n');
  return true;
}

////////////////////////////////////////////////////////////
// Dump all the typedef types from the pdb
//
bool DumpAllTypedefs(IDiaSymbol* pGlobal) {
  PRINTF(L"\n\n** TYPEDEFS\n\n");

  OrbitDiaEnumSymbols pEnumSymbols;

  if (FAILED(pGlobal->findChildren(SymTagTypedef, NULL, nsNone,
                                   &pEnumSymbols.m_Symbol))) {
    PRINTF(L"ERROR - DumpAllTypedefs() returned no symbols\n");
    return false;
  }

  OrbitDiaSymbol pSymbol;
  ULONG celt = 0;

  while (SUCCEEDED(pEnumSymbols->Next(1, &pSymbol.m_Symbol, &celt)) &&
         (celt == 1)) {
    PrintTypeInDetail(pSymbol.m_Symbol, 0);
    pSymbol.Release();
  }

  putwchar(L'\n');
  return true;
}

////////////////////////////////////////////////////////////
// Dump OEM specific types
//
bool DumpAllOEMs(IDiaSymbol* pGlobal) {
  PRINTF(L"\n\n*** OEM Specific types\n\n");

  OrbitDiaEnumSymbols pEnumSymbols;

  if (FAILED(pGlobal->findChildren(SymTagCustomType, NULL, nsNone,
                                   &pEnumSymbols.m_Symbol))) {
    PRINTF(L"ERROR - DumpAllOEMs() returned no symbols\n");
    return false;
  }

  OrbitDiaSymbol pSymbol;
  ULONG celt = 0;

  while (SUCCEEDED(pEnumSymbols->Next(1, &pSymbol.m_Symbol, &celt)) &&
         (celt == 1)) {
    PrintTypeInDetail(pSymbol.m_Symbol, 0);
    pSymbol.Release();
  }

  putwchar(L'\n');
  return true;
}

////////////////////////////////////////////////////////////
// For each compiland in the PDB dump all the source files
//
bool DumpAllFiles(IDiaSession* pSession, IDiaSymbol* pGlobal) {
  PRINTF(L"\n\n*** FILES\n\n");

  // In order to find the source files, we have to look at the image's
  // compilands/modules

  OrbitDiaEnumSymbols pEnumSymbols;

  if (FAILED(pGlobal->findChildren(SymTagCompiland, NULL, nsNone,
                                   &pEnumSymbols.m_Symbol))) {
    return false;
  }

  OrbitDiaSymbol pCompiland;
  ULONG celt = 0;

  while (SUCCEEDED(pEnumSymbols->Next(1, &pCompiland.m_Symbol, &celt)) &&
         (celt == 1)) {
    BSTR bstrName;
    if (pCompiland->get_name(&bstrName) == S_OK) {
      PRINTF(L"\nCompiland = %s\n\n", bstrName);
      SysFreeString(bstrName);
    }

    // Every compiland could contain multiple references to the source files
    // which were used to build it Retrieve all source files by compiland by
    // passing NULL for the name of the source file

    OrbitDiaEnumSourceFiles pEnumSourceFiles;

    if (SUCCEEDED(pSession->findFile(pCompiland.m_Symbol, NULL, nsNone,
                                     &pEnumSourceFiles.m_Symbol))) {
      OrbitDiaSourceFile pSourceFile;

      while (
          SUCCEEDED(pEnumSourceFiles->Next(1, &pSourceFile.m_Symbol, &celt)) &&
          (celt == 1)) {
        PrintSourceFile(pSourceFile.m_Symbol);
        putwchar(L'\n');

        pSourceFile.Release();
      }

      pCompiland.Release();
    }

    putwchar(L'\n');
  }

  return true;
}

////////////////////////////////////////////////////////////
// Dump all the line numbering information contained in the PDB
//  Only function symbols have corresponding line numbering information
bool DumpAllLines(IDiaSession* pSession, IDiaSymbol* pGlobal) {
  PRINTF(L"\n\n*** LINES\n\n");

  OrbitDiaEnumSymbols pEnumSymbols;
  if (FAILED(pGlobal->findChildren(SymTagCompiland, NULL, nsNone,
                                   &pEnumSymbols.m_Symbol))) {
    return false;
  }

  OrbitDiaSymbol pCompiland;
  ULONG celt = 0;
  while (SUCCEEDED(pEnumSymbols->Next(1, &pCompiland.m_Symbol, &celt)) &&
         (celt == 1)) {
    OrbitDiaEnumSymbols pEnumFunction;
    if (SUCCEEDED(pCompiland->findChildren(SymTagFunction, NULL, nsNone,
                                           &pEnumFunction.m_Symbol))) {
      OrbitDiaSymbol pFunction;
      while (SUCCEEDED(pEnumFunction->Next(1, &pFunction.m_Symbol, &celt)) &&
             (celt == 1)) {
        PrintLines(pSession, pFunction.m_Symbol);
        pFunction.Release();
      }
    }

    pCompiland.Release();
  }

  putwchar(L'\n');
  return true;
}

////////////////////////////////////////////////////////////
// Dump all the line numbering information for a given RVA
// and a given range
//
bool DumpAllLines(IDiaSession* pSession, DWORD dwRVA, DWORD dwRange) {
  // Retrieve and print the lines that corresponds to a specified RVA

  IDiaEnumLineNumbers* pLines;

  if (FAILED(pSession->findLinesByRVA(dwRVA, dwRange, &pLines))) {
    return false;
  }

  PrintLines(pLines);
  pLines->Release();
  putwchar(L'\n');

  return true;
}

////////////////////////////////////////////////////////////
// Dump all the section contributions from the PDB
//
//  Section contributions are stored in a table which will
//  be retrieved via IDiaSession->getEnumTables through
//  QueryInterface()using the REFIID of the IDiaEnumSectionContribs
//
bool DumpAllSecContribs(IDiaSession* pSession) {
  PRINTF(L"\n\n*** SECTION CONTRIBUTION\n\n");

  IDiaEnumSectionContribs* pEnumSecContribs;

  if (FAILED(GetTable(pSession, __uuidof(IDiaEnumSectionContribs),
                      (void**)&pEnumSecContribs))) {
    return false;
  }

  PRINTF(L"    RVA        Address       Size    Module\n");

  IDiaSectionContrib* pSecContrib;
  ULONG celt = 0;

  while (SUCCEEDED(pEnumSecContribs->Next(1, &pSecContrib, &celt)) &&
         (celt == 1)) {
    PrintSecContribs(pSecContrib);

    pSecContrib->Release();
  }

  pEnumSecContribs->Release();

  putwchar(L'\n');

  return true;
}

////////////////////////////////////////////////////////////
// Dump all debug data streams contained in the PDB
//
bool DumpAllDebugStreams(IDiaSession* pSession) {
  IDiaEnumDebugStreams* pEnumStreams;

  PRINTF(L"\n\n*** DEBUG STREAMS\n\n");
  if (FAILED(pSession->getEnumDebugStreams(&pEnumStreams))) {
    return false;
  }

  IDiaEnumDebugStreamData* pStream;
  ULONG celt = 0;

  for (; SUCCEEDED(pEnumStreams->Next(1, &pStream, &celt)) && (celt == 1);
       pStream = NULL) {
    PrintStreamData(pStream);
    pStream->Release();
  }

  pEnumStreams->Release();
  putwchar(L'\n');
  return true;
}

////////////////////////////////////////////////////////////
// Dump all the injected source from the PDB
//
//  Injected sources data is stored in a table which will
//  be retrieved via IDiaSession->getEnumTables through
//  QueryInterface()using the REFIID of the IDiaEnumSectionContribs
//
bool DumpAllInjectedSources(IDiaSession* pSession) {
  PRINTF(L"\n\n*** INJECTED SOURCES TABLE\n\n");

  IDiaEnumInjectedSources* pEnumInjSources = NULL;

  if (SUCCEEDED(GetTable(pSession, __uuidof(IDiaEnumInjectedSources),
                         (void**)&pEnumInjSources))) {
    return false;
  }

  IDiaInjectedSource* pInjSource;
  ULONG celt = 0;

  while (SUCCEEDED(pEnumInjSources->Next(1, &pInjSource, &celt)) &&
         (celt == 1)) {
    PrintGeneric(pInjSource);

    pInjSource->Release();
  }

  pEnumInjSources->Release();

  putwchar(L'\n');

  return true;
}

////////////////////////////////////////////////////////////
// Dump info corresponing to a given injected source filename
//
bool DumpInjectedSource(IDiaSession* pSession, const wchar_t* szName) {
  // Retrieve a source that has been placed into the symbol store by attribute
  // providers or
  //  other components of the compilation process

  IDiaEnumInjectedSources* pEnumInjSources;

  if (FAILED(pSession->findInjectedSource(szName, &pEnumInjSources))) {
    PRINTF(L"ERROR - DumpInjectedSources() could not find %s\n", szName);

    return false;
  }

  IDiaInjectedSource* pInjSource;
  ULONG celt = 0;

  while (SUCCEEDED(pEnumInjSources->Next(1, &pInjSource, &celt)) &&
         (celt == 1)) {
    PrintGeneric(pInjSource);

    pInjSource->Release();
  }

  pEnumInjSources->Release();

  return true;
}

////////////////////////////////////////////////////////////
// Dump all the source file information stored in the PDB
// We have to go through every compiland in order to retrieve
//   all the information otherwise checksums for instance
//   will not be available
// Compilands can have multiple source files with the same
//   name but different content which produces diffrent
//   checksums
//
bool DumpAllSourceFiles(IDiaSession* pSession, IDiaSymbol* pGlobal) {
  PRINTF(L"\n\n*** SOURCE FILES\n\n");

  // To get the complete source file info we must go through the compiland first
  // by passing NULL instead all the source file names only will be retrieved

  OrbitDiaEnumSymbols pEnumSymbols;

  if (FAILED(pGlobal->findChildren(SymTagCompiland, NULL, nsNone,
                                   &pEnumSymbols.m_Symbol))) {
    return false;
  }

  OrbitDiaSymbol pCompiland;
  ULONG celt = 0;

  while (SUCCEEDED(pEnumSymbols->Next(1, &pCompiland.m_Symbol, &celt)) &&
         (celt == 1)) {
    BSTR bstrName;

    if (pCompiland->get_name(&bstrName) == S_OK) {
      PRINTF(L"\nCompiland = %s\n\n", bstrName);

      SysFreeString(bstrName);
    }

    // Every compiland could contain multiple references to the source files
    // which were used to build it Retrieve all source files by compiland by
    // passing NULL for the name of the source file

    OrbitDiaEnumSourceFiles pEnumSourceFiles;

    if (SUCCEEDED(pSession->findFile(pCompiland.m_Symbol, NULL, nsNone,
                                     &pEnumSourceFiles.m_Symbol))) {
      OrbitDiaSourceFile pSourceFile;
      while (
          SUCCEEDED(pEnumSourceFiles->Next(1, &pSourceFile.m_Symbol, &celt)) &&
          (celt == 1)) {
        PrintSourceFile(pSourceFile.m_Symbol);
        putwchar(L'\n');
        pSourceFile.Release();
      }
    }

    putwchar(L'\n');

    pCompiland.Release();
  }

  return true;
}

////////////////////////////////////////////////////////////
// Dump all the FPO info
//
//  FPO data stored in a table which will be retrieved via
//    IDiaSession->getEnumTables through QueryInterface()
//    using the REFIID of the IDiaEnumFrameData
//
bool DumpAllFPO(IDiaSession* pSession) {
  IDiaEnumFrameData* pEnumFrameData;

  PRINTF(L"\n\n*** FPO\n\n");

  if (FAILED(GetTable(pSession, __uuidof(IDiaEnumFrameData),
                      (void**)&pEnumFrameData))) {
    return false;
  }

  IDiaFrameData* pFrameData;
  ULONG celt = 0;

  while (SUCCEEDED(pEnumFrameData->Next(1, &pFrameData, &celt)) &&
         (celt == 1)) {
    PrintFrameData(pFrameData);

    pFrameData->Release();
  }

  pEnumFrameData->Release();

  putwchar(L'\n');

  return true;
}

////////////////////////////////////////////////////////////
// Dump FPO info for a function at the specified RVA
//
bool DumpFPO(IDiaSession* pSession, DWORD dwRVA) {
  IDiaEnumFrameData* pEnumFrameData;

  // Retrieve first the table holding all the FPO info

  if ((dwRVA != 0) && SUCCEEDED(GetTable(pSession, __uuidof(IDiaEnumFrameData),
                                         (void**)&pEnumFrameData))) {
    IDiaFrameData* pFrameData;

    // Retrieve the frame data corresponding to the given RVA

    if (SUCCEEDED(pEnumFrameData->frameByRVA(dwRVA, &pFrameData))) {
      PrintGeneric(pFrameData);

      pFrameData->Release();
    }

    else {
      // Some function might not have FPO data available (see ASM funcs like
      // strcpy)

      PRINTF(L"ERROR - DumpFPO() frameByRVA invalid RVA: 0x%08X\n", dwRVA);

      pEnumFrameData->Release();

      return false;
    }

    pEnumFrameData->Release();
  }

  else {
    PRINTF(L"ERROR - DumpFPO() GetTable\n");

    return false;
  }

  putwchar(L'\n');

  return true;
}

////////////////////////////////////////////////////////////
// Dump FPO info for a specified function symbol using its
//  name (a regular expression string is used for the search)
//
bool DumpFPO(IDiaSession* pSession, IDiaSymbol* pGlobal,
             const wchar_t* szSymbolName) {
  OrbitDiaEnumSymbols pEnumSymbols;
  OrbitDiaSymbol pSymbol;
  ULONG celt = 0;
  DWORD dwRVA;

  // Find first all the function symbols that their names matches the search
  // criteria

  if (FAILED(pGlobal->findChildren(SymTagFunction, szSymbolName,
                                   nsRegularExpression,
                                   &pEnumSymbols.m_Symbol))) {
    PRINTF(L"ERROR - DumpFPO() findChildren could not find symol %s\n",
           szSymbolName);
    return false;
  }

  while (SUCCEEDED(pEnumSymbols->Next(1, &pSymbol.m_Symbol, &celt)) &&
         (celt == 1)) {
    if (pSymbol->get_relativeVirtualAddress(&dwRVA) == S_OK) {
      PrintPublicSymbol(pSymbol.m_Symbol);
      DumpFPO(pSession, dwRVA);
    }

    pSymbol.Release();
  }
  putwchar(L'\n');
  return true;
}

////////////////////////////////////////////////////////////
// Dump a specified compiland and all the symbols defined in it
//
bool DumpCompiland(IDiaSymbol* pGlobal, const wchar_t* szCompName) {
  IDiaEnumSymbols* pEnumSymbols;

  if (FAILED(pGlobal->findChildren(SymTagCompiland, szCompName,
                                   nsCaseInsensitive, &pEnumSymbols))) {
    return false;
  }

  OrbitDiaSymbol pCompiland;
  ULONG celt = 0;

  while (SUCCEEDED(pEnumSymbols->Next(1, &pCompiland.m_Symbol, &celt)) &&
         (celt == 1)) {
    PRINTF(L"\n** Module: ");
    BSTR bstrName;
    if (pCompiland->get_name(&bstrName) != S_OK) {
      PRINTF(L"(???)\n\n");
    } else {
      PRINTF(L"%s\n\n", bstrName);
      SysFreeString(bstrName);
    }

    OrbitDiaEnumSymbols pEnumChildren;

    if (SUCCEEDED(pCompiland->findChildren(SymTagNull, NULL, nsNone,
                                           &pEnumChildren.m_Symbol))) {
      OrbitDiaSymbol pSymbol;
      ULONG celt_ = 0;
      while (SUCCEEDED(pEnumChildren->Next(1, &pSymbol.m_Symbol, &celt_)) &&
             (celt_ == 1)) {
        PrintSymbol(pSymbol.m_Symbol, 0);
        pSymbol.Release();
      }
    }

    pCompiland.Release();
  }

  return true;
}

////////////////////////////////////////////////////////////
// Dump the line numbering information for a specified RVA
//
bool DumpLines(IDiaSession* pSession, DWORD dwRVA) {
  IDiaEnumLineNumbers* pLines;

  if (FAILED(pSession->findLinesByRVA(dwRVA, MAX_RVA_LINES_BYTES_RANGE,
                                      &pLines))) {
    return false;
  }

  PrintLines(pLines);

  pLines->Release();

  return true;
}

////////////////////////////////////////////////////////////
// Dump the all line numbering information for a specified
//  function symbol name (as a regular expression string)
//
bool DumpLines(IDiaSession* pSession, IDiaSymbol* pGlobal,
               const wchar_t* szFuncName) {
  OrbitDiaEnumSymbols pEnumSymbols;

  if (FAILED(pGlobal->findChildren(SymTagFunction, szFuncName,
                                   nsRegularExpression,
                                   &pEnumSymbols.m_Symbol))) {
    return false;
  }

  OrbitDiaSymbol pFunction;
  ULONG celt = 0;

  while (SUCCEEDED(pEnumSymbols->Next(1, &pFunction.m_Symbol, &celt)) &&
         (celt == 1)) {
    PrintLines(pSession, pFunction.m_Symbol);
    pFunction.Release();
  }

  return true;
}

////////////////////////////////////////////////////////////
// Dump the symbol information corresponding to a specified RVA
//
bool DumpSymbolWithRVA(IDiaSession* pSession, DWORD dwRVA,
                       const wchar_t* szChildname) {
  OrbitDiaSymbol pSymbol;
  LONG lDisplacement;

  if (FAILED(pSession->findSymbolByRVAEx(dwRVA, SymTagNull, &pSymbol.m_Symbol,
                                         &lDisplacement))) {
    return false;
  }

  PRINTF(L"Displacement = 0x%X\n", lDisplacement);

  PrintGeneric(pSymbol);

  DumpSymbolWithChildren(pSymbol.m_Symbol, szChildname);

  while (pSymbol.m_Symbol != NULL) {
    IDiaSymbol* pParent;

    if ((pSymbol->get_lexicalParent(&pParent) == S_OK) && pParent) {
      PRINTF(L"\nParent\n");

      PrintSymbol(pParent, 0);

      pSymbol.Release();
      pSymbol.m_Symbol = pParent;
    } else {
      pSymbol.Release();
      break;
    }
  }

  return true;
}

////////////////////////////////////////////////////////////
// Dump the symbols information where their names matches a
//  specified regular expression string
//
bool DumpSymbolsWithRegEx(IDiaSymbol* pGlobal, const wchar_t* szRegEx,
                          const wchar_t* szChildname) {
  OrbitDiaEnumSymbols pEnumSymbols;
  if (FAILED(pGlobal->findChildren(SymTagNull, szRegEx, nsRegularExpression,
                                   &pEnumSymbols.m_Symbol))) {
    return false;
  }

  bool bReturn = true;

  OrbitDiaSymbol pSymbol;
  ULONG celt = 0;

  while (SUCCEEDED(pEnumSymbols->Next(1, &pSymbol.m_Symbol, &celt)) &&
         (celt == 1)) {
    PrintGeneric(pSymbol.m_Symbol);
    bReturn = DumpSymbolWithChildren(pSymbol.m_Symbol, szChildname);
    pSymbol.Release();
  }

  return bReturn;
}

////////////////////////////////////////////////////////////
// Dump the information corresponding to a symbol name which
//  is a children of the specified parent symbol
//
bool DumpSymbolWithChildren(IDiaSymbol* pSymbol, const wchar_t* szChildname) {
  if (szChildname != NULL) {
    OrbitDiaEnumSymbols pEnumSyms;

    if (FAILED(pSymbol->findChildren(SymTagNull, szChildname,
                                     nsRegularExpression,
                                     &pEnumSyms.m_Symbol))) {
      return false;
    }

    OrbitDiaSymbol pChild;
    DWORD celt = 1;

    while (SUCCEEDED(pEnumSyms->Next(celt, &pChild.m_Symbol, &celt)) &&
           (celt == 1)) {
      PrintGeneric(pChild.m_Symbol);
      PrintSymbol(pChild.m_Symbol, 0);
      pChild.Release();
    }
  }

  else {
    // If the specified name is NULL then only the parent symbol data is
    // displayed

    DWORD dwSymTag;

    if ((pSymbol->get_symTag(&dwSymTag) == S_OK) &&
        (dwSymTag == SymTagPublicSymbol)) {
      PrintPublicSymbol(pSymbol);
    }

    else {
      PrintSymbol(pSymbol, 0);
    }
  }

  return true;
}

////////////////////////////////////////////////////////////
// Dump all the type symbols information that matches their
//  names to a specified regular expression string
//
bool DumpType(IDiaSymbol* pGlobal, const wchar_t* szRegEx) {
  OrbitDiaEnumSymbols pEnumSymbols;

  if (FAILED(pGlobal->findChildren(SymTagUDT, szRegEx, nsRegularExpression,
                                   &pEnumSymbols.m_Symbol))) {
    return false;
  }

  OrbitDiaSymbol pSymbol;
  ULONG celt = 0;

  while (SUCCEEDED(pEnumSymbols->Next(1, &pSymbol.m_Symbol, &celt)) &&
         (celt == 1)) {
    PrintTypeInDetail(pSymbol.m_Symbol, 0);

    pSymbol.Release();
  }

  return true;
}

////////////////////////////////////////////////////////////
// Dump line numbering information for a given file name and
//  an optional line number
//
bool DumpLinesForSourceFile(IDiaSession* pSession, const wchar_t* szFileName,
                            DWORD dwLine) {
  IDiaEnumSourceFiles* pEnumSrcFiles;

  if (FAILED(
          pSession->findFile(NULL, szFileName, nsFNameExt, &pEnumSrcFiles))) {
    return false;
  }

  IDiaSourceFile* pSrcFile;
  ULONG celt = 0;

  while (SUCCEEDED(pEnumSrcFiles->Next(1, &pSrcFile, &celt)) && (celt == 1)) {
    IDiaEnumSymbols* pEnumCompilands;

    if (pSrcFile->get_compilands(&pEnumCompilands) == S_OK) {
      OrbitDiaSymbol pCompiland;

      celt = 0;
      while (SUCCEEDED(pEnumCompilands->Next(1, &pCompiland.m_Symbol, &celt)) &&
             (celt == 1)) {
        BSTR bstrName;

        if (pCompiland->get_name(&bstrName) == S_OK) {
          PRINTF(L"Compiland = %s\n", bstrName);

          SysFreeString(bstrName);
        }

        else {
          PRINTF(L"Compiland = (???)\n");
        }

        IDiaEnumLineNumbers* pLines;

        if (dwLine != 0) {
          if (SUCCEEDED(pSession->findLinesByLinenum(
                  pCompiland.m_Symbol, pSrcFile, dwLine, 0, &pLines))) {
            PrintLines(pLines);
            pLines->Release();
          }
        }

        else {
          if (SUCCEEDED(pSession->findLines(pCompiland.m_Symbol, pSrcFile,
                                            &pLines))) {
            PrintLines(pLines);
            pLines->Release();
          }
        }
      }

      pEnumCompilands->Release();
    }

    pSrcFile->Release();
  }

  pEnumSrcFiles->Release();

  return true;
}

////////////////////////////////////////////////////////////
// Dump public symbol information for a given number of
//  symbols around a given RVA address
//
bool DumpPublicSymbolsSorted(IDiaSession* pSession, DWORD dwRVA, DWORD dwRange,
                             bool bReverse) {
  IDiaEnumSymbolsByAddr* pEnumSymsByAddr;

  if (FAILED(pSession->getSymbolsByAddr(&pEnumSymsByAddr))) {
    return false;
  }

  OrbitDiaSymbol pSymbol;

  if (SUCCEEDED(pEnumSymsByAddr->symbolByRVA(dwRVA, &pSymbol.m_Symbol))) {
    if (dwRange == 0) {
      PrintPublicSymbol(pSymbol.m_Symbol);
    }

    ULONG celt;
    ULONG i;

    if (bReverse) {
      pSymbol.Release();

      i = 0;

      for (pSymbol = NULL;
           (i < dwRange) &&
           SUCCEEDED(pEnumSymsByAddr->Next(1, &pSymbol.m_Symbol, &celt)) &&
           (celt == 1);
           i++) {
        PrintPublicSymbol(pSymbol.m_Symbol);
        pSymbol.Release();
      }
    } else {
      PrintPublicSymbol(pSymbol.m_Symbol);
      pSymbol.Release();

      i = 1;
      for (pSymbol.m_Symbol = NULL;
           (i < dwRange) &&
           SUCCEEDED(pEnumSymsByAddr->Prev(1, &pSymbol.m_Symbol, &celt)) &&
           (celt == 1);
           i++) {
        PrintPublicSymbol(pSymbol.m_Symbol);
      }
    }
  }

  pEnumSymsByAddr->Release();

  return true;
}

////////////////////////////////////////////////////////////
// Dump label symbol information at a given RVA
//
bool DumpLabel(IDiaSession* pSession, DWORD dwRVA) {
  OrbitDiaSymbol pSymbol;
  LONG lDisplacement;

  if (FAILED(pSession->findSymbolByRVAEx(dwRVA, SymTagLabel, &pSymbol.m_Symbol,
                                         &lDisplacement)) ||
      (pSymbol.m_Symbol == NULL)) {
    return false;
  }

  PRINTF(L"Displacement = 0x%X\n", lDisplacement);
  PrintGeneric(pSymbol.m_Symbol);
  return true;
}

////////////////////////////////////////////////////////////
// Dump annotation symbol information at a given RVA
//
bool DumpAnnotations(IDiaSession* pSession, DWORD dwRVA) {
  OrbitDiaSymbol pSymbol;
  LONG lDisplacement;

  if (FAILED(pSession->findSymbolByRVAEx(dwRVA, SymTagAnnotation,
                                         &pSymbol.m_Symbol, &lDisplacement)) ||
      (pSymbol.m_Symbol == NULL)) {
    return false;
  }

  PRINTF(L"Displacement = 0x%X\n", lDisplacement);
  PrintGeneric(pSymbol.m_Symbol);
  return true;
}

struct OMAP_DATA {
  DWORD dwRVA;
  DWORD dwRVATo;
};

////////////////////////////////////////////////////////////
//
bool DumpMapToSrc(IDiaSession* pSession, DWORD dwRVA) {
  IDiaEnumDebugStreams* pEnumStreams;
  IDiaEnumDebugStreamData* pStream;
  ULONG celt;

  if (FAILED(pSession->getEnumDebugStreams(&pEnumStreams))) {
    return false;
  }

  celt = 0;

  for (; SUCCEEDED(pEnumStreams->Next(1, &pStream, &celt)) && (celt == 1);
       pStream = NULL) {
    BSTR bstrName;

    if (pStream->get_name(&bstrName) != S_OK) {
      bstrName = NULL;
    }

    if (bstrName && wcscmp(bstrName, L"OMAPTO") == 0) {
      OMAP_DATA data, datasav;
      DWORD cbData;
      DWORD dwRVATo = 0;
      unsigned int i;

      datasav.dwRVATo = 0;
      datasav.dwRVA = 0;

      while (SUCCEEDED(pStream->Next(1, sizeof(data), &cbData, (BYTE*)&data,
                                     &celt)) &&
             (celt == 1)) {
        if (dwRVA > data.dwRVA) {
          datasav = data;
          continue;
        }

        else if (dwRVA == data.dwRVA) {
          dwRVATo = data.dwRVATo;
        }

        else if (datasav.dwRVATo) {
          dwRVATo = datasav.dwRVATo + (dwRVA - datasav.dwRVA);
        }
        break;
      }

      PRINTF(
          L"image rva = %08X ==> source rva = %08X\n\nRelated OMAP entries:\n",
          dwRVA, dwRVATo);
      PRINTF(L"image rva ==> source rva\n");
      PRINTF(L"%08X  ==> %08X\n", datasav.dwRVA, datasav.dwRVATo);

      i = 0;

      do {
        PRINTF(L"%08X  ==> %08X\n", data.dwRVA, data.dwRVATo);
      } while ((++i) < 5 &&
               SUCCEEDED(pStream->Next(1, sizeof(data), &cbData, (BYTE*)&data,
                                       &celt)) &&
               (celt == 1));
    }

    if (bstrName != NULL) {
      SysFreeString(bstrName);
    }

    pStream->Release();
  }

  pEnumStreams->Release();

  return true;
}

////////////////////////////////////////////////////////////
//
bool DumpMapFromSrc(IDiaSession* pSession, DWORD dwRVA) {
  IDiaEnumDebugStreams* pEnumStreams;

  if (FAILED(pSession->getEnumDebugStreams(&pEnumStreams))) {
    return false;
  }

  IDiaEnumDebugStreamData* pStream;
  ULONG celt = 0;

  for (; SUCCEEDED(pEnumStreams->Next(1, &pStream, &celt)) && (celt == 1);
       pStream = NULL) {
    BSTR bstrName;

    if (pStream->get_name(&bstrName) != S_OK) {
      bstrName = NULL;
    }

    if (bstrName && wcscmp(bstrName, L"OMAPFROM") == 0) {
      OMAP_DATA data;
      OMAP_DATA datasav;
      DWORD cbData;
      DWORD dwRVATo = 0;
      unsigned int i;

      datasav.dwRVATo = 0;
      datasav.dwRVA = 0;

      while (SUCCEEDED(pStream->Next(1, sizeof(data), &cbData, (BYTE*)&data,
                                     &celt)) &&
             (celt == 1)) {
        if (dwRVA > data.dwRVA) {
          datasav = data;
          continue;
        }

        else if (dwRVA == data.dwRVA) {
          dwRVATo = data.dwRVATo;
        }

        else if (datasav.dwRVATo) {
          dwRVATo = datasav.dwRVATo + (dwRVA - datasav.dwRVA);
        }
        break;
      }

      PRINTF(
          L"source rva = %08X ==> image rva = %08X\n\nRelated OMAP entries:\n",
          dwRVA, dwRVATo);
      PRINTF(L"source rva ==> image rva\n");
      PRINTF(L"%08X  ==> %08X\n", datasav.dwRVA, datasav.dwRVATo);

      i = 0;

      do {
        PRINTF(L"%08X  ==> %08X\n", data.dwRVA, data.dwRVATo);
      } while ((++i) < 5 &&
               SUCCEEDED(pStream->Next(1, sizeof(data), &cbData, (BYTE*)&data,
                                       &celt)) &&
               (celt == 1));
    }

    if (bstrName != NULL) {
      SysFreeString(bstrName);
    }

    pStream->Release();
  }

  pEnumStreams->Release();

  return true;
}

////////////////////////////////////////////////////////////
// Retreive the table that matches the given iid
//
//  A PDB table could store the section contributions, the frame data,
//  the injected sources
//
HRESULT GetTable(IDiaSession* pSession, REFIID iid, void** ppUnk) {
  IDiaEnumTables* pEnumTables;

  if (FAILED(pSession->getEnumTables(&pEnumTables))) {
    PRINTF(L"ERROR - GetTable() getEnumTables\n");

    return E_FAIL;
  }

  IDiaTable* pTable;
  ULONG celt = 0;

  while (SUCCEEDED(pEnumTables->Next(1, &pTable, &celt)) && (celt == 1)) {
    // There's only one table that matches the given IID

    if (SUCCEEDED(pTable->QueryInterface(iid, (void**)ppUnk))) {
      pTable->Release();
      pEnumTables->Release();

      return S_OK;
    }

    pTable->Release();
  }

  pEnumTables->Release();

  return E_FAIL;
}
