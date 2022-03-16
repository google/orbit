// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "WindowsUtils/AdjustTokenPrivilege.h"

#include "OrbitBase/StringConversion.h"

namespace orbit_windows_utils {

ErrorMessageOr<void> AdjustTokenPrivilege(LPCTSTR token_name, bool enabled) {
  std::string token_name_str(orbit_base::ToStdString(token_name));
  HANDLE token_handle;
  if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &token_handle)) {
    return ErrorMessage{absl::StrFormat("Unable to open process token \"%s\"", token_name_str)};
  }

  TOKEN_PRIVILEGES token_privileges;
  LUID luid;
  if (!LookupPrivilegeValue(NULL, token_name, &luid)) {
    return ErrorMessage{
        absl::StrFormat("Unable to lookup privilege value for token \"%s\"", token_name_str)};
  }

  token_privileges.PrivilegeCount = 1;
  token_privileges.Privileges[0].Luid = luid;
  token_privileges.Privileges[0].Attributes = enabled ? SE_PRIVILEGE_ENABLED : 0;

  if (!AdjustTokenPrivileges(token_handle, /*DisableAllPrivileges=*/FALSE, &token_privileges,
                             sizeof(TOKEN_PRIVILEGES), /*PreviousState=*/nullptr,
                             /*ReturnLength=*/nullptr)) {
    return ErrorMessage{
        absl::StrFormat("Unable to adjust privilege value for token \"%s\"", token_name_str)};
  }

  if (GetLastError() == ERROR_NOT_ALL_ASSIGNED) {
    return ErrorMessage{
        absl::StrFormat("The token \"%s\" does not have the specified privilege", token_name_str)};
  }

  return outcome::success();
}

}  // namespace orbit_windows_utils
