// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/strings/match.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <unistd.h>

#include <filesystem>
#include <optional>
#include <string>

#include "GrpcProtos/services.pb.h"
#include "OrbitBase/NotFoundOr.h"
#include "OrbitBase/Result.h"
#include "ProcessService/CpuTime.h"
#include "ProcessServiceUtils.h"
#include "Test/Path.h"
#include "TestUtils/TestUtils.h"

namespace orbit_process_service {

using orbit_base::NotFoundOr;
using orbit_grpc_protos::GetDebugInfoFileRequest;
using orbit_test_utils::HasError;
using orbit_test_utils::HasValue;

TEST(ProcessServiceUtils, GetCumulativeTotalCpuTime) {
  // There is not much invariance here which we can test.
  // We know the optional should return a value and we know it's positive and
  // monotonically increasing.

  const auto& total_cpu_time1 = GetCumulativeTotalCpuTime();
  ASSERT_TRUE(total_cpu_time1.has_value());
  ASSERT_TRUE(total_cpu_time1->jiffies.value > 0ul);
  ASSERT_TRUE(total_cpu_time1->cpus > 0ul);

  const auto& total_cpu_time2 = GetCumulativeTotalCpuTime();
  ASSERT_TRUE(total_cpu_time2.has_value());
  ASSERT_TRUE(total_cpu_time2->jiffies.value > 0ul);
  ASSERT_TRUE(total_cpu_time2->cpus == total_cpu_time1->cpus);

  ASSERT_TRUE(total_cpu_time2->jiffies.value >= total_cpu_time1->jiffies.value);
}

TEST(ProcessServiceUtils, GetCumulativeCpuTimeFromProcess) {
  const auto& jiffies1 = GetCumulativeCpuTimeFromProcess(getpid());
  ASSERT_TRUE(jiffies1.has_value());

  const auto& jiffies2 = GetCumulativeCpuTimeFromProcess(getpid());
  ASSERT_TRUE(jiffies2.has_value());

  ASSERT_TRUE(jiffies2->value >= jiffies1->value);

  const auto& total_cpu_time = GetCumulativeTotalCpuTime();
  ASSERT_TRUE(total_cpu_time.has_value());
  ASSERT_TRUE(total_cpu_time->jiffies.value > 0ul);

  // A single process should never have consumed more CPU cycles than the total CPU time
  ASSERT_TRUE(jiffies2->value <= total_cpu_time->jiffies.value);
}

TEST(ProcessServiceUtils, FindSymbolsFilePath) {
  const std::filesystem::path test_directory = orbit_test::GetTestdataDir();

  {  // elf - same file
    const std::filesystem::path module_path = test_directory / "hello_world_elf";
    GetDebugInfoFileRequest request;
    request.set_module_path(module_path.string());
    request.add_additional_search_directories(test_directory);
    const ErrorMessageOr<NotFoundOr<std::filesystem::path>> result = FindSymbolsFilePath(request);
    ASSERT_THAT(result, HasValue());
    ASSERT_FALSE(orbit_base::IsNotFound(result.value()));
    const auto result_path = orbit_base::GetFound(result.value());
    EXPECT_EQ(result_path, module_path);
  }

  {  // coff - same file (the coff file actually does not include a build id)
    const std::filesystem::path module_path = test_directory / "libtest.dll";
    GetDebugInfoFileRequest request;
    request.set_module_path(module_path.string());
    request.add_additional_search_directories(test_directory);
    const ErrorMessageOr<NotFoundOr<std::filesystem::path>> result = FindSymbolsFilePath(request);
    ASSERT_THAT(result, HasValue());
    ASSERT_FALSE(orbit_base::IsNotFound(result.value()));
    const auto result_path = orbit_base::GetFound(result.value());
    EXPECT_EQ(result_path, module_path);
  }

  {  // elf - separate file
    const std::filesystem::path module_path = test_directory / "no_symbols_elf";
    const std::filesystem::path symbols_path = test_directory / "no_symbols_elf.debug";
    GetDebugInfoFileRequest request;
    request.set_module_path(module_path.string());
    request.add_additional_search_directories(test_directory);
    const ErrorMessageOr<NotFoundOr<std::filesystem::path>> result = FindSymbolsFilePath(request);
    ASSERT_THAT(result, HasValue());
    ASSERT_FALSE(orbit_base::IsNotFound(result.value()));
    const auto result_path = orbit_base::GetFound(result.value());
    EXPECT_EQ(result_path, symbols_path);
  }

  {  // coff/pdb - separate file
    const std::filesystem::path module_path = test_directory / "dllmain.dll";
    const std::filesystem::path symbols_path = test_directory / "dllmain.pdb";
    GetDebugInfoFileRequest request;
    request.set_module_path(module_path.string());
    request.add_additional_search_directories(test_directory);
    const ErrorMessageOr<NotFoundOr<std::filesystem::path>> result = FindSymbolsFilePath(request);
    ASSERT_THAT(result, HasValue());
    ASSERT_FALSE(orbit_base::IsNotFound(result.value()));
    const auto result_path = orbit_base::GetFound(result.value());
    EXPECT_EQ(result_path, symbols_path);
  }

  {  // non existing module
    const std::filesystem::path module_path = test_directory / "not_existing_file";
    GetDebugInfoFileRequest request;
    request.set_module_path(module_path.string());
    request.add_additional_search_directories(test_directory);
    const ErrorMessageOr<NotFoundOr<std::filesystem::path>> result = FindSymbolsFilePath(request);
    EXPECT_THAT(result, HasError("Unable to load object file"));
  }

  {  // elf - no build id, but does include symbols
    const std::filesystem::path module_path = test_directory / "hello_world_elf_no_build_id";
    GetDebugInfoFileRequest request;
    request.set_module_path(module_path.string());
    request.add_additional_search_directories(test_directory);
    const ErrorMessageOr<NotFoundOr<std::filesystem::path>> result = FindSymbolsFilePath(request);
    ASSERT_THAT(result, HasValue());
    ASSERT_FALSE(orbit_base::IsNotFound(result.value()));
    const auto result_path = orbit_base::GetFound(result.value());
    EXPECT_EQ(result_path, module_path);
  }

  {  // elf - no build id, no symbols
    const std::filesystem::path module_path = test_directory / "no_symbols_no_build_id";
    GetDebugInfoFileRequest request;
    request.set_module_path(module_path.string());
    request.add_additional_search_directories(test_directory);
    const ErrorMessageOr<NotFoundOr<std::filesystem::path>> result = FindSymbolsFilePath(request);
    ASSERT_THAT(result, HasValue());
    ASSERT_TRUE(orbit_base::IsNotFound(result.value()));

    EXPECT_TRUE(absl::StrContains(orbit_base::GetNotFoundMessage(result.value()),
                                  "does not contain symbols and does not contain a build id"));
  }
}

}  // namespace orbit_process_service
