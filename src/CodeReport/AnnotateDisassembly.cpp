// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CodeReport/AnnotateDisassembly.h"

#include <absl/container/flat_hash_map.h>
#include <absl/hash/hash.h>
#include <absl/strings/str_split.h>
#include <stddef.h>

#include <algorithm>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <utility>

#include "OrbitBase/Result.h"
#include "OrbitBase/Sort.h"

namespace orbit_code_report {

static std::vector<std::string_view> SplitIntoLines(std::string_view source_file_contents) {
  std::vector<std::string_view> source_file_lines = absl::StrSplit(source_file_contents, '\n');
  for (auto& line : source_file_lines) {
    if (!line.empty() && line.back() == '\r') line = line.substr(0, line.size() - 1);
  }

  return source_file_lines;
}

[[nodiscard]] std::vector<AnnotatingLine> AnnotateDisassemblyWithSourceCode(
    const orbit_client_data::FunctionInfo& function_info,
    const orbit_grpc_protos::LineInfo& location_info, std::string_view source_file_contents,
    orbit_object_utils::ElfFile* elf, const DisassemblyReport& report) {
  const std::vector<std::string_view> source_file_lines = SplitIntoLines(source_file_contents);

  // We will show each source code line above the first related instruction
  absl::flat_hash_map<size_t, uint64_t> source_line_to_first_instruction_offset;

  for (uint64_t current_offset = 0; current_offset < function_info.size(); ++current_offset) {
    const auto line_info_or_error = elf->GetLineInfo(current_offset + function_info.address());
    if (line_info_or_error.has_error()) continue;
    if (line_info_or_error.value().source_file() != location_info.source_file()) continue;
    if (line_info_or_error.value().source_line() == 0) continue;

    const auto source_line = line_info_or_error.value().source_line() - 1;
    if (source_line >= static_cast<size_t>(source_file_lines.size())) continue;

    source_line_to_first_instruction_offset.emplace(source_line, current_offset);
  }

  std::vector<AnnotatingLine> annotating_lines{};
  annotating_lines.reserve(source_line_to_first_instruction_offset.size());

  for (const auto& [source_line, offset] : source_line_to_first_instruction_offset) {
    const auto disassembly_line_number =
        report.GetLineAtAddress(report.GetAbsoluteFunctionAddress() + offset);
    if (!disassembly_line_number.has_value()) continue;

    AnnotatingLine annotating_line{};
    annotating_line.reference_line = disassembly_line_number.value() + 1;
    annotating_line.line_number = source_line + 1;
    annotating_line.line_contents = std::string{source_file_lines[static_cast<int>(source_line)]};
    annotating_lines.emplace_back(std::move(annotating_line));
  }

  orbit_base::sort(annotating_lines.begin(), annotating_lines.end(),
                   &AnnotatingLine::reference_line);

  return annotating_lines;
}
}  // namespace orbit_code_report