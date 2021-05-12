// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "LibunwindstackMaps.h"

namespace orbit_linux_tracing {

namespace {

class LibunwindstackMapsImpl : public LibunwindstackMaps {
 public:
  explicit LibunwindstackMapsImpl(std::unique_ptr<unwindstack::BufferMaps> maps)
      : maps_{std::move(maps)} {}

  unwindstack::MapInfo* Find(uint64_t pc) override { return maps_->Find(pc); }

  unwindstack::Maps* Get() override { return maps_.get(); }

  void Add(uint64_t start, uint64_t end, uint64_t offset, uint64_t flags, const std::string& name,
           uint64_t load_bias) override {
    maps_->Add(start, end, offset, flags, name, load_bias);
  }

  void Sort() override { maps_->Sort(); }

 private:
  std::unique_ptr<unwindstack::BufferMaps> maps_;
};
}  // namespace

std::unique_ptr<LibunwindstackMaps> LibunwindstackMaps::ParseMaps(const std::string& maps_buffer) {
  auto maps = std::make_unique<unwindstack::BufferMaps>(maps_buffer.c_str());
  if (!maps->Parse()) {
    return nullptr;
  }
  return std::make_unique<LibunwindstackMapsImpl>(std::move(maps));
}

}  // namespace orbit_linux_tracing