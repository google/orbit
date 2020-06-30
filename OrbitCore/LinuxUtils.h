// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "BaseTypes.h"
#include "module.pb.h"

struct Module;

//-----------------------------------------------------------------------------
namespace LinuxUtils {
std::string ExecuteCommand(const char* a_Cmd);
std::vector<std::string> ReadProcMaps(pid_t pid);
std::vector<ModuleInfo> ListModules(int32_t pid);
std::unordered_map<pid_t, double> GetCpuUtilization();
bool Is64Bit(pid_t pid);
}  // namespace LinuxUtils
