// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitFunction.h"

#include "FunctionUtils.h"
#include "Serialization.h"

Function::Function(std::string_view name, std::string_view pretty_name,
                   uint64_t address, uint64_t load_bias, uint64_t size,
                   std::string_view file, uint32_t line,
                   std::string_view loaded_module_path,
                   uint64_t module_base_address)
    : name_(name),
      pretty_name_(pretty_name),
      loaded_module_path_(loaded_module_path),
      module_base_address_(module_base_address),
      address_(address),
      load_bias_(load_bias),
      size_(size),
      file_(file),
      line_(line) {
  stats_->Clear();
  FunctionUtils::SetOrbitTypeFromName(this);
}

ORBIT_SERIALIZE(Function, 5) {
  ORBIT_NVP_VAL(4, name_);
  ORBIT_NVP_VAL(4, pretty_name_);
  ORBIT_NVP_VAL(4, loaded_module_path_);
  ORBIT_NVP_VAL(4, module_base_address_);
  ORBIT_NVP_VAL(4, address_);
  ORBIT_NVP_VAL(4, load_bias_);
  ORBIT_NVP_VAL(4, size_);
  ORBIT_NVP_VAL(4, file_);
  ORBIT_NVP_VAL(4, line_);
  // ORBIT_NVP_VAL(4, stats_);
}
