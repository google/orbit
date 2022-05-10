// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ClientServices/LaunchedProcess.h"

#include "OrbitBase/Logging.h"

namespace orbit_client_services {

using orbit_grpc_protos::ProcessInfo;
using orbit_grpc_protos::ProcessToLaunch;

ErrorMessageOr<LaunchedProcess> LaunchedProcess::LaunchProcess(
    const orbit_grpc_protos::ProcessToLaunch& process_to_launch, ProcessClient* client) {
  OUTCOME_TRY(ProcessInfo process_info, client->LaunchProcess(process_to_launch));
  LaunchedProcess::State initial_state = process_to_launch.spin_at_entry_point()
                                             ? LaunchedProcess::State::kSpinningAtEntryPoint
                                             : LaunchedProcess::State::kExecutingOrExited;
  return LaunchedProcess(initial_state, process_info);
}

LaunchedProcess::LaunchedProcess(State initial_state, orbit_grpc_protos::ProcessInfo process_info)
    : state_(initial_state), process_info_(std::move(process_info)) {}

ErrorMessageOr<void> LaunchedProcess::SuspendProcessSpinningAtEntryPoint(ProcessClient* client) {
  ORBIT_CHECK(state_ == State::kSpinningAtEntryPoint);
  OUTCOME_TRY(client->SuspendProcessSpinningAtEntryPoint(process_info_.pid()));
  state_ = State::kSuspendedAtEntryPoint;
  return outcome::success();
}

ErrorMessageOr<void> LaunchedProcess::ResumeProcessSuspendedAtEntryPoint(ProcessClient* client) {
  ORBIT_CHECK(state_ == State::kSuspendedAtEntryPoint);
  OUTCOME_TRY(client->ResumeProcessSuspendedAtEntryPoint(process_info_.pid()));
  state_ = State::kExecutingOrExited;
  return outcome::success();
}

const ProcessInfo& LaunchedProcess::GetProcessInfo() const { return process_info_; }

bool LaunchedProcess::IsProcessSpinningAtEntryPoint() const {
  return state_ == State::kSpinningAtEntryPoint;
}

bool LaunchedProcess::IsProcessSuspendedAtEntryPoint() const {
  return state_ == State::kSuspendedAtEntryPoint;
}

bool LaunchedProcess::IsProcessExecutingOrExited() const {
  return state_ == State::kExecutingOrExited;
}

}  // namespace orbit_client_services
