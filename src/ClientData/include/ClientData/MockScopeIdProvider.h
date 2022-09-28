
// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_CLIENT_DATA_MOCK_SCOPE_ID_PROVIDER_H_
#define ORBIT_CLIENT_DATA_MOCK_SCOPE_ID_PROVIDER_H_

#include <gmock/gmock.h>
#include <stdint.h>

#include "ClientData/ScopeIdProvider.h"
#include "GrpcProtos/capture.pb.h"

namespace orbit_client_data {

class MockScopeIdProvider : public ScopeIdProvider {
 public:
  MOCK_METHOD(std::optional<ScopeId>, FunctionIdToScopeId, (uint64_t), (const, override));
  MOCK_METHOD(uint64_t, ScopeIdToFunctionId, (ScopeId), (const, override));
  MOCK_METHOD(ScopeId, GetMaxId, (), (const, override));
  MOCK_METHOD(std::optional<ScopeId>, ProvideId, (const TimerInfo&), (override));
  MOCK_METHOD(std::vector<ScopeId>, GetAllProvidedScopeIds, (), (const, override));
  MOCK_METHOD(const ScopeInfo&, GetScopeInfo, (ScopeId), (const, override));
  MOCK_METHOD(const FunctionInfo*, GetFunctionInfo, (ScopeId), (const, override));
  MOCK_METHOD(void, UpdateFunctionInfoAddress, (orbit_grpc_protos::InstrumentedFunction),
              (override));
  MOCK_METHOD(std::optional<uint64_t>, FindFunctionIdSlow, (const FunctionInfo&),
              (const, override));
};

}  // namespace orbit_client_data

#endif  // ORBIT_CLIENT_DATA_MOCK_SCOPE_ID_PROVIDER_H_