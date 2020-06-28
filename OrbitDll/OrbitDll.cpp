// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitDll.h"

#include <windows.h>

#include <fstream>
#include <iostream>

#include "..\OrbitCore\OrbitLib.h"
#include "..\OrbitCore\PrintVar.h"
#include "..\OrbitCore\TcpClient.h"

extern "C" {
__declspec(dllexport) void __cdecl OrbitInit(void* a_Host) {
  PRINT_FUNC;
  std::string host = (char*)a_Host;
  Orbit::Init(host);
}

__declspec(dllexport) void __cdecl OrbitInitRemote(void* a_Host) {
  PRINT_FUNC;
  std::string host = (char*)a_Host;
  Orbit::InitRemote(host);
}

__declspec(dllexport) bool __cdecl OrbitIsConnected() {
  return GTcpClient && GTcpClient->IsValid();
}

__declspec(dllexport) bool __cdecl OrbitStart() {
  if (OrbitIsConnected()) {
    Orbit::Start();
    return true;
  }

  return false;
}

__declspec(dllexport) bool __cdecl OrbitStop() {
  if (OrbitIsConnected()) {
    Orbit::Stop();
    return true;
  }

  return false;
}
}

BOOL WINAPI DllMain(_In_ HINSTANCE hinstDLL, _In_ DWORD fdwReason,
                    _In_ LPVOID lpvReserved) {
  switch (fdwReason) {
    case DLL_PROCESS_ATTACH:
      // OutputDebugString(L"DLL_PROCESS_ATTACH\n");
      break;

    case DLL_PROCESS_DETACH:
      // OutputDebugString(L"DLL_PROCESS_DETACH\n");
      break;

    case DLL_THREAD_ATTACH:
      // OutputDebugString(L"DLL_THREAD_ATTACH\n");
      break;

    case DLL_THREAD_DETACH:
      // OutputDebugString(L"DLL_THREAD_DETACH\n");
      break;
    default:
      // OutputDebugString(L"DLL_UNKNOWN\n");
      break;
  }

  return TRUE;
}
