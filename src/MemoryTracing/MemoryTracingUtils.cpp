// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "MemoryTracingUtils.h"

#include <absl/container/flat_hash_set.h>
#include <absl/strings/ascii.h>
#include <absl/strings/numbers.h>
#include <absl/strings/str_split.h>
#include <stdlib.h>

#include <string>

#include "GrpcProtos/Constants.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Profiling.h"
#include "OrbitBase/ReadFileToString.h"
#include "OrbitBase/Result.h"

namespace orbit_memory_tracing {

using orbit_grpc_protos::kMissingInfo;
using orbit_grpc_protos::SystemMemoryUsage;

// See http://en.wikipedia.org/wiki/Kilobyte for details on SI decimal and IEC binary definitions.
//
// SI decimal      SI decimal   IEC binary       IEC binary
// prefix          value        prefix           value
// kilobyte (kB)   10^3         kibibyte (KiB)   2^10
// megabyte (MB)   10^6         mebibyte (MiB)   2^20
// gigabyte (GB)   10^9         gibibyte (GiB)   2^30
// terabyte (TB)   10^12        tebibyte (TiB)   2^40
// petabyte (PB)   10^15        pebibyte (PiB)   2^50
// exabyte  (EB)   10^18        exbibyte (EiB)   2^60
constexpr uint64_t kDecimalBase = 1000;
constexpr uint64_t kKiloBytes = kDecimalBase;
constexpr uint64_t kMegaBytes = kKiloBytes * kDecimalBase;
constexpr uint64_t kGigaBytes = kMegaBytes * kDecimalBase;
constexpr uint64_t kBinaryBase = 1024;
constexpr uint64_t kKibiBytes = kBinaryBase;
constexpr uint64_t kMebiBytes = kKibiBytes * kBinaryBase;
constexpr uint64_t kGibiBytes = kMebiBytes * kBinaryBase;

std::optional<uint64_t> ExtractSizeUintConversionFactor(const std::string& unit_string) {
  // Decimal base
  const absl::flat_hash_set<std::string> kKiloBytesAliases = {"k", "kb", "kilobyte", "kilobytes"};
  const absl::flat_hash_set<std::string> kMegaBytesAliases = {"m", "mb", "megabyte", "megabytes"};
  const absl::flat_hash_set<std::string> kGigaBytesAliases = {"gb", "gigabyte", "gigabytes"};

  // Binary base
  const absl::flat_hash_set<std::string> kKibiBytesAliases = {"kib", "kibibyte", "kibibytes"};
  const absl::flat_hash_set<std::string> kMebiBytesAliases = {"mib", "mebibyte", "mebibytes"};
  const absl::flat_hash_set<std::string> kGibiBytesAliases = {"GiB", "gibibyte", "gibibytes"};

  if (unit_string.empty()) return std::nullopt;

  // Return the size unit conversion factor when converting to Bytes.
  std::string lowercase_unit_string = unit_string;
  absl::AsciiStrToLower(&lowercase_unit_string);
  if (kKiloBytesAliases.contains(lowercase_unit_string)) {
    return kKiloBytes;
  } else if (kMegaBytesAliases.contains(lowercase_unit_string)) {
    return kMegaBytes;
  } else if (kGigaBytesAliases.contains(lowercase_unit_string)) {
    return kGigaBytes;
  } else if (kKibiBytesAliases.contains(lowercase_unit_string)) {
    return kKibiBytes;
  } else if (kMebiBytesAliases.contains(lowercase_unit_string)) {
    return kMebiBytes;
  } else if (kGibiBytesAliases.contains(lowercase_unit_string)) {
    return kGibiBytes;
  }

  return std::nullopt;
}

std::optional<SystemMemoryUsage> ParseMemInfo(const std::string& meminfo_content) {
  if (meminfo_content.empty()) return std::nullopt;

  std::vector<std::string> lines = absl::StrSplit(meminfo_content, '\n');
  if (lines.empty()) return std::nullopt;

  SystemMemoryUsage memory_info;
  memory_info.set_total_kb(kMissingInfo);
  memory_info.set_free_kb(kMissingInfo);
  memory_info.set_available_kb(kMissingInfo);
  memory_info.set_buffers_kb(kMissingInfo);
  memory_info.set_cached_kb(kMissingInfo);

  constexpr size_t kNumLines = 5;
  std::vector<std::string> top_lines(
      lines.begin(), lines.begin() + (lines.size() > kNumLines ? kNumLines : lines.size()));
  std::optional<uint64_t> unit_conversion_factor;
  int64_t memory_size_value;
  for (const std::string& line : top_lines) {
    // Each line of the /proc/meminfo file consists of a parameter name, followed by a colon, the
    // value of the parameter, and an option unit of measurement(e.g., "kB").
    std::vector<std::string> splits = absl::StrSplit(line, ' ', absl::SkipWhitespace{});
    if (splits.size() < 3) continue;

    unit_conversion_factor = ExtractSizeUintConversionFactor(splits[2]);
    if (!unit_conversion_factor.has_value()) continue;

    if (!absl::SimpleAtoi(splits[1], &memory_size_value)) continue;
    // As we use kilobytes in the protos, the memory size unit will be converted to Kilobytes.
    memory_size_value = memory_size_value * unit_conversion_factor.value() / kKiloBytes;

    if (splits[0] == "MemTotal:") {
      memory_info.set_total_kb(memory_size_value);
    } else if (splits[0] == "MemFree:") {
      memory_info.set_free_kb(memory_size_value);
    } else if (splits[0] == "MemAvailable:") {
      memory_info.set_available_kb(memory_size_value);
    } else if (splits[0] == "Buffers:") {
      memory_info.set_buffers_kb(memory_size_value);
    } else if (splits[0] == "Cached:") {
      memory_info.set_cached_kb(memory_size_value);
    }
  }

  return memory_info;
}

std::optional<SystemMemoryUsage> GetSystemMemoryUsage() noexcept {
  uint64_t current_timestamp_ns = orbit_base::CaptureTimestampNs();

  ErrorMessageOr<std::string> reading_result = orbit_base::ReadFileToString("/proc/meminfo");
  if (!reading_result.has_value()) {
    ERROR("%s", reading_result.error().message());
    return std::nullopt;
  }

  std::optional<SystemMemoryUsage> parsing_result = ParseMemInfo(reading_result.value());
  if (parsing_result.has_value()) {
    parsing_result.value().set_timestamp_ns(current_timestamp_ns);
  }
  return parsing_result;
}

}  // namespace orbit_memory_tracing