// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#ifndef ORBIT_BASE_APPEND_H_
#define ORBIT_BASE_APPEND_H_

namespace orbit_base {

template <class T>
void Append(std::vector<T>& dest, const std::vector<T>& source) {
  dest.insert(std::end(dest), std::begin(source), std::end(source));
}

}  // namespace orbit_base

#endif  // ORBIT_BASE_APPEND_H_
