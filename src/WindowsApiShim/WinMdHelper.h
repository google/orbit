// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_WINDOWS_API_SHIM_WIN_MD_HELPER_H_
#define ORBIT_WINDOWS_API_SHIM_WIN_MD_HELPER_H_

#include <absl/container/flat_hash_map.h>
#include <cppwin32/cmd_reader.h>
#include <cppwin32/winmd/winmd_reader.h>

#include <string_view>

namespace orbit_windows_api_shim {

class WinMdHelper {
 public:
  WinMdHelper() = delete;
  WinMdHelper(const winmd::reader::database& db);

  [[nodiscard]] std::string GetFunctionKeyFromMethodDef(
      const winmd::reader::MethodDef& method_def) const;
  [[nodiscard]] std::string GetModuleNameFromMethodDef(
      const winmd::reader::MethodDef& method_def) const;

 private:
  std::map<winmd::reader::MethodDef, winmd::reader::ModuleRef> method_def_to_module_ref_map_;
};

}  // namespace orbit_windows_api_shim

#endif  // ORBIT_WINDOWS_API_SHIM_WIN_MD_HELPER_H_
