// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FAKE_CLIENT_GRAPHICS_CAPTURE_EVENT_PROCESSOR_H_
#define FAKE_CLIENT_GRAPHICS_CAPTURE_EVENT_PROCESSOR_H_

#include "CaptureClient/CaptureEventProcessor.h"
#include "Flags.h"
#include "OrbitBase/WriteStringToFile.h"

namespace {
template <typename Distribution, typename UnaryOperation>
void ForEachCentile(uint32_t numCentiles, const Distribution& distribution,
                    UnaryOperation&& operation) {
  uint32_t populationSize = std::accumulate(std::begin(distribution), std::end(distribution), 0);
  size_t currentBucket = 0;
  uint32_t runningCount = 0;
  for (uint32_t centile = 1; centile <= numCentiles; ++centile) {
    while (currentBucket < distribution.size() &&
           // checks if runningCount is less than centile/numCentiles of the
           // population size, but keeps the calculation within the integral
           // domain by scaling with numCentiles
           (runningCount * numCentiles) < (populationSize * centile)) {
      runningCount += distribution[currentBucket];
      ++currentBucket;
    }
    operation(currentBucket);
  }
}
}  // namespace

namespace orbit_fake_client {

// This implementation of CaptureEventProcessor mostly discard all events it receives, except for
// keeping track of the calls to the frame boundary function and GPU queue submissions.
class GraphicsCaptureEventProcessor : public orbit_capture_client::CaptureEventProcessor {
 public:
  void ProcessEvent(const orbit_grpc_protos::ClientCaptureEvent& event) override {
    switch (event.event_case()) {
      case orbit_grpc_protos::ClientCaptureEvent::kFunctionCall:
        ProcessFunctionCall(event.function_call());
        break;
      case orbit_grpc_protos::ClientCaptureEvent::kGpuQueueSubmission:
        ProcessGPUQueueSubmission(event.gpu_queue_submission());
        break;
      default:
        break;
    }
  }

  void ProcessGPUQueueSubmission(const orbit_grpc_protos::GpuQueueSubmission& submission) {
    submissions_.emplace_back(submission);
  }

  void ProcessFunctionCall(const orbit_grpc_protos::FunctionCall& function_call) {
    if (function_call.function_id() == kQueuePresentFunctionId) {
      uint64_t start_timestamp_ns = function_call.end_timestamp_ns() - function_call.duration_ns();
      frame_start_boundary_timestamps_.emplace_back(start_timestamp_ns);
    }
  }

  // Calculate the GPU frame times in [ms] by measuring the amount of time spent by the GPU
  // executing a GPU Job, from any queue that was submitted between two sequential vkQueuePresentKHR
  // calls. The algorithm take into account that is possible that two jobs from different queue's
  // can overlap, so the time is not account for multiple times.
  //
  // This function also generates:
  // - The corresponding GPU frame time histogram, that is divided in 1024 buckets, each bucket X
  // represents how many frames have a duration between [X, X+1) ms.
  // - Average GPU frame times[ms]
  void CalculateGPUTimes() {
    LOG("Calculating GPU Times");
    uint64_t total_duration_ns = 0;
    size_t current_submission = 0;
    gpu_time_distribution_.fill(0u);
    for (uint64_t next_frame_start_timestamp : frame_start_boundary_timestamps_) {
      uint64_t frame_time_ns = 0;
      std::vector<std::pair</*begin*/ uint64_t, /*end*/ uint64_t>> command_buffer_timestamps;
      while (current_submission < submissions_.size()) {
        const orbit_grpc_protos::GpuQueueSubmission& submission = submissions_[current_submission];
        if (submission.meta_info().pre_submission_cpu_timestamp() >= next_frame_start_timestamp) {
          break;
        }

        for (const orbit_grpc_protos::GpuSubmitInfo& submit_info : submission.submit_infos()) {
          for (const orbit_grpc_protos::GpuCommandBuffer& command_buffer :
               submit_info.command_buffers()) {
            command_buffer_timestamps.emplace_back(std::make_pair(
                command_buffer.begin_gpu_timestamp_ns(), command_buffer.end_gpu_timestamp_ns()));
          }
        }
        ++current_submission;
      }

      sort(command_buffer_timestamps.begin(), command_buffer_timestamps.end());
      uint64_t current_range_end = 0;
      for (const std::pair<uint64_t, uint64_t>& command_buffer_timestamp :
           command_buffer_timestamps) {
        uint64_t begin_timestamp_ns = command_buffer_timestamp.first;
        uint64_t end_timestamp_ns = command_buffer_timestamp.second;
        if (begin_timestamp_ns >= current_range_end) {
          frame_time_ns += (end_timestamp_ns - begin_timestamp_ns);
          current_range_end = end_timestamp_ns;
        } else if (end_timestamp_ns > current_range_end) {
          frame_time_ns += end_timestamp_ns - current_range_end;
          current_range_end = end_timestamp_ns;
        }
      }

      if (!command_buffer_timestamps.empty()) {
        double frame_time_ms = frame_time_ns / 1.0e6;
        int distribution_index = std::min(static_cast<int>(frame_time_ms), kMaxTimeMs);
        ++gpu_time_distribution_[distribution_index];
      }

      total_duration_ns += frame_time_ns;
    }

    gpu_avg_frame_time_ms = (total_duration_ns / frame_start_boundary_timestamps_.size()) / 1.0e6;
  }

  // Calculate the CPU frame times in [ms] as the time between sequential vkQueuePresentKHR
  // calls. This function also generates:
  // - The corresponding CPU frame time histogram, that is divided in 1024 buckets, where each
  // bucket X represents how many frames have a duration between [X, X+1) ms.
  // - The average CPU frame times [ms]
  void CalculateCPUTimes() {
    LOG("Calculating CPU Times");

    uint64_t frame_boundary_count = frame_start_boundary_timestamps_.size();
    FAIL_IF(frame_boundary_count <= 2,
            "Not enough calls to vkQueuePresentKHR to calculate CPU frame times.");

    cpu_time_distribution_.fill(0u);
    uint64_t total_duration_ns = 0;
    size_t current_frame = 1;
    while (current_frame < frame_start_boundary_timestamps_.size()) {
      uint64_t frame_time_ns = (frame_start_boundary_timestamps_[current_frame] -
                                frame_start_boundary_timestamps_[current_frame - 1]);
      double frame_time_ms = frame_time_ns / 1.0e6;
      int distribution_index = std::min(static_cast<int>(frame_time_ms), kMaxTimeMs);
      total_duration_ns += frame_time_ns;
      ++cpu_time_distribution_[distribution_index];
      ++current_frame;
    }

    cpu_avg_frame_time_ms = static_cast<double>(total_duration_ns) / 1.0e6 /
                            static_cast<double>(frame_boundary_count - 1);
  }

  ~GraphicsCaptureEventProcessor() {
    CalculateCPUTimes();
    CalculateGPUTimes();
    std::filesystem::path filepath(absl::GetFlag(FLAGS_output_path));
    WriteToCSVFile(filepath.append(kCPUFrameTimeFilename).string(), cpu_time_distribution_,
                   cpu_avg_frame_time_ms, frame_start_boundary_timestamps_.size() - 1);
    filepath.assign(absl::GetFlag(FLAGS_output_path));
    WriteToCSVFile(filepath.append(kGPUFrameTimeFilename).string(), gpu_time_distribution_,
                   gpu_avg_frame_time_ms, frame_start_boundary_timestamps_.size());
  }

  template <typename Distribution>
  void WriteToCSVFile(absl::string_view filename, const Distribution& distribution,
                      const double avg, const size_t num_frames) const {
    constexpr int kCentiles = 100;
    constexpr const char* kCSVHeader =
        "num_frames,avg_ms_per_frame,1_100tile_ms_per_frame,2_100tile_ms_per_frame,3_100tile_ms_"
        "per_frame,4_100tile_ms_per_frame,5_100tile_ms_per_frame,6_100tile_ms_per_frame,7_100tile_"
        "ms_per_frame,8_100tile_ms_per_frame,9_100tile_ms_per_frame,10_100tile_ms_per_frame,11_"
        "100tile_ms_per_frame,12_100tile_ms_per_frame,13_100tile_ms_per_frame,14_100tile_ms_per_"
        "frame,15_100tile_ms_per_frame,16_100tile_ms_per_frame,17_100tile_ms_per_frame,18_100tile_"
        "ms_per_frame,19_100tile_ms_per_frame,20_100tile_ms_per_frame,21_100tile_ms_per_frame,22_"
        "100tile_ms_per_frame,23_100tile_ms_per_frame,24_100tile_ms_per_frame,25_100tile_ms_per_"
        "frame,26_100tile_ms_per_frame,27_100tile_ms_per_frame,28_100tile_ms_per_frame,29_100tile_"
        "ms_per_frame,30_100tile_ms_per_frame,31_100tile_ms_per_frame,32_100tile_ms_per_frame,33_"
        "100tile_ms_per_frame,34_100tile_ms_per_frame,35_100tile_ms_per_frame,36_100tile_ms_per_"
        "frame,37_100tile_ms_per_frame,38_100tile_ms_per_frame,39_100tile_ms_per_frame,40_100tile_"
        "ms_per_frame,41_100tile_ms_per_frame,42_100tile_ms_per_frame,43_100tile_ms_per_frame,44_"
        "100tile_ms_per_frame,45_100tile_ms_per_frame,46_100tile_ms_per_frame,47_100tile_ms_per_"
        "frame,48_100tile_ms_per_frame,49_100tile_ms_per_frame,50_100tile_ms_per_frame,51_100tile_"
        "ms_per_frame,52_100tile_ms_per_frame,53_100tile_ms_per_frame,54_100tile_ms_per_frame,55_"
        "100tile_ms_per_frame,56_100tile_ms_per_frame,57_100tile_ms_per_frame,58_100tile_ms_per_"
        "frame,59_100tile_ms_per_frame,60_100tile_ms_per_frame,61_100tile_ms_per_frame,62_100tile_"
        "ms_per_frame,63_100tile_ms_per_frame,64_100tile_ms_per_frame,65_100tile_ms_per_frame,66_"
        "100tile_ms_per_frame,67_100tile_ms_per_frame,68_100tile_ms_per_frame,69_100tile_ms_per_"
        "frame,70_100tile_ms_per_frame,71_100tile_ms_per_frame,72_100tile_ms_per_frame,73_100tile_"
        "ms_per_frame,74_100tile_ms_per_frame,75_100tile_ms_per_frame,76_100tile_ms_per_frame,77_"
        "100tile_ms_per_frame,78_100tile_ms_per_frame,79_100tile_ms_per_frame,80_100tile_ms_per_"
        "frame,81_100tile_ms_per_frame,82_100tile_ms_per_frame,83_100tile_ms_per_frame,84_100tile_"
        "ms_per_frame,85_100tile_ms_per_frame,86_100tile_ms_per_frame,87_100tile_ms_per_frame,88_"
        "100tile_ms_per_frame,89_100tile_ms_per_frame,90_100tile_ms_per_frame,91_100tile_ms_per_"
        "frame,92_100tile_ms_per_frame,93_100tile_ms_per_frame,94_100tile_ms_per_frame,95_100tile_"
        "ms_per_frame,96_100tile_ms_per_frame,97_100tile_ms_per_frame,98_100tile_ms_per_frame,99_"
        "100tile_ms_per_frame,100_100tile_ms_per_frame";
    std::string output;
    absl::StrAppend(&output, kCSVHeader, "\n", absl::StrFormat("%u,%.2f", num_frames, avg));

    std::vector<uint32_t> centiles;
    centiles.reserve(100);
    ForEachCentile(kCentiles, distribution, [&](uint32_t centile_value) {
      absl::StrAppend(&output, absl::StrFormat(",%u", centile_value));
    });
    absl::StrAppend(&output, "\n");

    ErrorMessageOr<void> frame_time_write_result = orbit_base::WriteStringToFile(filename, output);
    FAIL_IF(frame_time_write_result.has_error(), "Writing to \"%s\": %s", filename,
            frame_time_write_result.error().message());
  }

  // Instrument a function with this function id in order for GraphicsCaptureEventProcessor to use
  // it as frame boundary to compute the average CPU frame time.
  static constexpr uint64_t kQueuePresentFunctionId = std::numeric_limits<uint64_t>::max();

 private:
  static constexpr int kMaxTimeMs = 1023;
  static constexpr const char* kCPUFrameTimeFilename = "cpu_frame_times.txt";
  static constexpr const char* kGPUFrameTimeFilename = "gpu_frame_times.txt";

  double gpu_avg_frame_time_ms = 0.0;
  double cpu_avg_frame_time_ms = 0.0;
  std::array<uint32_t, kMaxTimeMs + 1> gpu_time_distribution_;
  std::array<uint32_t, kMaxTimeMs + 1> cpu_time_distribution_;
  std::vector<uint64_t> frame_start_boundary_timestamps_;
  std::vector<orbit_grpc_protos::GpuQueueSubmission> submissions_;
};

}  // namespace orbit_fake_client

#endif  // FAKE_CLIENT_GRAPHICS_CAPTURE_EVENT_PROCESSOR_H_
