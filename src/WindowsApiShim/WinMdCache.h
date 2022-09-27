// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_WINDOWS_API_SHIM_WIN_MD_CACHE_H_
#define ORBIT_WINDOWS_API_SHIM_WIN_MD_CACHE_H_

#include <cppwin32/winmd/winmd_reader.h>

#include <string_view>

namespace orbit_windows_api_shim {

// Wrapper around winmd::reader::cache that allows namespace filtering.
class WinMdCache {
 public:
  WinMdCache(winmd::reader::cache* winmd_cache);
  WinMdCache(winmd::reader::cache* winmd_cache, std::set<std::string_view> namespace_filters);

  struct Entry {
    std::string_view namespace_name;
    const winmd::reader::cache::namespace_members* namespace_members;
  };

  const std::vector<Entry>& GetCacheEntries() const { return cache_entries_; }

 private:
  std::vector<Entry> cache_entries_;
  winmd::reader::cache* winmd_cache_ = nullptr;
};

}  // namespace orbit_windows_api_shim

#endif  // ORBIT_WINDOWS_API_SHIM_WIN_MD_CACHE_H_