// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_API_ENCODED_EVENT_H_
#define ORBIT_API_ENCODED_EVENT_H_

#include <stdint.h>

#include <cstring>

#include "Orbit.h"

namespace orbit_api {

constexpr uint8_t kVersion = 1;

enum EventType : uint8_t {
  kNone = 0,
  kScopeStart = 1,
  kScopeStop = 2,
  kScopeStartAsync = 3,
  kScopeStopAsync = 4,
  kTrackInt = 5,
  kTrackInt64 = 6,
  kTrackUint = 7,
  kTrackUint64 = 8,
  kTrackFloat = 9,
  kTrackDouble = 10,
  kString = 11,
};

constexpr size_t kMaxEventStringSize = 34;
struct Event {
  uint8_t version;                 // 1
  uint8_t type;                    // 1
  char name[kMaxEventStringSize];  // 34
  orbit_api_color color;           // 4
  uint64_t data;                   // 8
};

union EncodedEvent {
  EncodedEvent(orbit_api::EventType type, const char* name = nullptr, uint64_t data = 0,
               orbit_api_color color = kOrbitColorAuto) {
    static_assert(sizeof(EncodedEvent) == 48, "orbit_api::EncodedEvent should be 48 bytes.");
    static_assert(sizeof(Event) == 48, "orbit_api::Event should be 48 bytes.");
    event.version = kVersion;
    event.type = static_cast<uint8_t>(type);
    memset(event.name, 0, kMaxEventStringSize);
    if (name != nullptr) {
      std::strncpy(event.name, name, kMaxEventStringSize - 1);
      event.name[kMaxEventStringSize - 1] = 0;
    }
    event.data = data;
    event.color = color;
  }

  EncodedEvent(uint64_t a0, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4, uint64_t a5) {
    args[0] = a0;
    args[1] = a1;
    args[2] = a2;
    args[3] = a3;
    args[4] = a4;
    args[5] = a5;
  }
  Event event;
  uint64_t args[6];
};

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

#endif  // ORBIT_API_ENCODED_EVENT_H_
