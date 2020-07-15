// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_CORE_VERSION_H_
#define ORBIT_CORE_VERSION_H_
#include <string>

namespace OrbitCore {
std::string GetVersion();
std::string GetCompiler();
std::string GetBuildTimestamp();
std::string GetBuildMachine();
std::string GetCommitHash();

// For usage with a "--version" command line flag
std::string GetBuildReport();

}  // namespace OrbitCore

#endif  // ORBIT_CORE_VERSION_H_
