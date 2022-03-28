// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CLIENT_DATA_API_STRING_EVENT_H_
#define CLIENT_DATA_API_STRING_EVENT_H_

#include <string>
#include <utility>
namespace orbit_client_data {

// Represents the association of an "async scope id" (from async scopes in manual instrumentation)
// with a specific text "name" that we will display on the respective time slices in the UI.
// Note, the legacy manual instrumentation Api allowed placing multiple api string event macros
// together, in order to support arbitrary length strings. To support this mode on old captures,
// we store the `should_concatenate_` bit.
// See `orbit_grpc_protos::ApiStringEvent` and `Orbit.h`.
class ApiStringEvent {
 public:
  ApiStringEvent(uint64_t async_scope_id, std::string name, bool should_concatenate)
      : async_scope_id_(async_scope_id),
        name_(std::move(name)),
        should_concatenate_(should_concatenate) {}

  [[nodiscard]] uint64_t async_scope_id() const { return async_scope_id_; }
  [[nodiscard]] const std::string& name() const { return name_; }
  [[nodiscard]] bool should_concatenate() const { return should_concatenate_; }

 private:
  uint64_t async_scope_id_;
  std::string name_;
  // The old manual instrumentation (orbit_grpc_protos::ApiEvent) allowed
  // arbitrarily long strings by concatenating strings associated with the same
  // async_scope_id. This is no longer the case because
  // orbit_grpc_protos::ApiStringEvent already supports arbitrarily long
  // strings.
  bool should_concatenate_;
};

}  // namespace orbit_client_data

#endif  // CLIENT_DATA_API_STRING_EVENT_H_
