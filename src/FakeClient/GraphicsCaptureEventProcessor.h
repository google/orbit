// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FAKE_CLIENT_GRAPHICS_CAPTURE_EVENT_PROCESSOR_H_
#define FAKE_CLIENT_GRAPHICS_CAPTURE_EVENT_PROCESSOR_H_

#include <absl/types/span.h>

#include <cstdint>

#include "CaptureClient/CaptureEventProcessor.h"
#include "Flags.h"
#include "OrbitBase/WriteStringToFile.h"

namespace orbit_fake_client {

namespace {
struct CommandBufferTimestamps {
  uint64_t begin;
  uint64_t end;

  bool operator<(const CommandBufferTimestamps& rhs) const {
    if (begin == rhs.begin) {
      return end < rhs.end;
    }

    return begin < rhs.begin;
  }
};

template <typename Distribution, typename UnaryOperation>
void ForEachCentile(uint32_t num_centiles, const Distribution& distribution,
                    UnaryOperation&& operation) {
  uint32_t population_size = std::accumulate(std::begin(distribution), std::end(distribution), 0);
  size_t current_bucket = 0;
  uint32_t running_count = 0;
  for (uint32_t centile = 1; centile <= num_centiles; ++centile) {
    while (current_bucket < distribution.size() &&
           // checks if `running_count` is less than `centile/num_centiles` of the
           // population size, but keeps the calculation within the integral
           // domain by scaling with `num_centiles`
           (running_count * num_centiles) < (population_size * centile)) {
      running_count += distribution[current_bucket];
      ++current_bucket;
    }
    operation(current_bucket);
  }
}

}  // namespace

// This implementation of CaptureEventProcessor mostly discards all events it receives, except for
// keeping track of the calls to the frame boundary function and GPU queue submissions.
class GraphicsCaptureEventProcessor : public orbit_capture_client::CaptureEventProcessor {
 public:
  ~GraphicsCaptureEventProcessor() {
    CalculateCpuStats();
    CalculateGpuStats();
    std::filesystem::path file_path(absl::GetFlag(FLAGS_output_path));
    std::string cpu_file_path = file_path.append(kCpuFrameTimeFilename).string();
    ORBIT_LOG("Writing CPU results to \"%s\"", cpu_file_path);
    WriteToCsvFile(cpu_file_path, cpu_time_distribution_, cpu_avg_frame_time_ms_,
                   frame_start_boundary_timestamps_.size() - 1);
    file_path.assign(absl::GetFlag(FLAGS_output_path));
    std::string gpu_file_path = file_path.append(kGpuFrameTimeFilename).string();
    ORBIT_LOG("Writing GPU results to \"%s\"", gpu_file_path);
    WriteToCsvFile(gpu_file_path, gpu_time_distribution_, gpu_avg_frame_time_ms_,
                   frame_start_boundary_timestamps_.size());
  }

 private:
  void ProcessEvent(const orbit_grpc_protos::ClientCaptureEvent& event) override {
    switch (event.event_case()) {
      case orbit_grpc_protos::ClientCaptureEvent::kFunctionCall:
        ProcessFunctionCall(event.function_call());
        break;
      case orbit_grpc_protos::ClientCaptureEvent::kGpuQueueSubmission:
        ProcessGpuQueueSubmission(event.gpu_queue_submission());
        break;
      default:
        break;
    }
  }

  void ProcessGpuQueueSubmission(const orbit_grpc_protos::GpuQueueSubmission& submission) {
    submissions_.emplace_back(submission);
  }

  void ProcessFunctionCall(const orbit_grpc_protos::FunctionCall& function_call) {
    if (function_call.function_id() == kQueuePresentFunctionId) {
      uint64_t start_timestamp_ns = function_call.end_timestamp_ns() - function_call.duration_ns();
      frame_start_boundary_timestamps_.emplace_back(start_timestamp_ns);
    }
  }

  void CalculateGpuStats() {
    ORBIT_LOG("Calculating GPU Times");
    ORBIT_LOG("Calculating frame durations");
    CalculateGpuFrameDurations();
    ORBIT_LOG("Generating duration distribution");
    GenerateGpuDurationDistribution();
    ORBIT_LOG("Calculating average frame time");
    CalculateGpuAvgFrameTime();
    ORBIT_LOG("Finished calculating GPU times");
  }

  void CalculateGpuFrameDurations() {
    size_t current_submission = 0;
    // Timestamps should come in CPU start timestamp order, but we sort regardless to be careful and
    // since runtime doesn't matter that much here.
    sort(frame_start_boundary_timestamps_.begin(), frame_start_boundary_timestamps_.end());
    sort(submissions_.begin(), submissions_.end(),
         [](const orbit_grpc_protos::GpuQueueSubmission& a,
            const orbit_grpc_protos::GpuQueueSubmission& b) {
           return a.meta_info().pre_submission_cpu_timestamp() <
                  b.meta_info().pre_submission_cpu_timestamp();
         });

    for (uint64_t next_frame_start_timestamp : frame_start_boundary_timestamps_) {
      std::vector<CommandBufferTimestamps> command_buffer_timestamps;
      while (current_submission < submissions_.size()) {
        const orbit_grpc_protos::GpuQueueSubmission& submission = submissions_[current_submission];
        if (submission.meta_info().pre_submission_cpu_timestamp() >= next_frame_start_timestamp) {
          break;
        }

        for (const orbit_grpc_protos::GpuSubmitInfo& submit_info : submission.submit_infos()) {
          for (const orbit_grpc_protos::GpuCommandBuffer& command_buffer :
               submit_info.command_buffers()) {
            CommandBufferTimestamps timestamps{.begin = command_buffer.begin_gpu_timestamp_ns(),
                                               .end = command_buffer.end_gpu_timestamp_ns()};
            command_buffer_timestamps.emplace_back(timestamps);
          }
        }
        ++current_submission;
      }

      uint64_t frame_time_ns = CalculateFrameGpuTime(command_buffer_timestamps);
      uint64_t frame_time_ms = frame_time_ns / 1000000;
      if (frame_time_ms > kMaxTimeMs) {
        ORBIT_LOG("Frame with a duration of %u(ms) is bigger than %d(ms)", frame_time_ms,
                  kMaxTimeMs);
        ORBIT_LOG("Dumping frame command buffers timestamps...");
        PrintCommandBufferTimestamps(command_buffer_timestamps);
      } else {
        frame_gpu_durations_ns_.emplace_back(frame_time_ns);
      }
    }
  }

  static void PrintCommandBufferTimestamps(
      absl::Span<const CommandBufferTimestamps> command_buffers_timestamps) {
    for (size_t i = 0; i < command_buffers_timestamps.size(); ++i) {
      const uint64_t begin_timestamp_ns = command_buffers_timestamps[i].begin;
      const uint64_t end_timestamp_ns = command_buffers_timestamps[i].end;
      const uint64_t duration_ns = end_timestamp_ns - begin_timestamp_ns;
      ORBIT_LOG("CommandBuffer #%u: Start: %u End: %u Duration: %u(ns)", i, begin_timestamp_ns,
                end_timestamp_ns, duration_ns);
    }
  }

  void GenerateGpuDurationDistribution() {
    gpu_time_distribution_.fill(0u);
    for (const uint64_t duration_ns : frame_gpu_durations_ns_) {
      UpdateFrameDurationDistribution(duration_ns, gpu_time_distribution_);
    }
  }

  void CalculateGpuAvgFrameTime() {
    uint64_t total_duration_ns = std::accumulate(frame_gpu_durations_ns_.begin(),
                                                 frame_gpu_durations_ns_.end(), uint64_t(0));
    gpu_avg_frame_time_ms_ = (total_duration_ns / frame_start_boundary_timestamps_.size()) / 1.0e6;
  }

  uint64_t CalculateFrameGpuTime(std::vector<CommandBufferTimestamps> command_buffers_timestamps) {
    // The frame gpu time is calculated as the union of all the command buffer intervals; to do
    // this, we sort the command buffer by starting time and compute the length of the union of all
    // intervals.
    //
    // There's no guarantee that the submission order is mantained throughout the execution; command
    // buffers that belong to different queues are executed in parallel or they can be executed out
    // of order because there isn't any depedency between them. So to get the correct results the
    // array needs to be sorted first.
    sort(command_buffers_timestamps.begin(), command_buffers_timestamps.end());
    uint64_t frame_time_ns = 0;
    uint64_t current_range_end = 0;  // Keeps track of the last interval end time.
    for (const CommandBufferTimestamps& command_buffer_timestamp : command_buffers_timestamps) {
      const uint64_t begin_timestamp_ns = command_buffer_timestamp.begin;
      const uint64_t end_timestamp_ns = command_buffer_timestamp.end;

      // Skip the command buffers that were partially tracked, the ones where the
      // `begin_timestamp_ns` is zero. This happens when the capture process starts while the
      // command buffer is being recorded, so the Orbit Vulkan layer only tracks the end of the
      // command buffer execution but not the start.
      if (begin_timestamp_ns == 0) {
        continue;
      }

      // If the interval doesn't overlap just add the length of the interval to the frame time, else
      // if there's overlap just add the new part of the interval that hasn't yet been taken into
      // account.
      if (begin_timestamp_ns >= current_range_end) {
        frame_time_ns += (end_timestamp_ns - begin_timestamp_ns);
        current_range_end = end_timestamp_ns;
      } else if (end_timestamp_ns > current_range_end) {
        frame_time_ns += end_timestamp_ns - current_range_end;
        current_range_end = end_timestamp_ns;
      }
    }

    return frame_time_ns;
  }

  void CalculateCpuStats() {
    ORBIT_LOG("Calculating CPU Times");

    uint64_t frame_boundary_count = frame_start_boundary_timestamps_.size();
    ORBIT_FAIL_IF(frame_boundary_count <= 2,
                  "Not enough calls to vkQueuePresentKHR to calculate CPU frame times.");

    ORBIT_LOG("Calculating frame durations");
    CalculateCpuFrameDurations();
    ORBIT_LOG("Generating duration distribution");
    GenerateCpuDurationDistribution();
    ORBIT_LOG("Calculating average frame times");
    CalculateCpuAvgFrameTime();
    ORBIT_LOG("Finished calculating CPU times");
  }

  void CalculateCpuFrameDurations() {
    for (size_t current_frame = 1; current_frame < frame_start_boundary_timestamps_.size();
         ++current_frame) {
      uint64_t frame_time_ns = (frame_start_boundary_timestamps_[current_frame] -
                                frame_start_boundary_timestamps_[current_frame - 1]);
      frame_cpu_durations_ns_.emplace_back(frame_time_ns);
    }
  }

  void GenerateCpuDurationDistribution() {
    cpu_time_distribution_.fill(0u);
    for (uint64_t frame_duration_ns : frame_cpu_durations_ns_) {
      UpdateFrameDurationDistribution(frame_duration_ns, cpu_time_distribution_);
    }
  }

  void CalculateCpuAvgFrameTime() {
    uint64_t total_duration_ns = std::accumulate(frame_cpu_durations_ns_.begin(),
                                                 frame_cpu_durations_ns_.end(), uint64_t(0));
    cpu_avg_frame_time_ms_ = (static_cast<double>(total_duration_ns) / 1.0e6) /
                             static_cast<double>(frame_start_boundary_timestamps_.size() - 1);
  }

  template <typename Distribution>
  static void UpdateFrameDurationDistribution(uint64_t frame_time_ns, Distribution& distribution) {
    if (frame_time_ns > 0) {
      uint64_t frame_time_ms_floor = frame_time_ns / 1000000;
      uint64_t index = std::min(frame_time_ms_floor, kMaxTimeMs);
      ++distribution[index];
    }
  }

  template <typename Distribution>
  static void WriteToCsvFile(absl::string_view filename, const Distribution& distribution,
                             const double avg, const size_t num_frames) {
    constexpr int kCentiles = 100;
    std::string header = "num_frames,avg_ms_per_frame";
    for (int centile = 1; centile <= kCentiles; ++centile) {
      absl::StrAppend(&header, absl::StrFormat(",%d_%dtile_ms_per_frame", centile, kCentiles));
    }
    absl::StrAppend(&header, "\n");
    std::string output;
    absl::StrAppend(&output, header, absl::StrFormat("%u,%.2f", num_frames, avg));

    std::vector<uint32_t> centiles;
    centiles.reserve(100);
    ForEachCentile(kCentiles, distribution, [&](uint32_t centile_value) {
      absl::StrAppend(&output, absl::StrFormat(",%u", centile_value));
    });
    absl::StrAppend(&output, "\n");

    ErrorMessageOr<void> frame_time_write_result = orbit_base::WriteStringToFile(filename, output);
    ORBIT_FAIL_IF(frame_time_write_result.has_error(), "Writing to \"%s\": %s", filename,
                  frame_time_write_result.error().message());
  }

  // Instrument a function with this function id in order for GraphicsCaptureEventProcessor to use
  // it as frame boundary to compute the average CPU frame time.
  static constexpr uint64_t kQueuePresentFunctionId = std::numeric_limits<uint64_t>::max();

  static constexpr uint64_t kMaxTimeMs = 1023;  // This is an arbitrary number
  static constexpr const char* kCpuFrameTimeFilename = "cpu_frame_times.txt";
  static constexpr const char* kGpuFrameTimeFilename = "gpu_frame_times.txt";

  double gpu_avg_frame_time_ms_ = 0.0;
  double cpu_avg_frame_time_ms_ = 0.0;
  // The frame time histograms are divided in 1024 buckets, where each
  // bucket X represents how many frames have a duration between [X, X+1) ms.
  std::array<uint32_t, kMaxTimeMs + 1> gpu_time_distribution_;
  std::array<uint32_t, kMaxTimeMs + 1> cpu_time_distribution_;
  std::vector<uint64_t> frame_start_boundary_timestamps_;
  std::vector<uint64_t> frame_gpu_durations_ns_;
  std::vector<uint64_t> frame_cpu_durations_ns_;
  std::vector<orbit_grpc_protos::GpuQueueSubmission> submissions_;
};

}  // namespace orbit_fake_client

#endif  // FAKE_CLIENT_GRAPHICS_CAPTURE_EVENT_PROCESSOR_H_
