/*
 * Copyright (c) 2020 The Orbit Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#pragma once

#include <string>
#include <utility>

#include "Serialization.h"
#include "SerializationMacros.h"

struct LinuxAddressInfo {
  LinuxAddressInfo() = default;
  LinuxAddressInfo(uint64_t address, std::string module_name,
                   std::string function_name, uint64_t offset_in_function)
      : address{address},
        module_name{std::move(module_name)},
        function_name{std::move(function_name)},
        offset_in_function{offset_in_function} {}

  uint64_t address = 0;
  std::string module_name;
  std::string function_name;
  uint64_t offset_in_function = 0;

  ORBIT_SERIALIZABLE;
};

ORBIT_SERIALIZE(LinuxAddressInfo, 0) {
  ORBIT_NVP_VAL(0, module_name);
  ORBIT_NVP_VAL(0, function_name);
  ORBIT_NVP_VAL(0, address);
  ORBIT_NVP_VAL(0, offset_in_function);
}
