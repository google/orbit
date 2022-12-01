// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ApiUtils/EncodedString.h"

#include <absl/base/casts.h>

#include <cstring>

#include "ApiUtils/Event.h"         // IWYU pragma: keep
#include "GrpcProtos/capture.pb.h"  // IWYU pragma: keep

namespace orbit_api {

std::string DecodeString(uint64_t encoded_chunk_1, uint64_t encoded_chunk_2,
                         uint64_t encoded_chunk_3, uint64_t encoded_chunk_4,
                         uint64_t encoded_chunk_5, uint64_t encoded_chunk_6,
                         uint64_t encoded_chunk_7, uint64_t encoded_chunk_8,
                         const uint64_t* encoded_chunk_additional,
                         size_t encoded_chunk_addition_count) {
  std::string result{};
  if (encoded_chunk_1 == 0) return result;
  char* encoded_name_bytes = absl::bit_cast<char*>(&encoded_chunk_1);
  result.append(encoded_name_bytes, strnlen(encoded_name_bytes, sizeof(uint64_t)));

  if (encoded_chunk_2 == 0) return result;
  encoded_name_bytes = absl::bit_cast<char*>(&encoded_chunk_2);
  result.append(encoded_name_bytes, strnlen(encoded_name_bytes, sizeof(uint64_t)));

  if (encoded_chunk_3 == 0) return result;
  encoded_name_bytes = absl::bit_cast<char*>(&encoded_chunk_3);
  result.append(encoded_name_bytes, strnlen(encoded_name_bytes, sizeof(uint64_t)));

  if (encoded_chunk_4 == 0) return result;
  encoded_name_bytes = absl::bit_cast<char*>(&encoded_chunk_4);
  result.append(encoded_name_bytes, strnlen(encoded_name_bytes, sizeof(uint64_t)));

  if (encoded_chunk_5 == 0) return result;
  encoded_name_bytes = absl::bit_cast<char*>(&encoded_chunk_5);
  result.append(encoded_name_bytes, strnlen(encoded_name_bytes, sizeof(uint64_t)));

  if (encoded_chunk_6 == 0) return result;
  encoded_name_bytes = absl::bit_cast<char*>(&encoded_chunk_6);
  result.append(encoded_name_bytes, strnlen(encoded_name_bytes, sizeof(uint64_t)));

  if (encoded_chunk_7 == 0) return result;
  encoded_name_bytes = absl::bit_cast<char*>(&encoded_chunk_7);
  result.append(encoded_name_bytes, strnlen(encoded_name_bytes, sizeof(uint64_t)));

  if (encoded_chunk_8 == 0) return result;
  encoded_name_bytes = absl::bit_cast<char*>(&encoded_chunk_8);
  result.append(encoded_name_bytes, strnlen(encoded_name_bytes, sizeof(uint64_t)));

  for (size_t i = 0; i < encoded_chunk_addition_count; i++) {
    uint64_t current_encoded_name_additional = encoded_chunk_additional[i];
    if (current_encoded_name_additional == 0) return result;
    encoded_name_bytes = absl::bit_cast<char*>(&current_encoded_name_additional);
    result.append(encoded_name_bytes, strnlen(encoded_name_bytes, sizeof(uint64_t)));
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
  uint64_t encoded_eight_bytes = 0;
  strncpy(absl::bit_cast<char*>(&encoded_eight_bytes), source, sizeof(uint64_t));
  (dest->*write_chunk_1)(encoded_eight_bytes);
  if (strlen(source) <= sizeof(uint64_t)) {
    return;
  }
  source += sizeof(uint64_t);

  encoded_eight_bytes = 0;
  strncpy(absl::bit_cast<char*>(&encoded_eight_bytes), source, sizeof(uint64_t));
  (dest->*write_chunk_2)(encoded_eight_bytes);
  if (strlen(source) <= sizeof(uint64_t)) {
    return;
  }
  source += sizeof(uint64_t);

  encoded_eight_bytes = 0;
  strncpy(absl::bit_cast<char*>(&encoded_eight_bytes), source, sizeof(uint64_t));
  (dest->*write_chunk_3)(encoded_eight_bytes);
  if (strlen(source) <= sizeof(uint64_t)) {
    return;
  }
  source += sizeof(uint64_t);

  encoded_eight_bytes = 0;
  strncpy(absl::bit_cast<char*>(&encoded_eight_bytes), source, sizeof(uint64_t));
  (dest->*write_chunk_4)(encoded_eight_bytes);
  if (strlen(source) <= sizeof(uint64_t)) {
    return;
  }
  source += sizeof(uint64_t);

  encoded_eight_bytes = 0;
  strncpy(absl::bit_cast<char*>(&encoded_eight_bytes), source, sizeof(uint64_t));
  (dest->*write_chunk_5)(encoded_eight_bytes);
  if (strlen(source) <= sizeof(uint64_t)) {
    return;
  }
  source += sizeof(uint64_t);

  encoded_eight_bytes = 0;
  strncpy(absl::bit_cast<char*>(&encoded_eight_bytes), source, sizeof(uint64_t));
  (dest->*write_chunk_6)(encoded_eight_bytes);
  if (strlen(source) <= sizeof(uint64_t)) {
    return;
  }
  source += sizeof(uint64_t);

  encoded_eight_bytes = 0;
  strncpy(absl::bit_cast<char*>(&encoded_eight_bytes), source, sizeof(uint64_t));
  (dest->*write_chunk_7)(encoded_eight_bytes);
  if (strlen(source) <= sizeof(uint64_t)) {
    return;
  }
  source += sizeof(uint64_t);

  encoded_eight_bytes = 0;
  strncpy(absl::bit_cast<char*>(&encoded_eight_bytes), source, sizeof(uint64_t));
  (dest->*write_chunk_8)(encoded_eight_bytes);
  if (strlen(source) <= sizeof(uint64_t)) {
    return;
  }
  source += sizeof(uint64_t);

  while (true) {
    encoded_eight_bytes = 0;
    strncpy(absl::bit_cast<char*>(&encoded_eight_bytes), source, sizeof(uint64_t));
    (dest->*append_additional_chunk)(encoded_eight_bytes);
    if (strlen(source) <= sizeof(uint64_t)) {
      return;
    }
    source += sizeof(uint64_t);
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

template void EncodeString<orbit_grpc_protos::ApiTrackDouble>(
    const char* source, orbit_grpc_protos::ApiTrackDouble* out);

template void EncodeString<orbit_grpc_protos::ApiTrackFloat>(const char* source,
                                                             orbit_grpc_protos::ApiTrackFloat* out);

template void EncodeString<orbit_grpc_protos::ApiTrackInt>(const char* source,
                                                           orbit_grpc_protos::ApiTrackInt* out);

template void EncodeString<orbit_grpc_protos::ApiTrackInt64>(const char* source,
                                                             orbit_grpc_protos::ApiTrackInt64* out);

template void EncodeString<orbit_grpc_protos::ApiTrackUint>(const char* source,
                                                            orbit_grpc_protos::ApiTrackUint* out);

template void EncodeString<orbit_grpc_protos::ApiTrackUint64>(
    const char* source, orbit_grpc_protos::ApiTrackUint64* out);

template void EncodeString<ApiEncodedString>(const char* source, ApiEncodedString* out);
}  // namespace orbit_api