// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_CORE_KEY_AND_STRING_H_
#define ORBIT_CORE_KEY_AND_STRING_H_

#include <string>
#include <utility>

struct KeyAndString {
  KeyAndString() = default;
  KeyAndString(uint64_t key, std::string str) : key(key), str(std::move(str)) {}

  uint64_t key = 0;
  std::string str;
};

#endif  // ORBIT_CORE_KEY_AND_STRING_H_
