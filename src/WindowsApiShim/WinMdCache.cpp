// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "WinMdCache.h"

#include <absl/strings/match.h>

namespace orbit_windows_api_shim {

WinMdCache::WinMdCache(winmd::reader::cache* winmd_cache) : winmd_cache_(winmd_cache) {
  for (auto& [ns, members] : winmd_cache_->namespaces()) {
    if (!ns.empty()) {
      cache_entries_.emplace_back(Entry{ns, &members});
    }
  }
}

WinMdCache::WinMdCache(winmd::reader::cache* winmd_cache,
                       std::set<std::string_view> namespace_filters)
    : winmd_cache_(winmd_cache) {
  for (auto& [ns, members] : winmd_cache_->namespaces()) {
    for (std::string_view filter : namespace_filters) {
      if (absl::StrContains(ns, filter)) {
        cache_entries_.emplace_back(Entry{ns, &members});
        break;
      }
    }
  }
}

}  // namespace orbit_windows_api_shim