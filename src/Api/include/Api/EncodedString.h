// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_API_ENCODED_STRING_H_
#define ORBIT_API_ENCODED_STRING_H_

#include <cstddef>
#include <cstdint>
#include <string>

// In order to avoid expensive allocations in proto buffers, we encode the first 64 characters in
// eight fixed64 fields (byte by byte). Any additional characters will be also encoded in a sequence
// of 64bit fields. So a string [0x11, 0x22, 0x33] would get encoded as 0x1122330000000000, all
// other fields are zero and there are no additional fields.
namespace orbit_api {

template <typename Dest>
void EncodeString(const char* source, Dest* dest, void (Dest::*write_chunk_1)(uint64_t),
                  void (Dest::*write_chunk_2)(uint64_t), void (Dest::*write_chunk_3)(uint64_t),
                  void (Dest::*write_chunk_4)(uint64_t), void (Dest::*write_chunk_5)(uint64_t),
                  void (Dest::*write_chunk_6)(uint64_t), void (Dest::*write_chunk_7)(uint64_t),
                  void (Dest::*write_chunk_8)(uint64_t),
                  void (Dest::*append_additional_chunk)(uint64_t));

template <typename Dest>
void EncodeString(const char* source, Dest* out);

std::string DecodeString(uint64_t encoded_name_1, uint64_t encoded_name_2, uint64_t encoded_name_3,
                         uint64_t encoded_name_4, uint64_t encoded_name_5, uint64_t encoded_name_6,
                         uint64_t encoded_name_7, uint64_t encoded_name_8,
                         const uint64_t* encoded_name_additional,
                         size_t encoded_name_addition_count);

}  // namespace orbit_api

#endif  // ORBIT_API_ENCODED_STRING_H_
