// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINUX_TRACING_USER_SPACE_INSTRUMENTATION_ADDRESSES_H_
#define LINUX_TRACING_USER_SPACE_INSTRUMENTATION_ADDRESSES_H_

#include <cstdint>
#include <string_view>

namespace orbit_linux_tracing {

// This interface carries information about memory addresses dedicated to user space
// instrumentation, in particular the entry trampolines, the return trampoline, and the injected
// library. It allows querying whether an address belongs to a trampoline, and the map name of the
// injected library.
class UserSpaceInstrumentationAddresses {
 public:
  virtual ~UserSpaceInstrumentationAddresses() = default;
  [[nodiscard]] virtual bool IsInEntryTrampoline(uint64_t address) const = 0;
  [[nodiscard]] virtual bool IsInReturnTrampoline(uint64_t address) const = 0;
  [[nodiscard]] virtual std::string_view GetInjectedLibraryMapName() const = 0;

  [[nodiscard]] bool IsInEntryOrReturnTrampoline(uint64_t address) const {
    return IsInEntryTrampoline(address) || IsInReturnTrampoline(address);
  }
};

}  // namespace orbit_linux_tracing

#endif  // LINUX_TRACING_USER_SPACE_INSTRUMENTATION_ADDRESSES_H_
