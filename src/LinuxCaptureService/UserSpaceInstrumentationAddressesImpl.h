// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINUX_CAPTURE_SERVICE_USER_SPACE_INSTRUMENTATION_ADDRESSES_IMPL_H_
#define LINUX_CAPTURE_SERVICE_USER_SPACE_INSTRUMENTATION_ADDRESSES_IMPL_H_

#include <algorithm>
#include <vector>

#include "LinuxTracing/UserSpaceInstrumentationAddresses.h"
#include "UserSpaceInstrumentation/AddressRange.h"

namespace orbit_linux_capture_service {

class UserSpaceInstrumentationAddressesImpl final
    : public orbit_linux_tracing::UserSpaceInstrumentationAddresses {
 public:
  explicit UserSpaceInstrumentationAddressesImpl(
      std::vector<orbit_user_space_instrumentation::AddressRange> entry_trampoline_address_ranges,
      orbit_user_space_instrumentation::AddressRange return_trampoline_address_range,
      std::string injected_library_map_name)
      : entry_trampoline_address_ranges_{std::move(entry_trampoline_address_ranges)},
        return_trampoline_address_range_{return_trampoline_address_range},
        injected_library_map_name_{std::move(injected_library_map_name)} {}

  [[nodiscard]] bool IsInEntryTrampoline(uint64_t address) const override {
    // The number of expected AddressRanges for entry trampolines is very limited (one per
    // dynamically instrumented module), so keep it simple and just perform a linear search.
    return std::any_of(
        entry_trampoline_address_ranges_.begin(), entry_trampoline_address_ranges_.end(),
        [address](const orbit_user_space_instrumentation::AddressRange& address_range) {
          return address >= address_range.start && address < address_range.end;
        });
  }

  [[nodiscard]] bool IsInReturnTrampoline(uint64_t address) const override {
    return address >= return_trampoline_address_range_.start &&
           address < return_trampoline_address_range_.end;
  }

  [[nodiscard]] std::string_view GetInjectedLibraryMapName() const override {
    return injected_library_map_name_;
  }

 private:
  std::vector<orbit_user_space_instrumentation::AddressRange> entry_trampoline_address_ranges_;
  // User space instrumentation creates and uses a single return trampoline per process.
  orbit_user_space_instrumentation::AddressRange return_trampoline_address_range_;
  std::string injected_library_map_name_;
};

}  // namespace orbit_linux_capture_service

#endif  // LINUX_CAPTURE_SERVICE_USER_SPACE_INSTRUMENTATION_ADDRESSES_IMPL_H_
