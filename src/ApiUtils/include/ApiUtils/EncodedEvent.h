// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_API_UTILS_ENCODED_EVENT_H_
#define ORBIT_API_UTILS_ENCODED_EVENT_H_

#include <stdint.h>

#include <cstring>

#include "ApiInterface/Orbit.h"

// TODO(kuebler): Either remove Encode/Decode completely or at least move it to a more suitable
//  place.
namespace orbit_api {

template <typename Dest, typename Source>
inline Dest Encode(const Source& source) {
  static_assert(sizeof(Source) <= sizeof(Dest), "orbit_api::Encode destination type is too small");
  Dest dest = 0;
  std::memcpy(&dest, &source, sizeof(Source));
  return dest;
}

template <typename Dest, typename Source>
inline Dest Decode(const Source& source) {
  static_assert(sizeof(Dest) <= sizeof(Source), "orbit_api::Decode destination type is too big");
  Dest dest = 0;
  std::memcpy(&dest, &source, sizeof(Dest));
  return dest;
}

}  // namespace orbit_api

#endif  // ORBIT_API_UTILS_ENCODED_EVENT_H_
