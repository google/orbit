// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/container/flat_hash_map.h>
#include <absl/flags/flag.h>
#include <absl/flags/parse.h>
#include <absl/flags/usage.h>
#include <absl/time/clock.h>
#include <grpcpp/grpcpp.h>

#include <atomic>
#include <csignal>
#include <cstdlib>
#include <limits>
#include <memory>
#include <thread>

#include "CaptureClient/CaptureClient.h"
#include "CaptureClient/CaptureListener.h"
#include "FakeCaptureEventProcessor.h"
#include "GrpcProtos/Constants.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/ThreadPool.h"
#include "OrbitClientData/ModuleManager.h"
#include "capture_data.pb.h"

ABSL_FLAG(uint64_t, port, 44765, "Port OrbitService's gRPC service is listening on");
ABSL_FLAG(int32_t, pid, 0, "PID of the process to capture");
ABSL_FLAG(uint32_t, duration, std::numeric_limits<uint32_t>::max(),
          "Duration of the capture in seconds (stop earlier with Ctrl+C)");
ABSL_FLAG(uint16_t, sampling_rate, 1000,
          "Callstack sampling rate in samples per second (0: no sampling)");
ABSL_FLAG(bool, frame_pointers, false, "Use frame pointers for unwinding");
ABSL_FLAG(bool, scheduling, true, "Collect scheduling information");
ABSL_FLAG(bool, thread_state, false, "Collect thread state information");
ABSL_FLAG(uint16_t, memory_sampling_rate, 0,
          "Memory usage sampling rate in samples per second (0: no sampling)");

namespace {
std::atomic<bool> exit_requested = false;

void SigintHandler(int signum) {
  if (signum == SIGINT) {
    exit_requested = true;
  }
}

// Use SIGINT to stop capturing before the specified duration has elapsed.
void InstallSigintHandler() {
  struct sigaction act {};
  act.sa_handler = SigintHandler;
  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;
  act.sa_restorer = nullptr;
  sigaction(SIGINT, &act, nullptr);
}

}  // namespace

// OrbitFakeClient is a simple command line client that connects to a local instance of
// OrbitService and asks it to take a capture, with capture options specified through command line
// arguments.
// It provides a simple way to make OrbitService take a capture in order to tests its performance
// with various capture options.
// All the received events are discarded, except for counting them and their total size (see
// FakeCaptureEventProcessor).
int main(int argc, char* argv[]) {
  absl::SetProgramUsageMessage("Orbit fake client for testing");
  absl::ParseCommandLine(argc, argv);

  FAIL_IF(absl::GetFlag(FLAGS_pid) == 0, "PID to capture not specified");
  FAIL_IF(absl::GetFlag(FLAGS_duration) == 0, "Specified a zero-length duration");

  int32_t process_id = absl::GetFlag(FLAGS_pid);
  LOG("process_id=%d", process_id);
  uint16_t samples_per_second = absl::GetFlag(FLAGS_sampling_rate);
  LOG("samples_per_second=%u", samples_per_second);
  orbit_grpc_protos::UnwindingMethod unwinding_method =
      absl::GetFlag(FLAGS_frame_pointers)
          ? orbit_grpc_protos::UnwindingMethod::kFramePointerUnwinding
          : orbit_grpc_protos::UnwindingMethod::kDwarfUnwinding;
  LOG("unwinding_method=%s",
      unwinding_method == orbit_grpc_protos::UnwindingMethod::kFramePointerUnwinding
          ? "Frame pointers"
          : "DWARF");
  bool collect_scheduling_info = absl::GetFlag(FLAGS_scheduling);
  LOG("collect_scheduling_info=%d", collect_scheduling_info);
  bool collect_thread_state = absl::GetFlag(FLAGS_thread_state);
  LOG("collect_thread_state=%d", collect_thread_state);
  constexpr bool kEnableApi = false;
  constexpr bool kEnableIntrospection = false;
  constexpr uint64_t kMaxLocalMarkerDepthPerCommandBuffer = std::numeric_limits<uint64_t>::max();
  bool collect_memory_info = absl::GetFlag(FLAGS_memory_sampling_rate) > 0;
  LOG("collect_memory_info=%d", collect_memory_info);
  uint64_t memory_sampling_period_ns = 0;
  if (collect_memory_info) {
    memory_sampling_period_ns = 1'000'000'000 / absl::GetFlag(FLAGS_memory_sampling_rate);
    LOG("memory_sampling_period_ns=%u", memory_sampling_period_ns);
  }

  uint32_t grpc_port = absl::GetFlag(FLAGS_port);
  std::string service_address = absl::StrFormat("127.0.0.1:%d", grpc_port);
  std::shared_ptr<grpc::Channel> grpc_channel =
      grpc::CreateChannel(service_address, grpc::InsecureChannelCredentials());
  LOG("service_address=%s", service_address);
  CHECK(grpc_channel != nullptr);

  InstallSigintHandler();

  orbit_capture_client::CaptureClient capture_client{grpc_channel};
  std::unique_ptr<ThreadPool> thread_pool = ThreadPool::Create(1, 1, absl::Seconds(1));
  orbit_client_data::ModuleManager module_manager;

  auto capture_event_processor = std::make_unique<orbit_fake_client::FakeCaptureEventProcessor>();

  auto capture_outcome_future = capture_client.Capture(
      thread_pool.get(), process_id, module_manager,
      absl::flat_hash_map<uint64_t, orbit_client_protos::FunctionInfo>{}, TracepointInfoSet{},
      samples_per_second, unwinding_method, collect_scheduling_info, collect_thread_state,
      kEnableApi, kEnableIntrospection, kMaxLocalMarkerDepthPerCommandBuffer, collect_memory_info,
      memory_sampling_period_ns, std::move(capture_event_processor));
  LOG("Asked to start capture");

  uint32_t duration_s = absl::GetFlag(FLAGS_duration);
  absl::Time start_time = absl::Now();
  while (!exit_requested && absl::Now() < start_time + absl::Seconds(duration_s)) {
    std::this_thread::sleep_for(std::chrono::milliseconds{100});
    CHECK(!capture_outcome_future.IsFinished());
  }
  CHECK(capture_client.StopCapture());
  LOG("Asked to stop capture");

  auto capture_outcome_or_error = capture_outcome_future.Get();
  if (capture_outcome_or_error.has_error()) {
    FATAL("Capture failed: %s", capture_outcome_or_error.error().message());
  }
  CHECK(capture_outcome_or_error.value() ==
        orbit_capture_client::CaptureListener::CaptureOutcome::kComplete);
  LOG("Capture completed");

  thread_pool->ShutdownAndWait();

  return 0;
}
