// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitBase/OsVersionWindows.h"

#include <Windows.h>
#include <Winnt.h>
#include <absl/strings/str_format.h>

#include "OrbitBase/GetProcAddress.h"
#include "OrbitBase/StringConversion.h"

namespace orbit_base {

typedef long NTSTATUS;
#define STATUS_SUCCESS ((NTSTATUS)0x00000000L)

ErrorMessageOr<WindowsVersion> GetWindowsVersion() {
  static auto rtl_get_version = orbit_base::GetProcAddress<NTSTATUS(WINAPI*)(PRTL_OSVERSIONINFOW)>(
      "ntdll.dll", "RtlGetVersion");
  if (rtl_get_version == nullptr) {
    return ErrorMessage("Could not find address of \"RtlGetVersion\" function in \"ntdll.dll\"");
  }

  RTL_OSVERSIONINFOEXW info{};
  info.dwOSVersionInfoSize = sizeof(info);
  if (rtl_get_version((PRTL_OSVERSIONINFOW)&info) == STATUS_SUCCESS) {
    WindowsVersion version;
    version.major_version = info.dwMajorVersion;
    version.minor_version = info.dwMinorVersion;
    version.build_number = info.dwBuildNumber;
    version.service_pack_version = orbit_base::ToStdString(info.szCSDVersion);
    return version;
  }

  // From Microsoft's doc: "RtlGetVersion returns STATUS_SUCCESS", so this should never happen.
  return ErrorMessage("Error calling \"RtlGetVersion\"");
}

ErrorMessageOr<std::string> GetWindowsVersionAsString() {
  OUTCOME_TRY(WindowsVersion version, GetWindowsVersion());
  return absl::StrFormat("%u.%u build: %u service pack: \"%s\" platform id: %u",
                         version.major_version, version.minor_version, version.build_number,
                         version.service_pack_version, version.platform_id);
}

}  // namespace orbit_base
