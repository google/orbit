// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_CORE_FUNCTION_UTILS_H_
#define ORBIT_CORE_FUNCTION_UTILS_H_

#include <cstdint>
#include <string>

#include "OrbitFunction.h"
#include "ScopeTimer.h"

namespace function {

inline const std::string& GetDisplayName(const Function& func) {
  return func.pretty_name().empty() ? func.name() : func.pretty_name();
}

std::string GetLoadedModuleName(const Function& func);
uint64_t GetHash(const Function& func);

// Calculates and returns the absolute address of the function.
uint64_t Offset(const Function& func);
inline uint64_t GetAbsoluteAddress(const Function& func) {
  return func.address() + func.module_base_address() - func.load_bias();
}

bool IsOrbitFunc(const Function& func);

void Select(Function* func);
void UnSelect(Function* func);
bool IsSelected(const Function& func);

void Print(const Function& func);

bool SetOrbitTypeFromName(Function* func);
void UpdateStats(Function* func, const Timer& timer);

}  // namespace function

#endif  // ORBIT_CORE_FUNCTION_UTILS_H_
