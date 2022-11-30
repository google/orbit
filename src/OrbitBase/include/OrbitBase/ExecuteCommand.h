// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_BASE_EXECUTE_COMMAND_H_
#define ORBIT_BASE_EXECUTE_COMMAND_H_

#include <optional>
#include <string>

namespace orbit_base {

#if defined(__linux)

std::optional<std::string> ExecuteCommand(std::string_view cmd);

#endif  // defined(__linux)

}  // namespace orbit_base

#endif  // ORBIT_BASE_EXECUTE_COMMAND_H_
