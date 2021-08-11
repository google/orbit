// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_API_UTILS_ENCODED_STRING_H_
#define ORBIT_API_UTILS_ENCODED_STRING_H_

#include <cstddef>
#include <cstdint>
#include <string>

// In order to avoid expensive allocations in proto buffers, we encode the first 64 characters in
// eight 64bit fields (byte by byte). Any additional characters will be also encoded in a sequence
// of 64bit fields. So a string [0x11, 0x22, 0x33] would get encoded as 0x0000000000332211, all
// other fields are zero and there are no additional fields.
namespace orbit_api {

template <typename Dest>
void EncodeString(const char* source, Dest* dest, void (Dest::*write_chunk_1)(uint64_t),
                  void (Dest::*write_chunk_2)(uint64_t), void (Dest::*write_chunk_3)(uint64_t),
                  void (Dest::*write_chunk_4)(uint64_t), void (Dest::*write_chunk_5)(uint64_t),
                  void (Dest::*write_chunk_6)(uint64_t), void (Dest::*write_chunk_7)(uint64_t),
                  void (Dest::*write_chunk_8)(uint64_t),
                  void (Dest::*append_additional_chunk)(uint64_t));

// This function is a specialization of the `EncodeString` function above that assumes
// `encode_name_#` and `encoded_name_additional` as field names.
template <typename Dest>
void EncodeString(const char* source, Dest* out);

std::string DecodeString(uint64_t encoded_chunk_1, uint64_t encoded_chunk_2,
                         uint64_t encoded_chunk_3, uint64_t encoded_chunk_4,
                         uint64_t encoded_chunk_5, uint64_t encoded_chunk_6,
                         uint64_t encoded_chunk_7, uint64_t encoded_chunk_8,
                         const uint64_t* encoded_chunk_additional,
                         size_t encoded_chunk_addition_count);

}  // namespace orbit_api

#endif  // ORBIT_API_UTILS_ENCODED_STRING_H_
