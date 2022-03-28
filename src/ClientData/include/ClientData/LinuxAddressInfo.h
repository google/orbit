// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CLIENT_DATA_LINUX_ADDRESS_INFO_H_
#define CLIENT_DATA_LINUX_ADDRESS_INFO_H_

#include <string>

namespace orbit_client_data {

class LinuxAddressInfo {
 public:
  explicit LinuxAddressInfo(uint64_t absolute_address, uint64_t offset_in_function,
                            std::string module_path, std::string function_name)
      : absolute_address_{absolute_address},
        offset_in_function_{offset_in_function},
        module_path_{std::move(module_path)},
        function_name_{std::move(function_name)} {}

  [[nodiscard]] uint64_t absolute_address() const { return absolute_address_; }
  [[nodiscard]] uint64_t offset_in_function() const { return offset_in_function_; }
  [[nodiscard]] const std::string& module_path() const { return module_path_; }
  [[nodiscard]] const std::string& function_name() const { return function_name_; }

 private:
  uint64_t absolute_address_;
  uint64_t offset_in_function_;
  std::string module_path_;
  std::string function_name_;
};

}  // namespace orbit_client_data

#endif  // CLIENT_DATA_LINUX_ADDRESS_INFO_H_
