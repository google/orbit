// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_BASE_MAKE_UNIQUE_FOR_OVERWRITE_H_
#define ORBIT_BASE_MAKE_UNIQUE_FOR_OVERWRITE_H_

#include <memory>

// Unlike std::make_unique, make_unique_for_overwrite calls new without
// parentheses, causing the object to be default-initialized instead of
// value-initialized. With C-style structs and arrays of such structs,
// value-initialization causes zero-initialization, which might not be needed.
// Standard version coming in C++20.

template <typename T>
struct MakeUniqueForOverwriteIf {
  using SingleObject = std::unique_ptr<T>;
};

template <typename T>
struct MakeUniqueForOverwriteIf<T[]> {
  using UnknownBoundArray = std::unique_ptr<T[]>;
};

template <typename T, std::size_t Bound>
struct MakeUniqueForOverwriteIf<T[Bound]> {
  using KnownBoundArray = void;
};

template <typename T>
inline typename MakeUniqueForOverwriteIf<T>::SingleObject make_unique_for_overwrite() {
  return std::unique_ptr<T>(new T);
}

template <typename T>
inline typename MakeUniqueForOverwriteIf<T>::UnknownBoundArray make_unique_for_overwrite(
    std::size_t size) {
  return std::unique_ptr<T>(new typename std::remove_extent<T>::type[size]);
}

template <typename T, typename... Args>
inline typename MakeUniqueForOverwriteIf<T>::KnownBoundArray make_unique_for_overwrite(Args&&...) =
    delete;

#endif  // ORBIT_BASE_MAKE_UNIQUE_FOR_OVERWRITE_H_
