// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "include/Api/EncodedString.h"

#include <absl/base/casts.h>

#include "capture.pb.h"

namespace orbit_api {

namespace {
inline void DecodeString(uint64_t encoded_name, std::string* out) {
  for (int32_t i = 56; i >= 0; i -= 8) {
    char c = static_cast<char>((encoded_name & (0xffL << i)) >> i);
    if (c == '\0') {
      return;
    }
    out->push_back(c);
  }
}

inline bool EncodeFirstChunk(const char* source, uint64_t* out) {
  uint64_t result = 0;
  for (size_t i = 0; i < 8; i++) {
    auto current = absl::bit_cast<unsigned char>(source[i]);
    if (current == '\0') {
      *out = result;
      return false;
    }
    result |= (static_cast<uint64_t>(current) << (7 - i) * 8);
  }
  *out = result;
  return true;
}

#define ENCODE_CHUNK_NUMBER(i)                                                                   \
  do {                                                                                           \
    bool continue_reading = EncodeFirstChunk(source + ((i - 1) * 8), &encoded_first_four_bytes); \
    (dest->*write_chunk_##i)(encoded_first_four_bytes);                                          \
    if (!continue_reading) {                                                                     \
      return;                                                                                    \
    }                                                                                            \
  } while (0)
}  // namespace

std::string DecodeString(uint64_t encoded_name_1, uint64_t encoded_name_2, uint64_t encoded_name_3,
                         uint64_t encoded_name_4, uint64_t encoded_name_5, uint64_t encoded_name_6,
                         uint64_t encoded_name_7, uint64_t encoded_name_8,
                         const uint64_t* encoded_name_additional,
                         size_t encoded_name_addition_count) {
  std::string result{};
  if (encoded_name_1 == 0) return result;
  DecodeString(encoded_name_1, &result);

  if (encoded_name_2 == 0) return result;
  DecodeString(encoded_name_2, &result);

  if (encoded_name_3 == 0) return result;
  DecodeString(encoded_name_3, &result);

  if (encoded_name_4 == 0) return result;
  DecodeString(encoded_name_4, &result);

  if (encoded_name_5 == 0) return result;
  DecodeString(encoded_name_5, &result);

  if (encoded_name_6 == 0) return result;
  DecodeString(encoded_name_6, &result);

  if (encoded_name_7 == 0) return result;
  DecodeString(encoded_name_7, &result);

  if (encoded_name_8 == 0) return result;
  DecodeString(encoded_name_8, &result);

  for (size_t i = 0; i < encoded_name_addition_count; i++) {
    uint64_t current_encoded_name_additional = encoded_name_additional[i];
    if (current_encoded_name_additional == 0) return result;
    DecodeString(current_encoded_name_additional, &result);
  }
  return result;
}

template <typename Dest>
void EncodeString(const char* source, Dest* dest, void (Dest::*write_chunk_1)(uint64_t),
                  void (Dest::*write_chunk_2)(uint64_t), void (Dest::*write_chunk_3)(uint64_t),
                  void (Dest::*write_chunk_4)(uint64_t), void (Dest::*write_chunk_5)(uint64_t),
                  void (Dest::*write_chunk_6)(uint64_t), void (Dest::*write_chunk_7)(uint64_t),
                  void (Dest::*write_chunk_8)(uint64_t),
                  void (Dest::*append_additional_chunk)(uint64_t)) {
  uint64_t encoded_first_four_bytes = 0;
  ENCODE_CHUNK_NUMBER(1);
  ENCODE_CHUNK_NUMBER(2);
  ENCODE_CHUNK_NUMBER(3);
  ENCODE_CHUNK_NUMBER(4);
  ENCODE_CHUNK_NUMBER(5);
  ENCODE_CHUNK_NUMBER(6);
  ENCODE_CHUNK_NUMBER(7);
  ENCODE_CHUNK_NUMBER(8);

  bool continue_reading = true;
  source += 64;
  while (continue_reading) {
    continue_reading = EncodeFirstChunk(source, &encoded_first_four_bytes);
    if (encoded_first_four_bytes == 0) return;
    (dest->*append_additional_chunk)(encoded_first_four_bytes);
    source += 8;
  }
}

template <typename Dest>
void EncodeString(const char* source, Dest* out) {
  EncodeString(source, out, &Dest::set_encoded_name_1, &Dest::set_encoded_name_2,
               &Dest::set_encoded_name_3, &Dest::set_encoded_name_4, &Dest::set_encoded_name_5,
               &Dest::set_encoded_name_6, &Dest::set_encoded_name_7, &Dest::set_encoded_name_8,
               &Dest::add_encoded_name_additional);
}

template void EncodeString<orbit_grpc_protos::ApiScopeStart>(const char* source,
                                                             orbit_grpc_protos::ApiScopeStart* out);

template void EncodeString<orbit_grpc_protos::ApiScopeStartAsync>(
    const char* source, orbit_grpc_protos::ApiScopeStartAsync* out);

template void EncodeString<orbit_grpc_protos::ApiStringEvent>(
    const char* source, orbit_grpc_protos::ApiStringEvent* out);
}  // namespace orbit_api