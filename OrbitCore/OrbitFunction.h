// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <string>
#include <string_view>

#include "SerializationMacros.h"
#include "capture.pb.h"

class Function {
 public:
  // TODO enum class instead of enum
  enum OrbitType {
    NONE,
    ORBIT_TIMER_START,
    ORBIT_TIMER_STOP,
    ORBIT_LOG,
    ORBIT_TIMER_START_ASYNC,
    ORBIT_TIMER_STOP_ASYNC,
    ORBIT_TRACK_INT,
    ORBIT_TRACK_INT_64,
    ORBIT_TRACK_UINT,
    ORBIT_TRACK_UINT_64,
    ORBIT_TRACK_FLOAT,
    ORBIT_TRACK_DOUBLE,
    ORBIT_TRACK_FLOAT_AS_INT,
    ORBIT_TRACK_DOUBLE_AS_INT_64,
    // Append new types here.

    NUM_TYPES
  };

  Function() : Function{"", "", 0, 0, 0, "", 0, "", 0} {}

  Function(std::string_view name, std::string_view pretty_name,
           uint64_t address, uint64_t load_bias, uint64_t size,
           std::string_view file, uint32_t line,
           std::string_view loaded_module_path, uint64_t module_base_address);

  const std::string& name() const { return name_; }
  const std::string& pretty_name() const { return pretty_name_; }
  const std::string& loaded_module_path() const { return loaded_module_path_; }
  void set_loaded_module_path(const std::string& loaded_module_path) {
    loaded_module_path_ = loaded_module_path;
  }
  uint64_t module_base_address() const { return module_base_address_; }
  void set_module_base_address(uint64_t module_base_address) {
    module_base_address_ = module_base_address;
  }

  uint64_t size() const { return size_; }
  const std::string& file() const { return file_; }
  void set_file(std::string file) { file_ = std::move(file); }
  uint32_t line() const { return line_; }
  void set_line(uint32_t line) { line_ = line; }
  uint64_t address() const { return address_; }
  uint64_t load_bias() const { return load_bias_; }
  OrbitType orbit_type() const { return type_; }
  void set_orbit_type(OrbitType type) { type_ = type; }
  std::shared_ptr<FunctionStats> stats() const { return stats_; }

  ORBIT_SERIALIZABLE;

 private:
  std::string name_;
  std::string pretty_name_;
  std::string loaded_module_path_;
  uint64_t module_base_address_ = 0;
  uint64_t address_;
  uint64_t load_bias_;
  uint64_t size_;
  std::string file_;
  uint32_t line_;
  OrbitType type_ = NONE;
  std::shared_ptr<FunctionStats> stats_;
};
