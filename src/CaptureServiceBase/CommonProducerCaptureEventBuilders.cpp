// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CaptureServiceBase/CommonProducerCaptureEventBuilders.h"

#include <filesystem>
#include <memory>
#include <string>
#include <utility>

#include "GrpcProtos/capture.pb.h"
#include "ObjectUtils/CoffFile.h"
#include "ObjectUtils/ElfFile.h"
#include "OrbitBase/ExecutablePath.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"
#include "OrbitVersion/OrbitVersion.h"

using orbit_grpc_protos::CaptureFinished;
using orbit_grpc_protos::CaptureOptions;
using orbit_grpc_protos::CaptureStarted;
using orbit_grpc_protos::ProducerCaptureEvent;

namespace orbit_capture_service_base {

namespace {

ErrorMessageOr<std::string> GetBuildIdFromCoffExecutable(
    const std::filesystem::path& executable_path) {
  OUTCOME_TRY(std::unique_ptr<orbit_object_utils::CoffFile> coff_file,
              orbit_object_utils::CreateCoffFile(executable_path));
  return coff_file->GetBuildId();
}

ErrorMessageOr<std::string> GetBuildIdFromElfExecutable(
    const std::filesystem::path& executable_path) {
  OUTCOME_TRY(std::unique_ptr<orbit_object_utils::ElfFile> coff_file,
              orbit_object_utils::CreateElfFile(executable_path));
  return coff_file->GetBuildId();
}

}  // namespace

ProducerCaptureEvent CreateCaptureStartedEvent(const CaptureOptions& capture_options,
                                               absl::Time capture_start_time,
                                               uint64_t capture_start_timestamp_ns) {
  ProducerCaptureEvent event;
  CaptureStarted* capture_started = event.mutable_capture_started();

  uint32_t target_pid = capture_options.pid();

  capture_started->set_process_id(target_pid);
  auto executable_path_or_error = orbit_base::GetExecutablePath(target_pid);

  if (executable_path_or_error.has_value()) {
    const std::filesystem::path& executable_path = executable_path_or_error.value();
    capture_started->set_executable_path(executable_path.u8string());

    ErrorMessageOr<std::string> build_id_or_error = ErrorMessage("");
    if (executable_path.extension() == ".exe") {
      build_id_or_error = GetBuildIdFromCoffExecutable(executable_path);
    } else {
      build_id_or_error = GetBuildIdFromElfExecutable(executable_path);
    }

    if (build_id_or_error.has_value()) {
      capture_started->set_executable_build_id(build_id_or_error.value());
    } else {
      ORBIT_ERROR("Unable to find build id for module \"%s\": %s", executable_path.u8string(),
                  build_id_or_error.error().message());
    }
  } else {
    ORBIT_ERROR("%s", executable_path_or_error.error().message());
  }

  capture_started->set_capture_start_unix_time_ns(absl::ToUnixNanos(capture_start_time));
  capture_started->set_capture_start_timestamp_ns(capture_start_timestamp_ns);
  orbit_version::Version version = orbit_version::GetVersion();
  capture_started->set_orbit_version_major(version.major_version);
  capture_started->set_orbit_version_minor(version.minor_version);
  capture_started->mutable_capture_options()->CopyFrom(capture_options);
  return event;
}

ProducerCaptureEvent CreateSuccessfulCaptureFinishedEvent() {
  ProducerCaptureEvent event;
  CaptureFinished* capture_finished = event.mutable_capture_finished();
  capture_finished->set_status(CaptureFinished::kSuccessful);
  return event;
}

ProducerCaptureEvent CreateInterruptedByServiceCaptureFinishedEvent(std::string message) {
  ProducerCaptureEvent event;
  CaptureFinished* capture_finished = event.mutable_capture_finished();
  capture_finished->set_status(CaptureFinished::kInterruptedByService);
  capture_finished->set_error_message(std::move(message));
  return event;
}

ProducerCaptureEvent CreateFailedCaptureFinishedEvent(std::string message) {
  ProducerCaptureEvent event;
  CaptureFinished* capture_finished = event.mutable_capture_finished();
  capture_finished->set_status(CaptureFinished::kFailed);
  capture_finished->set_error_message(std::move(message));
  return event;
}

ProducerCaptureEvent CreateClockResolutionEvent(uint64_t timestamp_ns, uint64_t resolution_ns) {
  ProducerCaptureEvent event;
  orbit_grpc_protos::ClockResolutionEvent* clock_resolution_event =
      event.mutable_clock_resolution_event();
  clock_resolution_event->set_timestamp_ns(timestamp_ns);
  clock_resolution_event->set_clock_resolution_ns(resolution_ns);
  return event;
}

ProducerCaptureEvent CreateErrorEnablingOrbitApiEvent(uint64_t timestamp_ns, std::string message) {
  ProducerCaptureEvent event;
  orbit_grpc_protos::ErrorEnablingOrbitApiEvent* error_enabling_orbit_api_event =
      event.mutable_error_enabling_orbit_api_event();
  error_enabling_orbit_api_event->set_timestamp_ns(timestamp_ns);
  error_enabling_orbit_api_event->set_message(std::move(message));
  return event;
}

ProducerCaptureEvent CreateErrorEnablingUserSpaceInstrumentationEvent(uint64_t timestamp_ns,
                                                                      std::string message) {
  ProducerCaptureEvent event;
  orbit_grpc_protos::ErrorEnablingUserSpaceInstrumentationEvent*
      error_enabling_user_space_instrumentation_event =
          event.mutable_error_enabling_user_space_instrumentation_event();
  error_enabling_user_space_instrumentation_event->set_timestamp_ns(timestamp_ns);
  error_enabling_user_space_instrumentation_event->set_message(std::move(message));
  return event;
}

ProducerCaptureEvent CreateWarningInstrumentingWithUserSpaceInstrumentationEvent(
    uint64_t timestamp_ns,
    const absl::flat_hash_map<uint64_t, std::string>& function_ids_to_error_messages) {
  ProducerCaptureEvent event;
  orbit_grpc_protos::WarningInstrumentingWithUserSpaceInstrumentationEvent* warning_event =
      event.mutable_warning_instrumenting_with_user_space_instrumentation_event();
  warning_event->set_timestamp_ns(timestamp_ns);
  for (auto const& [id, error_message] : function_ids_to_error_messages) {
    orbit_grpc_protos::FunctionThatFailedToBeInstrumented* function =
        warning_event->add_functions_that_failed_to_instrument();
    function->set_function_id(id);
    function->set_error_message(error_message);
  }
  return event;
}

ProducerCaptureEvent CreateWarningEvent(uint64_t timestamp_ns, std::string message) {
  ProducerCaptureEvent event;
  orbit_grpc_protos::WarningEvent* warning_event = event.mutable_warning_event();
  warning_event->set_timestamp_ns(timestamp_ns);
  warning_event->set_message(std::move(message));
  return event;
}

}  // namespace orbit_capture_service_base
