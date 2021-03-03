// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "SourceCodeReport.h"

#include <algorithm>
#include <limits>
#include <optional>

#include "OrbitBase/Logging.h"
#include "OrbitClientData/PostProcessedSamplingData.h"

namespace orbit_gl {

SourceCodeReport::SourceCodeReport(std::string_view source_file,
                                   const orbit_client_protos::FunctionInfo& function,
                                   uint64_t absolute_address, orbit_elf_utils::ElfFile* elf_file,
                                   const PostProcessedSamplingData& sampling_data,
                                   uint32_t total_samples_in_capture)
    : total_samples_in_capture_(total_samples_in_capture) {
  for (size_t offset = 0; offset < function.size(); ++offset) {
    const ThreadSampleData* summary = sampling_data.GetSummary();
    const auto it = summary->raw_address_count.find(absolute_address + offset);
    if (it == summary->raw_address_count.end()) continue;

    const uint32_t current_samples = it->second;
    if (current_samples == 0) continue;

    const auto maybe_current_line_info = elf_file->GetLineInfo(function.address() + offset);
    if (!maybe_current_line_info.has_value()) continue;

    const auto& current_line_info = maybe_current_line_info.value();
    if (source_file != current_line_info.source_file()) {
      ERROR(
          "Was trying to gather sampling data for function \"%s\" but the debug information "
          "tells me the function address %#x is defined in a different source file.",
          function.pretty_name(), function.address() + offset);
      ERROR("Expected: %s", source_file);
      ERROR("Actual: %s", current_line_info.source_file());
      continue;
    }

    min_line_number_ = std::min(min_line_number_, current_line_info.source_line());
    max_line_number_ = std::max(max_line_number_, current_line_info.source_line());

    number_of_samples_per_line_[current_line_info.source_line()] += current_samples;
    total_samples_in_function_ += current_samples;
  }
}

std::optional<uint32_t> SourceCodeReport::GetNumSamplesAtLine(size_t line) const {
  // Outside the function, no data. So we return an empty optional.
  if (line < min_line_number_ || line > max_line_number_) return std::nullopt;

  // We are inside the function and we have samples
  if (number_of_samples_per_line_.contains(line)) return number_of_samples_per_line_.at(line);

  // We are inside the function, but have no samples. So we return 0
  return 0;
}

}  // namespace orbit_gl