// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "WinMdHelper.h"

#include <absl/strings/ascii.h>
#include <absl/strings/match.h>
#include <absl/strings/str_format.h>
#include <absl/strings/str_replace.h>

namespace orbit_windows_api_shim {

using winmd::reader::database;
using winmd::reader::ImplMap;
using winmd::reader::MethodDef;
using winmd::reader::ModuleRef;

WinMdHelper::WinMdHelper(const database& db) {
  // Populate a map of method_def to module_ref.
  for (const ImplMap& impl_map : db.get_table<ImplMap>()) {
    ModuleRef module_ref = db.get_table<ModuleRef>()[impl_map.ImportScope().index()];
    const MethodDef& method_def = db.get_table<MethodDef>()[impl_map.MemberForwarded().index()];
    method_def_to_module_ref_map_.emplace(method_def, module_ref);
  }
}

std::string WinMdHelper::GetFunctionKeyFromMethodDef(
    const winmd::reader::MethodDef& method_def) const {
  // Include the module name so that all function names are globally unique.

  std::string module_name =
      absl::StrReplaceAll(GetModuleNameFromMethodDef(method_def), {{".", "_"}, {"-", "_"}});

  std::string_view function_name = method_def.Name();
  return absl::StrFormat("%s__%s", absl::AsciiStrToLower(module_name), function_name);
}

std::string WinMdHelper::GetModuleNameFromMethodDef(
    const winmd::reader::MethodDef& method_def) const {
  return std::string(method_def_to_module_ref_map_.at(method_def).Name());
}

}  // namespace orbit_windows_api_shim
