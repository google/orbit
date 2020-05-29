/*
 * Copyright (c) 2020 The Orbit Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#pragma once

struct OrbitWaitLoop;
struct FunctionArgInfo;
struct OrbitUnrealInfo;

namespace Hijacking {
bool CreateHook(void* a_FunctionAddress);
bool CreateZoneStartHook(void* a_FunctionAddress);
bool CreateZoneStopHook(void* a_FunctionAddress);
bool CreateOutputDebugStringHook(void* a_FunctionAddress);
bool CreateSendDataHook(void* a_FunctionAddress);
bool CreateUnrealActorHook(void* a_FunctionAddress);
bool CreateAllocHook(void* a_FunctionAddress);
bool CreateFreeHook(void* a_FunctionAddress);

bool CreateHook(void* a_FunctionAddress, void* a_PrologCallback,
                void* a_EpilogCallback);
bool EnableHook(void* a_FunctionAddress);
void EnableHooks(DWORD64* a_Addresses, uint32_t a_NumAddresses);
bool DisableHook(void* a_FunctionAddress);
bool DisableAllHooks();
bool SuspendBusyLoopThread(OrbitWaitLoop* a_WaitLoop);
bool ThawMainThread(OrbitWaitLoop* a_WaitLoop);

void ClearFunctionArguments();
void SetFunctionArguments(ULONG64 a_FunctionAddress,
                          const FunctionArgInfo& a_Args);
void TrackCallstack(ULONG64 a_FunctionAddress);
void SetUnrealInfo(OrbitUnrealInfo& a_UnrealInfo);
}  // namespace Hijacking

//-----------------------------------------------------------------------------
struct ReturnAddress {
  ReturnAddress()
      : m_AddressOfReturnAddress(nullptr),
        m_OriginalReturnAddress(nullptr),
        m_EpilogAddress(nullptr) {}

  void** m_AddressOfReturnAddress;
  void* m_OriginalReturnAddress;
  void* m_EpilogAddress;
};
